#include "Bag/InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Bag/InventoryComponent.h"

void UInventorySlotWidget::Setup(int32 InSlotIndex, UInventoryComponent* InInventoryComp)
{
	SlotIndex = InSlotIndex;
	InventoryRef = InInventoryComp;
}

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_Slot)
	{
		BTN_Slot->OnClicked.AddDynamic(this, &UInventorySlotWidget::OnSlotClicked);
	}
}

void UInventorySlotWidget::Refresh()
{
	if (!InventoryRef) return;

	const TArray<FInventorySlot> Slots = InventoryRef->GetAllSlots();
	if (!Slots.IsValidIndex(SlotIndex)) return;

	const FInventorySlot& InvSlot = Slots[SlotIndex];

	if (InvSlot.IsEmpty())
	{
		if (IMG_Icon) IMG_Icon->SetBrushFromTexture(nullptr);
		if (TXT_Count)
		{
			TXT_Count->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		if (IMG_Icon) IMG_Icon->SetBrushFromTexture(InvSlot.ItemData.ItemIcon);
		if (TXT_Count)
		{
			TXT_Count->SetText(FText::AsNumber(InvSlot.ItemCount));
			TXT_Count->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UInventorySlotWidget::OnSlotClicked()
{
	if (!InventoryRef) return;
    
	// 调用 C++ 使用逻辑（里面会自动广播 OnInventoryChanged）
	InventoryRef->UseItemByIndex(SlotIndex);
}