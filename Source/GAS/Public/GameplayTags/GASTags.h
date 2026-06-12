#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace GASTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(None);
	
	namespace SetByCaller
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile);
	}
	
	namespace GASAbilities
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivateOnGive);
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ultimate);
		
		namespace Enemy
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack);
		}
	}
	
	namespace Events
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(KillScored);

		namespace Player
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(ComboInput);
		}
		
		namespace Enemy
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(EndAttack);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(MeleeTraceHit);
		}
		
		namespace Combo
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(ComboOpen);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(ComboClose);
		}
	}
}