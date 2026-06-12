#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GAS_MeleeAttack.generated.h"

UCLASS()
class GAS_API UGAS_MeleeAttack : public UAnimNotifyState
{
	GENERATED_BODY()

private:
	// 攻击检测骨骼插槽
	UPROPERTY(EditAnywhere, Category = "攻击检测")
	FName SocketName = TEXT("FX_Trail_01_R");

	// 插槽检测延伸长度
	UPROPERTY(EditAnywhere, Category = "攻击检测")
	float SocketExtensionOffset = {40.0f};

	// 球形碰撞检测半径
	UPROPERTY(EditAnywhere, Category = "攻击检测")
	float SphereTraceRadius = {60.0f};

	// 调试碰撞绘制开关
	UPROPERTY(EditAnywhere, Category = "调试")
	bool bDrawDebug = true;


	// 逐帧攻击检测核心入口
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;


	// 球形扫描碰撞逻辑
	TArray<FHitResult> PerformSphereTrace(USkeletalMeshComponent* MeshComp) const;

	// 命中发送GAS伤害事件
	void SendEventsToActors(USkeletalMeshComponent* MeshComp, const TArray<FHitResult>& Hits) const;
};