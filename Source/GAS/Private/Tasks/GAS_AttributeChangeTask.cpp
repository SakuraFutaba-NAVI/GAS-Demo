#include "Tasks/GAS_AttributeChangeTask.h"
#include "AbilitySystemComponent.h"

UGAS_AttributeChangeTask* UGAS_AttributeChangeTask::ListenForAttributeChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute, float& NewValue, float& OldValue)
{
	UGAS_AttributeChangeTask* WaitForAttributeChangeTask = NewObject<UGAS_AttributeChangeTask>();
	WaitForAttributeChangeTask->ASC = AbilitySystemComponent;
	WaitForAttributeChangeTask->AttributeToListenFor = Attribute;
	
	if (!IsValid(AbilitySystemComponent))
	{
		WaitForAttributeChangeTask->RemoveFromRoot();
		return nullptr;
	}
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(WaitForAttributeChangeTask,&UGAS_AttributeChangeTask::AttributeChange);
    return WaitForAttributeChangeTask;
}


void UGAS_AttributeChangeTask::EndTask()
{
	if (ASC.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeToListenFor).RemoveAll(this);
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}


void UGAS_AttributeChangeTask::AttributeChange(const FOnAttributeChangeData& Data)
{
	OnAttributeChange.Broadcast(Data.Attribute, Data.NewValue,Data.OldValue);
}
