#include "Bag/InventoryComponent.h"

#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"


UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	// 初始化空背包
	InventorySlots.SetNum(InventoryCapacity);
}

FInventoryItem UInventoryComponent::FindItemDataByID(int32 ItemID) const
{
	if (!ItemDataTable) return FInventoryItem();

	static const FString Context = TEXT("InventoryComponent");
	TArray<FInventoryItem*> AllItems;
	ItemDataTable->GetAllRows<FInventoryItem>(Context, AllItems);

	for (auto* Item : AllItems)
	{
		if (Item && Item->ItemID == ItemID)
		{
			return *Item;
		}
	}
	return FInventoryItem();
}

int32 UInventoryComponent::FindStackableSlot(int32 ItemID) const
{
	for (int32 i = 0; i < InventorySlots.Num(); i++)
	{
		const auto& Slot = InventorySlots[i];
		if (!Slot.IsEmpty() && Slot.ItemData.ItemID == ItemID && Slot.ItemCount < Slot.ItemData.MaxStack)
		{
			return i;
		}
	}
	return -1;
}

int32 UInventoryComponent::FindEmptySlot() const
{
	for (int32 i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].IsEmpty())
		{
			return i;
		}
	}
	return -1;
}

bool UInventoryComponent::AddItem(int32 ItemID, int32 Count)
{
	if (Count <= 0 || !ItemDataTable) return false;

	FInventoryItem ItemData = FindItemDataByID(ItemID);
	if (ItemData.ItemID == 0) return false;

	int32 RemainCount = Count;
	while (RemainCount > 0)
	{
		// 优先堆叠
		int32 StackIndex = FindStackableSlot(ItemID);
		if (StackIndex != -1)
		{
			auto& Slot = InventorySlots[StackIndex];
			int32 AddNum = FMath::Min(RemainCount, Slot.ItemData.MaxStack - Slot.ItemCount);
			Slot.ItemCount += AddNum;
			RemainCount -= AddNum;
		}
		else
		{
			// 无堆叠位 → 找空槽
			int32 EmptyIndex = FindEmptySlot();
			if (EmptyIndex == -1) break;

			auto& Slot = InventorySlots[EmptyIndex];
			Slot.ItemData = ItemData;
			Slot.ItemCount = FMath::Min(RemainCount, ItemData.MaxStack);
			RemainCount -= Slot.ItemCount;
		}
	}
	if (RemainCount < Count) OnInventoryChanged.Broadcast();
	
	return RemainCount < Count;
	
}

bool UInventoryComponent::RemoveItem(int32 ItemID, int32 Count)
{
	if (Count <= 0) return false;

	int32 RemainCount = Count;
	for (auto& Slot : InventorySlots)
	{
		if (RemainCount <= 0) break;
		if (Slot.IsEmpty() || Slot.ItemData.ItemID != ItemID) continue;

		int32 RemoveNum = FMath::Min(RemainCount, Slot.ItemCount);
		Slot.ItemCount -= RemoveNum;
		RemainCount -= RemoveNum;
	}
	OnInventoryChanged.Broadcast();
	
	return RemainCount < Count;
}

void UInventoryComponent::ExecuteItemUse(const FInventoryItem& Item)
{
	if (!Item.bCanUse) return;

	// 联动你的GAS系统：应用GameplayEffect
	AActor* Owner = GetOwner();
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner);
	if (!ASI || !Item.UseGameplayEffect) return;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;

	FGameplayEffectSpecHandle GESpec = ASC->MakeOutgoingSpec(
		Item.UseGameplayEffect,
		1.0f,
		ASC->MakeEffectContext()
	);

	if (GESpec.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*GESpec.Data);
	}
}

bool UInventoryComponent::UseItemByIndex(int32 SlotIndex)
{
	if (!InventorySlots.IsValidIndex(SlotIndex)) return false;

	auto& Slot = InventorySlots[SlotIndex];
	if (Slot.IsEmpty() || !Slot.ItemData.bCanUse) return false;

	// 执行使用逻辑
	ExecuteItemUse(Slot.ItemData);

	// 消耗品使用后减1
	if (Slot.ItemData.ItemType == EInventoryItemType::Consumable)
	{
		Slot.ItemCount--;
		if (Slot.ItemCount <= 0)
		{
			Slot = FInventorySlot();
		}
	}
	OnInventoryChanged.Broadcast(); 
	
	return true;
}

void UInventoryComponent::ClearInventory()
{
	InventorySlots.Empty();
	InventorySlots.SetNum(InventoryCapacity);
	OnInventoryChanged.Broadcast();
}
