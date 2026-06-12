#include "GameplayTags/GASTags.h"

namespace GASTags
{
	UE_DEFINE_GAMEPLAY_TAG(None,"GASTags.None");
	
	namespace SetByCaller
	{
		UE_DEFINE_GAMEPLAY_TAG(Projectile,"GASTags.SetByCaller.Projectile");
	}
	
	namespace GASAbilities
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActivateOnGive,"GASTags.GASAbilities.ActivateOnGive","激活时触发能力标签");
		
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack,"GASTags.GASAbilities.Attack","攻击能力标签");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill,"GASTags.GASAbilities.Skill","技能能力标签");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ultimate,"GASTags.GASAbilities.Ultimate","大招能力标签");
		
		namespace Enemy
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack,"GASTags.GASAbilities.Enemy.Attack","攻击能力标签");
		}
	}
	
	namespace Events
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(KillScored,"GASTags.Events.KillScored","击杀得分标签");
		
		namespace Player
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact,"GASTags.Events.Player.HitReact","受击能力标签");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death,"GASTags.Events.Player.Death","死亡能力标签");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ComboInput,"GASTags.Events.Player.ComboInput","连招输入能力标签");
		}
		
		namespace Enemy
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact,"GASTags.Events.Enemy.HitReact","受击能力标签");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(EndAttack,"GASTags.Events.Enemy.EndAttack","结束攻击能力标签");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(MeleeTraceHit,"GASTags.Events.Enemy.MeleeTraceHit","近战敌人攻击能力标签");
		}
		
		namespace Combo
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ComboOpen,"GASTags.Events.Combo.ComboOpen","开启连招能力标签");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(ComboClose,"GASTags.Events.Combo.ComboClose","关闭连招能力标签");
		}
	}
}

