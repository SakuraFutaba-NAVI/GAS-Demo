#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Bag/InventoryComponent.h"

#include "GAS_BaseCharacter.generated.h"


namespace GAS_Tags
{
	extern GAS_API const FName Player;
}

struct FOnAttributeChangeData;
class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FASCInitialized,UAbilitySystemComponent*, ASC,UAttributeSet* ,AS);

UCLASS(Abstract)//抽象类，不能直接实例化
class GAS_API AGAS_BaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()//生成反射数据和其他UE4特性所需的宏

public:
	AGAS_BaseCharacter();
	
	
	// 实现IAbilitySystemInterface接口，返回AbilitySystemComponent的指针
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UAttributeSet* GetAttributeSet() const {return nullptr;}
	
    bool IsAlive() const {return bAlive;}
	bool IsMove() const {return bMove;}
	bool IsJump() const {return bJump;}
	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;
	bool IsAttack() const {return bAttack;}
	
	
	void BeginAttack();
	void EndAttack();
	
	void SetAlive(bool bAliveStatus){bAlive = bAliveStatus;};
	void SetMove(bool bMoveStatus){bMove = bMoveStatus;};
	void SetJump(bool bJumpStatus){bJump = bJumpStatus;};
	void SetAttack(bool bAttackStatus){bAttack = bAttackStatus;};
	
	UPROPERTY(BlueprintAssignable)
	FASCInitialized OnASCInitialized;
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Death")
	virtual void HandleRespawn();
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Attribute")
	void ResetAttributes();
	
	UFUNCTION(BlueprintImplementableEvent)
	void RotateToTarget(AActor* RotateTarget);
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	// 蓝图快速获取背包（方便UI/逻辑调用）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }


protected:
	void GiveStartupAbilities();
	void InitializeAttributes() const;
	
	void OnHealthChange(const FOnAttributeChangeData& AttributeChangeData);
	virtual void HandleDeath();

private:
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> InitializeAttributesEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> ResetAttributesEffect;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	
	bool bAlive = true;
	bool bMove = false;
	bool bJump = false;
	bool bAttack = false;
};