#include "AbilitySystem/GAS_AbilitySystemComponent.h"

#include "GameplayTags/GASTags.h"


// 当某个 GameplayAbility 被"授予"（Give）到该 ASC 时触发。可用于实现"获得即激活"的被动技能逻辑（如出生自带的光环、常驻 Buff 等）
void UGAS_AbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);
	
	HandleAutoActivatedAbility(AbilitySpec);
}

//当 ActivatableAbilities 列表从服务端网络复制到客户端后触发
void UGAS_AbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	
	FScopedAbilityListLock ActivateScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		HandleAutoActivatedAbility(AbilitySpec);
	}
}

void UGAS_AbilitySystemComponent::SetAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
	if (IsValid(GetAvatarActor()) && !GetAvatarActor()->HasAuthority()) return;
	
	if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromClass(AbilityClass))
	{
		AbilitySpec->Level += Level;
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UGAS_AbilitySystemComponent::AddToAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
	if (IsValid(GetAvatarActor()) && !GetAvatarActor()->HasAuthority()) return;
	
	if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromClass(AbilityClass))
	{
		AbilitySpec->Level = Level;
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}


void UGAS_AbilitySystemComponent::HandleAutoActivatedAbility(const FGameplayAbilitySpec& AbilitySpec)
{
	if (!IsValid(AbilitySpec.Ability)) return;
	
	for (const FGameplayTag& Tag : AbilitySpec.Ability->GetAssetTags())
	{
		if (Tag.MatchesTagExact(GASTags::GASAbilities::ActivateOnGive))
		{
			TryActivateAbility(AbilitySpec.Handle);
			return;
		}
	}
}
