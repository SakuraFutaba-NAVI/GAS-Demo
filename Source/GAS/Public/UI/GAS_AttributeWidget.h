#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"

#include "GAS_AttributeWidget.generated.h"


class UGAS_AttributeSet;

UCLASS()
class GAS_API UGAS_AttributeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "GAS|Attributes")
	FGameplayAttribute Attribute;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "GAS|Attributes")
	FGameplayAttribute MaxAttribute;

	void OnAttributeChange(const TTuple<FGameplayAttribute, FGameplayAttribute>& Pair, const UGAS_AttributeSet* AttributeSet);
	bool MatchesAttributes(const TTuple<FGameplayAttribute, FGameplayAttribute>& Pair) const;
	
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Attribute Change"))
	void BP_OnAttributeChange(float NewValue, float NewMaxValue);
};
