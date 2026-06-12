#include "Player/GAS_PlayerState.h"
#include "AbilitySystem/GAS_AbilitySystemComponent.h"
#include "AbilitySystem/GAS_AttributeSet.h"

AGAS_PlayerState::AGAS_PlayerState()
{
	SetNetUpdateFrequency(100.f);//设置网络更新频率为100次每秒，这样可以确保玩家状态在网络上及时更新，减少延迟和卡顿的感觉。

	//创建一个默认的子对象，类型为UAbilitySystemComponent，并命名为"AbilitySystemComponent"。这个组件将用于管理玩家的能力系统。
	AbilitySystemComponent = CreateDefaultSubobject<UGAS_AbilitySystemComponent>("AbilitySystemComponent");

	// 设置AbilitySystemComponent组件为可复制，这样它的状态和属性可以在网络上同步到其他客户端。
	AbilitySystemComponent->SetIsReplicated(true);

	//设置AbilitySystemComponent组件的复制模式为Minimal，这意味着只有必要的属性和状态会被复制到其他客户端，以减少网络带宽的使用。
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	//创建一个默认的子对象，类型为UAttributeSet，并命名为"AttributeSet"。这个组件将用于管理玩家的属性系统。
	AttributeSet = CreateDefaultSubobject<UGAS_AttributeSet>("AttributeSet");
}


UAbilitySystemComponent* AGAS_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;

}
