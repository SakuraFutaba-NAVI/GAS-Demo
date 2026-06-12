#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/InventoryItemData.h"

#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

// 背包中实际存储的物品（带数量）
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "背包")
	FInventoryItem ItemData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "背包")
	int32 ItemCount = 0;

	// 判断是否为空槽
	bool IsEmpty() const { return ItemCount <= 0 || ItemData.ItemID == 0; }
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ===================== 编辑器配置 =====================
	// 背包最大格子数
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "背包设置", meta = (ClampMin = 1))
	int32 InventoryCapacity = 15;

	// 物品数据表（提前在编辑器创建）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "背包设置")
	TObjectPtr<UDataTable> ItemDataTable;

	// ===================== 蓝图调用函数 =====================
	// 添加物品
	UFUNCTION(BlueprintCallable, Category = "背包")
	bool AddItem(int32 ItemID, int32 Count = 1);

	// 移除物品
	UFUNCTION(BlueprintCallable, Category = "背包")
	bool RemoveItem(int32 ItemID, int32 Count = 1);

	// 使用物品（格子索引）
	UFUNCTION(BlueprintCallable, Category = "背包")
	bool UseItemByIndex(int32 SlotIndex);

	// 清空背包
	UFUNCTION(BlueprintCallable, Category = "背包")
	void ClearInventory();

	// 获取所有背包格子
	UFUNCTION(BlueprintCallable, Category = "背包")
	TArray<FInventorySlot> GetAllSlots() const { return InventorySlots; }
	
	
	UPROPERTY(BlueprintAssignable, Category = "背包事件")
	FOnInventoryChanged OnInventoryChanged;

private:
	// 背包格子数组
	UPROPERTY()
	TArray<FInventorySlot> InventorySlots;

	// 内部：根据ID获取物品数据
	FInventoryItem FindItemDataByID(int32 ItemID) const;
	// 内部：找到可堆叠的槽位
	int32 FindStackableSlot(int32 ItemID) const;
	// 内部：找到空槽位
	int32 FindEmptySlot() const;
	// 内部：执行物品使用逻辑
	void ExecuteItemUse(const FInventoryItem& Item);
};