#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Bag/InventoryWidget.h"

#include "GAS_PlayerController.generated.h"


class UInputMappingContext;
class UInputAction;
class UInventoryWidget;
struct FInputActionValue;
struct FGameplayTag;

UCLASS()
class GAS_API AGAS_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	
	virtual void SetupInputComponent() override;


private:
	UPROPERTY(EditAnywhere, Category = "GAS|Input")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;// 存储输入映射上下文的数组，可以在这里添加不同的输入映射上下文以支持不同的输入配置

	// 存储输入动作的指针，可以在这里指定一个输入动作来触发角色的行为
	UPROPERTY(EditAnywhere, Category = "GAS|Input|Movement")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "GAS|Input|Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "GAS|Input|Movement")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "GAS|Input|Ability")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, Category = "GAS|Input|Ability")
	TObjectPtr<UInputAction> SkillAction;

	UPROPERTY(EditAnywhere, Category = "GAS|Input|Ability")
	TObjectPtr<UInputAction> UltimateAction;
	
	UPROPERTY(EditAnywhere, Category = "GAS|Input|Inventory")
	TObjectPtr<UInputAction> InventoryAction;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|UI|Inventory")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> CurrentInventoryWidget;
	

	bool bIsInventoryOpen = false;	

	void Jump();
	void StopJumping();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void Attack();
	void Skill();
	void Ultimate();
	
	void ActivateAbility(const FGameplayTag&AbilityTag) const;


    bool IsAlive() const;
	bool IsMove() const;
	bool IsAttack() const;
	bool IsJump() const;
	
	void OnMoveStarted(const FInputActionValue& Value);
	void OnMoveCompleted(const FInputActionValue& Value);
	
	void ToggleInventory();
	void OpenInventory();
	void CloseInventory();
};
