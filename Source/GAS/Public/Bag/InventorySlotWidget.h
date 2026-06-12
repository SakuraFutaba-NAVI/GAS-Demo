#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UInventoryComponent;

UCLASS()
class GAS_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 初始化时传入：我是第几个格子，我的背包组件是谁
	void Setup(int32 InSlotIndex, UInventoryComponent* InInventoryComp);

	// 根据背包数据刷新自己的显示
	void Refresh();

protected:
	virtual void NativeConstruct() override;

	// ===== 蓝图控件绑定（要求 WB_InventorySlot 设计器里控件同名）=====
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Count;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BTN_Slot;

private:
	UFUNCTION()
	void OnSlotClicked();

	int32 SlotIndex = 0;
	TObjectPtr<UInventoryComponent> InventoryRef;
};