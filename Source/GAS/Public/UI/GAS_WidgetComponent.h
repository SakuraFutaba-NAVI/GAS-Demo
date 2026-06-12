#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Components/WidgetComponent.h"
#include "GAS_WidgetComponent.generated.h"


class UAbilitySystemComponent;
class UAttributeSet;
class UGAS_AttributeSet;
class AGAS_BaseCharacter;
class UGAS_AbilitySystemComponent;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGAS_WidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	TMap<FGameplayAttribute, FGameplayAttribute> AttributeMap;

private:
	TWeakObjectPtr<AGAS_BaseCharacter> GASCharacter;
	TWeakObjectPtr<UGAS_AbilitySystemComponent> AbilitySystemComponent;
    TWeakObjectPtr<UGAS_AttributeSet> AttributeSet;

    void InitAbilitySystemData();
	bool IsASCInitialized() const;
	void InitializeAttributeDelegate();
	void BindWidgetToAttributeChanges(UWidget* WidgetObject, const TTuple<FGameplayAttribute,FGameplayAttribute>& Pair) const;
	
	UFUNCTION()
	void OnASCInitialized(UAbilitySystemComponent* ASC, UAttributeSet* AS);

    UFUNCTION()
	void BindToAttributeChanges();
};
