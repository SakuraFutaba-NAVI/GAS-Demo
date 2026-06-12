#include "AbilitySystem/Abilities/Enemy/GAS_EnemyAbility.h"

#include "AIController.h"
#include "Abilities/Async/AbilityAsync.h"
#include "Abilities/Async/AbilityAsync_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystem/AbilityTasks/GAS_WaitGameplayEvent.h"
#include "Character/GAS_EnemyCharacter.h"
#include "GameplayTags/GASTags.h"
#include "Tasks/AITask_MoveTo.h"
#include "Utils/GAS_BlueprintLibrary.h"


UGAS_EnemyAbility::UGAS_EnemyAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}


//ai控制器
void UGAS_EnemyAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	OwningEnemy = Cast<AGAS_EnemyCharacter>(GetAvatarActorFromActorInfo());
	check(OwningEnemy.IsValid());
	
	OwningAIController = Cast<AAIController>(OwningEnemy->GetController());
	check(OwningAIController.IsValid());
	
	StartSearch();
	
	WaitGameplayEventTask = UGAS_WaitGameplayEvent::WaitGameplayEventToActorProxy(GetAvatarActorFromActorInfo(),GASTags::Events::Enemy::EndAttack);
	WaitGameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::EndAttackEventReceived);
    WaitGameplayEventTask->StartActivation();
}


//开始寻敌
void UGAS_EnemyAbility::StartSearch()
{
	if (bDrawDebugs)GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("UGAS_SearchForTarget::StartSearch")));
	if (!OwningEnemy.IsValid()) return;;
	
	const float SearchDelay = FMath::RandRange(OwningEnemy->MinAttackDelay, OwningEnemy->MaxAttackDelay);
	SearchDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, SearchDelay);
	SearchDelayTask->OnFinish.AddDynamic(this, &ThisClass::Search);
	SearchDelayTask->Activate();
}


//结束攻击
void UGAS_EnemyAbility::EndAttackEventReceived(FGameplayEventData Payload)
{
	if (OwningEnemy.IsValid() && !OwningEnemy->bIsBeingLaunched)
	StartSearch();
}


//搜索
void UGAS_EnemyAbility::Search()
{
	const FVector SearchOrigin = GetAvatarActorFromActorInfo()->GetActorLocation();
	FClosestActorWithTagResult ClosestActorResult = UGAS_BlueprintLibrary::FindClosestActorWithTag(this, SearchOrigin, GAS_Tags::Player);
	
	TargetBaseCharacter = Cast<AGAS_BaseCharacter>(ClosestActorResult.Actor);
	
	if (!TargetBaseCharacter.IsValid())
	{
		StartSearch();
		return;
	}
	if (TargetBaseCharacter->IsAlive())
	{
		MoveTargetAndAttack();
	}
	else
	{
		StartSearch();
	}
}


//移动到目标并攻击
void UGAS_EnemyAbility::MoveTargetAndAttack()
{
	if (!OwningEnemy.IsValid() || !OwningAIController.IsValid() || !TargetBaseCharacter.IsValid()) return;
	if (!OwningEnemy->IsAlive())
	{
	   	StartSearch();
		return;
	}
	
	MoveToLocationOnActorTask = UAITask_MoveTo::AIMoveTo(OwningAIController.Get(), FVector(), TargetBaseCharacter.Get(), OwningEnemy->AcceptanceRadius);
	
	MoveToLocationOnActorTask->OnMoveTaskFinished.AddUObject(this, &ThisClass::AttackTarget);
    MoveToLocationOnActorTask->ConditionalPerformMove();
}


//攻击目标
void UGAS_EnemyAbility::AttackTarget(TEnumAsByte<EPathFollowingResult::Type> Result, AAIController* AIController)
{
	if (Result != EPathFollowingResult::Success)
	{
		StartSearch();
		return;
	}
	
	OwningEnemy->RotateToTarget(TargetBaseCharacter.Get());
	
	AttackDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, OwningEnemy->GetTimelineLength());
	AttackDelayTask->OnFinish.AddDynamic(this, &ThisClass::Attack);
	AttackDelayTask->Activate();
}


//攻击
void UGAS_EnemyAbility::Attack()
{
	const FGameplayTag AttackTag = GASTags::GASAbilities::Enemy::Attack;
	GetAbilitySystemComponentFromActorInfo()->TryActivateAbilitiesByTag(AttackTag.GetSingleTagContainer());
}
