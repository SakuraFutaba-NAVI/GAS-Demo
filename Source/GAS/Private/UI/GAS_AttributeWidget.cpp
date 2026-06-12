#include "UI/GAS_AttributeWidget.h"

#include "AbilitySystem/GAS_AttributeSet.h"


void UGAS_AttributeWidget::OnAttributeChange(const TTuple<FGameplayAttribute, FGameplayAttribute>& Pair, const UGAS_AttributeSet* AttributeSet)
{
	const float AttributeValue = Pair.Key.GetNumericValue(AttributeSet);
	const float MaxAttributeValue = Pair.Value.GetNumericValue(AttributeSet);
	
	BP_OnAttributeChange(AttributeValue, MaxAttributeValue);
}

bool UGAS_AttributeWidget::MatchesAttributes(const TTuple<FGameplayAttribute, FGameplayAttribute>& Pair) const
{
	return Attribute == Pair.Key && MaxAttribute == Pair.Value;
}
