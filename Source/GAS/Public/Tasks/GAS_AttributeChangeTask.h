#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GAS_AttributeChangeTask.generated.h"


struct FOnAttributeChangeData;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChange, FGameplayAttribute, Attribute, float, NewValue, float, OldValue);

UCLASS(BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class GAS_API UGAS_AttributeChangeTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChange OnAttributeChange;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UGAS_AttributeChangeTask* ListenForAttributeChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute, float& NewValue, float& OldValue);

	UFUNCTION(BlueprintCallable)
	void EndTask();
	
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	FGameplayAttribute AttributeToListenFor;
	
	void AttributeChange(const FOnAttributeChangeData& Data);
};
