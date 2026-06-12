#pragma once

#include "CoreMinimal.h"
#include "Character/GAS_BaseCharacter.h"
#include "GAS_PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class GAS_API AGAS_PlayerCharacter : public AGAS_BaseCharacter
{
	GENERATED_BODY()
	
public:
	AGAS_PlayerCharacter();

	// 实现IAbilitySystemInterface接口，返回AbilitySystemComponent的指针
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual UAttributeSet* GetAttributeSet() const override;
	
	virtual void PossessedBy(AController* NewController) override;//当角色被控制器占领时调用，可以在这里进行一些初始化操作，比如绑定输入等。
	virtual void OnRep_PlayerState() override;//当PlayerState发生变化时调用，可以在这里更新角色的状态或者属性等。

	
private:
	UPROPERTY(VisibleAnywhere, Category = "GAS|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "GAS|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

};
