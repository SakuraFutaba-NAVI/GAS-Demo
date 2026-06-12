#include "UI/GAS_WidgetComponent.h"

#include "AbilitySystem/GAS_AbilitySystemComponent.h"
#include "AbilitySystem/GAS_AttributeSet.h"
#include "Blueprint/WidgetTree.h"
#include "Character/GAS_BaseCharacter.h"
#include "UI/GAS_AttributeWidget.h"


void UGAS_WidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 【1】尝试立即初始化GAS相关数据
	InitAbilitySystemData();
	
	if (!IsASCInitialized())
	{
		// 绑定到角色的ASC初始化完成事件,当角色的AbilitySystemComponent初始化完毕后会触发此回调
		GASCharacter->OnASCInitialized.AddDynamic(this, &ThisClass::OnASCInitialized);
	    return;
	}
	InitializeAttributeDelegate();
}


// 【初始化GAS数据】获取角色、属性集和ASC的引用
void UGAS_WidgetComponent::InitAbilitySystemData()
{
	// 【1】获取所属角色【2】获取角色的属性集【3】获取角色的能力系统组件
	GASCharacter = Cast<AGAS_BaseCharacter>(GetOwner());
	AttributeSet = Cast<UGAS_AttributeSet>(GASCharacter->GetAttributeSet());
	AbilitySystemComponent = Cast<UGAS_AbilitySystemComponent>(GASCharacter->GetAbilitySystemComponent());
}


// 【检查ASC初始化状态】验证关键指针是否有效
bool UGAS_WidgetComponent::IsASCInitialized() const
{
	// 同时检查ASC和AttributeSet是否都有效
	return AbilitySystemComponent.IsValid() && AttributeSet.IsValid();
}


void UGAS_WidgetComponent::InitializeAttributeDelegate()
{
	if (!AttributeSet->bAttributesInitialized)
	{
		AttributeSet->OnAttributesInitialized.AddDynamic(this, &ThisClass::BindToAttributeChanges);
	}
	else
	{
		BindToAttributeChanges();
	}
}


// 【ASC初始化回调】角色ASC就绪后触发
void UGAS_WidgetComponent::OnASCInitialized(UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	// 【1】缓存ASC引用（基类指针转派生类）
	AbilitySystemComponent = Cast<UGAS_AbilitySystemComponent>(ASC);
	
	// 【2】缓存AttributeSet引用
	AttributeSet = Cast<UGAS_AttributeSet>(AS);
	
	if (!IsASCInitialized()) return;
	InitializeAttributeDelegate();
}


void UGAS_WidgetComponent::BindToAttributeChanges()
{
	for (const TTuple<FGameplayAttribute, FGameplayAttribute>& Pair : AttributeMap)
	{
		BindWidgetToAttributeChanges(GetUserWidgetObject(), Pair);
		
		GetUserWidgetObject()->WidgetTree->ForEachWidget([this, &Pair](UWidget* ChildWidget)
		{
			BindWidgetToAttributeChanges(ChildWidget, Pair);
		});
	}
}


void UGAS_WidgetComponent::BindWidgetToAttributeChanges(UWidget* WidgetObject,const TTuple<FGameplayAttribute, FGameplayAttribute>& Pair) const
{
	UGAS_AttributeWidget* AttributeWidget = Cast<UGAS_AttributeWidget>(WidgetObject);
	if (!IsValid(AttributeWidget)) return;
	if (!AttributeWidget->MatchesAttributes(Pair)) return;
		
	AttributeWidget->OnAttributeChange(Pair, AttributeSet.Get());
		
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Key).AddLambda([this, AttributeWidget, &Pair](const FOnAttributeChangeData& AttributeChangeData)
	{
		AttributeWidget->OnAttributeChange(Pair, AttributeSet.Get());
	});
}
