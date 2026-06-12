#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GAS_GameplayAbility.h"
#include "GAS_EnemyAbility.generated.h"


namespace EPathFollowingResult
{
	enum Type : int;
}


class UAITask_MoveTo;
class AGAS_EnemyCharacter;
class AAIController;
class AGAS_BaseCharacter;
class UGAS_WaitGameplayEvent;
class UAbilityTask_WaitDelay;
class UAbilityTask_MoveTo;

UCLASS()
class GAS_API UGAS_EnemyAbility : public UGAS_GameplayAbility
{
	GENERATED_BODY()
	
public:
	UGAS_EnemyAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	TWeakObjectPtr<AGAS_EnemyCharacter> OwningEnemy;
	TWeakObjectPtr<AAIController> OwningAIController;
	TWeakObjectPtr<AGAS_BaseCharacter> TargetBaseCharacter;
	
private:
	UPROPERTY()
	TObjectPtr<UGAS_WaitGameplayEvent> WaitGameplayEventTask;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> SearchDelayTask;
	
	UPROPERTY()
	TObjectPtr<UAITask_MoveTo> MoveToLocationOnActorTask;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> AttackDelayTask;
	
	void StartSearch();
	
	UFUNCTION()
	void EndAttackEventReceived(FGameplayEventData Payload);
	
	UFUNCTION()
	void Search();
	
	UFUNCTION()
	void MoveTargetAndAttack();
	
	UFUNCTION()
	void AttackTarget(TEnumAsByte<EPathFollowingResult::Type> Result, AAIController* AIController);

	UFUNCTION()
	void Attack();
};
