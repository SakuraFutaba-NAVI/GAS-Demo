#pragma once

#include "CoreMinimal.h"
#include "Character/GAS_BaseCharacter.h"
#include "Item/DropItemData.h"
#include "Bag/InventoryComponent.h"

#include "GAS_EnemyCharacter.generated.h"


class UAbilitySystemComponent;
class UAttributeSet;

USTRUCT(BlueprintType)
struct FEnemyDropEntry
{
	GENERATED_BODY()

	// 直接引用你背包的物品数据表行（不用再填ID了，直接选物品）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "掉落")
	FDataTableRowHandle ItemRow;

	// 掉落概率 0.0~1.0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "掉落", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DropChance = 0.5f;

	// 掉落数量范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "掉落", meta = (ClampMin = 1))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "掉落", meta = (ClampMin = 1))
	int32 MaxCount = 1;
};


UCLASS()
class GAS_API AGAS_EnemyCharacter : public AGAS_BaseCharacter
{
	GENERATED_BODY()
	
public:
	AGAS_EnemyCharacter();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UAttributeSet* GetAttributeSet() const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|AI")
	float AcceptanceRadius{500.f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|AI")
	float MinAttackDelay{.1f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|AI")
	float MaxAttackDelay{.5f};
	
	UFUNCTION(BlueprintImplementableEvent)
	float GetTimelineLength();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bIsBeingLaunched{false};
	
	void StopMovement();
	
	// 敌人掉落表（蓝图里配置）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落系统")
	TArray<FEnemyDropEntry> DropTable;

	// 最后一次造成伤害的对象（弱引用，死亡时用来发奖励）
	UPROPERTY(BlueprintReadOnly, Category = "掉落系统")
	TWeakObjectPtr<AActor> LastAttacker;

	// 死亡时生成掉落物（服务器端执行）
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnLoot();
	virtual void Server_SpawnLoot_Implementation();
	virtual bool Server_SpawnLoot_Validate();
	
protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath() override;
	
	
	
private:
	
	UFUNCTION()
	void EnableMovementOnLanded(const FHitResult& Hit);
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeSet> AttributeSet;
};
