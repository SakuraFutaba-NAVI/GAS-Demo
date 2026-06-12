#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync_WaitGameplayEvent.h"
#include "GAS_WaitGameplayEvent.generated.h"


UCLASS()
class GAS_API UGAS_WaitGameplayEvent : public UAbilityAsync_WaitGameplayEvent
{
	GENERATED_BODY()
	
public:
	void StartActivation();
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Async", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UGAS_WaitGameplayEvent* WaitGameplayEventToActorProxy(AActor* TargetActor,UPARAM(meta=(GameplayTagFilter="GameplayEventTagsCategory")) FGameplayTag EventTag,bool OnlyTriggerOnce = false,bool OnlyMatchExact = true);

};
