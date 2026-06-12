#include "AbilitySystem/Abilities/Enemy/GAS_HitReact.h"

void UGAS_HitReact::CacheHitDirectionVectors(AActor* Instigator)
{
	//获取受击角色（Avatar）的当前朝向（前向向量）
	AvatarForward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	
	//获取受击角色的世界位置。获取攻击者（Instigator）的世界位置
	const FVector AvatarLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
	const FVector InstigatorLocation = Instigator->GetActorLocation();
	
	//计算从受击者指向攻击者的方向向量
	ToInstigator = InstigatorLocation - AvatarLocation;
	ToInstigator.Normalize();
}
