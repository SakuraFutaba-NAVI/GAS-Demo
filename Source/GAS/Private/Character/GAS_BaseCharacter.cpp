#include "Character/GAS_BaseCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Bag/InventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/Player/GAS_Attack.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySpec.h"

namespace GAS_Tags
{
	const FName Player = FName("Player");
}


AGAS_BaseCharacter::AGAS_BaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	//获取Mesh组件，并设置其动画更新选项为始终更新姿势和刷新骨骼。这意味着无论角色是否可见，动画系统都会持续更新角色的姿势和骨骼信息。
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	
	//挂载背包组件
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}


//获取生命周期复制属性
void AGAS_BaseCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bAlive);
}


//获取能力组件
UAbilitySystemComponent* AGAS_BaseCharacter::GetAbilitySystemComponent() const
{
	return nullptr;
}

void AGAS_BaseCharacter::Jump()
{
	bJump = true;
	Super::Jump();
}

void AGAS_BaseCharacter::Landed(const FHitResult& Hit)
{
	bJump = false;
	Super::Landed(Hit);
}



void AGAS_BaseCharacter::BeginAttack()
{
	bAttack = true;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}
}

void AGAS_BaseCharacter::EndAttack()
{
	bAttack = false;
}


//赋予能力
void AGAS_BaseCharacter::GiveStartupAbilities()
{
	if (!IsValid(GetAbilitySystemComponent()))return;
	for (const auto& Ability : StartupAbilities)
	{
		
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability);
		GetAbilitySystemComponent()->GiveAbility(AbilitySpec);
	}
}


//初始化属性
void AGAS_BaseCharacter::InitializeAttributes() const
{
	checkf(IsValid(InitializeAttributesEffect), TEXT("InitializeAttributesEffect not set"));
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(InitializeAttributesEffect,1.f, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}


//生命变化时
void AGAS_BaseCharacter::OnHealthChange(const FOnAttributeChangeData& AttributeChangeData)
{
	if (AttributeChangeData.NewValue <= 0.f)
	{
		HandleDeath();
	}
}


//处理死亡
void AGAS_BaseCharacter::HandleDeath()
{
	bAlive = false;
}


//处理重生
void AGAS_BaseCharacter::HandleRespawn()
{
	bAlive = true;
}


//重置属性
void AGAS_BaseCharacter::ResetAttributes()
{
	checkf(IsValid(ResetAttributesEffect), TEXT("ResetAttributesEffect not set"));
	if (!IsValid(GetAbilitySystemComponent()))return;
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(InitializeAttributesEffect,1.f, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
