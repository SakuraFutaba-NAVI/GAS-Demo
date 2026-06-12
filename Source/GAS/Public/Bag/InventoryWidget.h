#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Bag/InventoryComponent.h"
#include "InventoryWidget.generated.h"

class UUniformGridPanel;
class UButton;
class UInventorySlotWidget;

UCLASS()
class GAS_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 外部调用：传入背包组件，自动构建所有格子
	void InitInventory(UInventoryComponent* InInventoryComp);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== 蓝图控件绑定（要求 WB_Bag 设计器里控件同名）=====
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GRID_Items;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true)) // Optional：允许没有关闭按钮
	TObjectPtr<UButton> BTN_Close;

	// 在蓝图里指定格子模板类（默认选你的 WB_InventorySlot）
	UPROPERTY(EditDefaultsOnly, Category = "背包UI")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

private:
	void BuildGrid();
	void RefreshAllSlots();

	UFUNCTION()
	void OnInventoryChanged();

	UFUNCTION()
	void OnCloseClicked();

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgets;
};