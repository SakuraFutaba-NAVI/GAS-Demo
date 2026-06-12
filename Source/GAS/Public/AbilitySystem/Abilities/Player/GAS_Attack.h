#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GAS_GameplayAbility.h"
#include "GAS_Attack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UGameplayEffect;
struct FGameplayEventData;

UCLASS()
class GAS_API UGAS_Attack : public UGAS_GameplayAbility
{
	GENERATED_BODY()

public:
	UGAS_Attack();

	// 板块1：基础能力框架（生命周期）
	// ==============================================
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	// 基础通用工具函数
	void CleanupAllTasks();
	void EndAbilityAndReset();
	void ResetFullState();

	// ==============================================
	// 板块2：攻击吸附系统（独立模块，开关控制）
	// ==============================================
public:
	UFUNCTION(BlueprintCallable, Category = "GAS|Adhesion")
	AActor* FindAdhesionTarget() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Adhesion")
	void PerformAttackAdhesion(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "GAS|Adhesion")
	bool TryAttackAdhesion();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion")
	bool bEnableAttackAdhesion = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion", meta = (EditCondition = "bEnableAttackAdhesion", ClampMin = "0"))
	float AdhesionRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion", meta = (EditCondition = "bEnableAttackAdhesion", ClampMin = "0", ClampMax = "180"))
	float AdhesionMaxAngle = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion", meta = (EditCondition = "bEnableAttackAdhesion", ClampMin = "0"))
	float AdhesionDesiredDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion", meta = (EditCondition = "bEnableAttackAdhesion", ClampMin = "0"))
	float AdhesionMaxMoveDistance = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion", meta = (EditCondition = "bEnableAttackAdhesion"))
	bool bInstantRotationToTarget = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion", meta = (EditCondition = "bEnableAttackAdhesion"))
	bool bUsePositionAdhesion = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion", meta = (EditCondition = "bEnableAttackAdhesion && bUsePositionAdhesion", ClampMin = "0"))
	float AdhesionMoveDuration = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Adhesion", meta = (EditCondition = "bEnableAttackAdhesion"))
	FGameplayTag AdhesionTargetFilterTag;

	// ==============================================
	// 板块3：连击系统 + 输入阻塞（核心连招逻辑）
	// ==============================================
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|Combo")
	void BP_OnComboSegmentStart(int32 ComboIndex);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Combo")
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Tags")
	FGameplayTag AttackBlockTag; // 攻击阻塞Tag，禁止非窗口期乱按

private:
	int32 CurrentComboIndex = 0;
	bool bComboWindowOpen = false;
	bool bComboWindowMissed = false;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask = nullptr;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitComboOpenTask = nullptr;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitComboCloseTask = nullptr;

	void PlayComboMontage(int32 Index);
	void SetAttackBlockState(bool bBlock); // 阻塞/解锁攻击输入

	UFUNCTION() void OnComboWindowOpen(FGameplayEventData Payload);
	UFUNCTION() void OnComboWindowClose(FGameplayEventData Payload);
	UFUNCTION() void OnMontageEnded();


	// 板块4：伤害检测与命中反馈（受击框、GE伤害、受击事件）
public:
	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	void SendHitReactEventToActor(const TArray<AActor*> ActorsHit);

	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|Damage")
	void BP_OnHitBoxHit(const TArray<AActor*>& HitActors);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	float HitBoxRadius = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	float HitBoxForwardOffset = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	float HitBoxElevationOffset = 20.f;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Damage")
	FGameplayTag AttackHitEventTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Damage")
	TEnumAsByte<ECollisionChannel> HitBoxCollisionChannel = ECC_Pawn;

	// 武器插槽名称（在角色/武器网格体上的插槽，如 WeaponSocket/RightHandSocket）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|HitBox|Weapon")
	FName WeaponSocketName = "WeaponSocket";

	// 武器碰撞盒半尺寸（X=长度 Y=宽度 Z=高度，适配刀剑/拳套/斧头）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|HitBox|Weapon", meta = (ClampMin = 0))
	FVector WeaponHitBoxExtent = FVector(150.f, 100.f, 100.f);

	// 调试绘制碰撞盒（测试用）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|HitBox|Weapon")
	bool bDrawDebugHitBox = false;

	
private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitHitEventTask = nullptr;
	
	void PerformHitBoxCheck();
	
	UFUNCTION() 
	void OnAttackHitDetect(FGameplayEventData Payload);
	
	bool bIsTransitioningCombo = false;
};