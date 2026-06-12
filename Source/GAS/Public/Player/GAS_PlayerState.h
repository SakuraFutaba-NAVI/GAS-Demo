#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"

#include "GAS_PlayerState.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;


UCLASS()
class GAS_API AGAS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AGAS_PlayerState();

	// 实现IAbilitySystemInterface接口，返回AbilitySystemComponent的指针
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const {return AttributeSet;}

private:
	UPROPERTY(EditAnywhere, Category = "Ability")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
};
