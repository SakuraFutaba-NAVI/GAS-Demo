#include "Character/GAS_EnemyCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/GAS_AbilitySystemComponent.h"
#include "AbilitySystem/GAS_AttributeSet.h"
#include "GameplayTags/GASTags.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Net/UnrealNetwork.h"


AGAS_EnemyCharacter::AGAS_EnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AbilitySystemComponent = CreateDefaultSubobject<UGAS_AbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent-> SetIsReplicated(true);// 开启网络复制：使该组件的数据能够在服务端与客户端之间同步
	AbilitySystemComponent-> SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UGAS_AttributeSet>("AttributeSet");
}

//获取生命周期
void AGAS_EnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     
	DOREPLIFETIME(ThisClass, bIsBeingLaunched);
}


//获取能力组件
UAbilitySystemComponent* AGAS_EnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


//获取属性组件
UAttributeSet* AGAS_EnemyCharacter::GetAttributeSet() const
{
	return AttributeSet;
}


void AGAS_EnemyCharacter::StopMovement()
{
	bIsBeingLaunched = true;
	
	AAIController* AIController = Cast<AAIController>(GetOwner());
	if (!IsValid(AIController)) return;
	AIController->StopMovement();
	
	if (!LandedDelegate.IsAlreadyBound(this, &ThisClass::EnableMovementOnLanded))
	{
		LandedDelegate.AddDynamic(this, &ThisClass::EnableMovementOnLanded);
	}
}


void AGAS_EnemyCharacter::EnableMovementOnLanded(const FHitResult& Hit)
{
	bIsBeingLaunched = false;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, GASTags::Events::Enemy::EndAttack, FGameplayEventData());
	LandedDelegate.RemoveAll(this);
}


void AGAS_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsValid(GetAbilitySystemComponent())) return;
	AbilitySystemComponent->AddSpawnedAttribute(AttributeSet);
	
	GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);
	OnASCInitialized.Broadcast(GetAbilitySystemComponent(),GetAttributeSet());
	
	if (!HasAuthority()) return;
	
	GiveStartupAbilities();	
	InitializeAttributes();
	
	UGAS_AttributeSet* GAS_AttributeSet = Cast<UGAS_AttributeSet>(GetAttributeSet());
	if (!IsValid(GAS_AttributeSet)) return;
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(GAS_AttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChange);
}


//处理死亡
void AGAS_EnemyCharacter::HandleDeath()
{
	Super::HandleDeath();
	
	AAIController* AIController = Cast<AAIController>(GetOwner());
	if (!IsValid(AIController)) return;
	AIController->StopMovement();
	
	if (HasAuthority())
	{
		Server_SpawnLoot();
	}
}


bool AGAS_EnemyCharacter::Server_SpawnLoot_Validate()
{
	// 简单验证，只要敌人有效就允许生成
	return IsValid(this) && !DropTable.IsEmpty();
}


void AGAS_EnemyCharacter::Server_SpawnLoot_Implementation()
{
	AActor* Killer = LastAttacker.Get();
	if (!IsValid(Killer) || DropTable.IsEmpty()) return;

	UInventoryComponent* KillerInventory = Killer->FindComponentByClass<UInventoryComponent>();
	if (!IsValid(KillerInventory)) return;

	for (const FEnemyDropEntry& DropEntry : DropTable)
	{
		// 跳过没选物品的空条目
		if (!DropEntry.ItemRow.RowName.IsNone()) continue;

		// 概率判断
		if (FMath::FRand() <= DropEntry.DropChance)
		{
			// 从你背包的物品数据表行里自动获取ItemID
			// 注意：这里把"FInventoryItem"改成你物品数据表的行结构体名
			const FInventoryItem* ItemData = DropEntry.ItemRow.GetRow<FInventoryItem>("");
			if (!ItemData) continue;

			int32 FinalCount = FMath::RandRange(DropEntry.MinCount, DropEntry.MaxCount);
            
			// 直接添加到背包
			KillerInventory->AddItem(ItemData->ItemID, FinalCount);
		}
	}
}