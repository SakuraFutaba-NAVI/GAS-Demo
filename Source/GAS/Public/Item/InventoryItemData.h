#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "InventoryItemData.generated.h"

// 物品类型
UENUM(BlueprintType)
enum class EInventoryItemType : uint8
{
	Consumable UMETA(DisplayName = "消耗品"),
	Weapon UMETA(DisplayName = "武器"),
	Material UMETA(DisplayName = "材料")
};

// 物品数据表结构体（编辑器配置所有物品）
USTRUCT(BlueprintType)
struct FInventoryItem : public FTableRowBase
{
	GENERATED_BODY()

	// 唯一物品ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "物品")
	int32 ItemID = 0;

	// 物品名称
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "物品")
	FText ItemName;

	// 物品图标
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "物品")
	TObjectPtr<UTexture2D> ItemIcon;

	// 物品类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "物品")
	EInventoryItemType ItemType = EInventoryItemType::Consumable;

	// 最大堆叠数量
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "物品", meta = (ClampMin = 1))
	int32 MaxStack = 99;

	// 使用物品触发的GE（联动你的GAS伤害/治疗系统）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS联动")
	TSubclassOf<UGameplayEffect> UseGameplayEffect;

	// 是否可使用
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "物品")
	bool bCanUse = true;
};