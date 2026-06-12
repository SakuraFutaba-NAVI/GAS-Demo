#include "Bag/InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Button.h"
#include "Bag/InventorySlotWidget.h"

void UInventoryWidget::InitInventory(UInventoryComponent* InInventoryComp)
{
    InventoryComp = InInventoryComp;
    BuildGrid();
}

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (BTN_Close)
    {
        BTN_Close->OnClicked.AddDynamic(this, &UInventoryWidget::OnCloseClicked);
    }
}

void UInventoryWidget::NativeDestruct()
{
    // 解绑委托，防止野指针
    if (InventoryComp)
    {
        InventoryComp->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::OnInventoryChanged);
    }
    Super::NativeDestruct();
}

void UInventoryWidget::BuildGrid()
{
    if (!GRID_Items || !InventoryComp || !SlotWidgetClass) return;

    // 清空旧格子
    GRID_Items->ClearChildren();
    SlotWidgets.Empty();

    const int32 Capacity = InventoryComp->InventoryCapacity;
    constexpr int32 ColumnsPerRow = 5; // 每行 5 个

    for (int32 i = 0; i < Capacity; ++i)
    {
        UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
        if (!SlotWidget) continue;

        SlotWidget->Setup(i, InventoryComp);
        SlotWidgets.Add(SlotWidget);

        // 添加到网格：Row = i/5, Column = i%5
        if (UUniformGridSlot* GridSlot = GRID_Items->AddChildToUniformGrid(SlotWidget, i / ColumnsPerRow, i % ColumnsPerRow))
        {
            GridSlot->SetHorizontalAlignment(HAlign_Fill);
            GridSlot->SetVerticalAlignment(VAlign_Fill);
        }
    }

    // 首次刷新
    RefreshAllSlots();

    // 绑定背包数据变更，以后自动刷新
    if (InventoryComp)
    {
        InventoryComp->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);
    }
}

void UInventoryWidget::RefreshAllSlots()
{
    for (auto& InvSlot : SlotWidgets)
    {
        if (InvSlot) InvSlot->Refresh();
    }
}

void UInventoryWidget::OnInventoryChanged()
{
    RefreshAllSlots();
}

void UInventoryWidget::OnCloseClicked()
{
    RemoveFromParent();
}