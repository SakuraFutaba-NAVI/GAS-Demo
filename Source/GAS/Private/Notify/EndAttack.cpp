#include "Notify/EndAttack.h"

#include "Character/GAS_BaseCharacter.h"

void UAN_EndAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 获取角色本体
	AGAS_BaseCharacter* BaseChar = Cast<AGAS_BaseCharacter>(MeshComp->GetOwner());
	if (IsValid(BaseChar))
	{
		// 直接调用结束攻击、解锁移动
		BaseChar->EndAttack();
	}
}