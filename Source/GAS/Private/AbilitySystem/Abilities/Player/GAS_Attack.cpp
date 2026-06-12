#include "AbilitySystem/Abilities/Player/GAS_Attack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameplayTags/GASTags.h"
#include "GameplayEffect.h"


//基础能力框架
UGAS_Attack::UGAS_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = true;
}

bool UGAS_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return false;

	// 非连招窗口期：直接禁止激活，解决鬼畜乱按
	if (AttackBlockTag.IsValid() && ASC->HasMatchingGameplayTag(AttackBlockTag)) return false;

	return true;
}

void UGAS_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	

	//连击逻辑
	if (CurrentComboIndex >= ComboMontages.Num())
	{
		CurrentComboIndex = 0;
	}

	// 如果当前没有可用蒙太奇，直接结束
	if (!ComboMontages.IsValidIndex(CurrentComboIndex) || !ComboMontages[CurrentComboIndex])
	{
		EndAbilityAndReset();
		return;
	}

	// 设置过渡保护标志，防止 CleanupAllTasks 时触发 OnMontageEnded 导致 Ability 被错误结束
	bIsTransitioningCombo = true;

	SetAttackBlockState(true);
	CleanupAllTasks();
	PlayComboMontage(CurrentComboIndex);
    
	// 索引递增，为下一段做准备
	CurrentComboIndex++;
    
	bIsTransitioningCombo = false;
}


void UGAS_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetFullState();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGAS_Attack::CleanupAllTasks()
{
	if (CurrentMontageTask) 
	{ 
		// 关键修复：先清除委托再 EndTask，防止触发 OnMontageEnded 导致 CurrentComboIndex 被重置
		CurrentMontageTask->OnCompleted.Clear();
		CurrentMontageTask->OnBlendOut.Clear();
		CurrentMontageTask->OnInterrupted.Clear();
		CurrentMontageTask->OnCancelled.Clear();
		CurrentMontageTask->EndTask(); 
		CurrentMontageTask = nullptr; 
	}
	if (WaitComboOpenTask) { WaitComboOpenTask->EndTask(); WaitComboOpenTask = nullptr; }
	if (WaitComboCloseTask) { WaitComboCloseTask->EndTask(); WaitComboCloseTask = nullptr; }
	if (WaitHitEventTask) { WaitHitEventTask->EndTask(); WaitHitEventTask = nullptr; }
}

void UGAS_Attack::EndAbilityAndReset()
{
	ResetFullState();
	K2_EndAbility();
}

void UGAS_Attack::ResetFullState()
{
	SetAttackBlockState(false);
	bComboWindowOpen = false;
	bComboWindowMissed = false;
	CleanupAllTasks();
}

// ==============================================
// 板块2：攻击吸附系统（完全独立，不依赖其他模块）
// ==============================================
AActor* UGAS_Attack::FindAdhesionTarget() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !GetWorld()) return nullptr;

	FVector Loc = Avatar->GetActorLocation();
	FVector Fwd = Avatar->GetActorForwardVector();
	AActor* Best = nullptr; float BestScore = FLT_MAX;
	TArray<FOverlapResult> Res;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(AdhesionRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Adhesion), false, Avatar);

	if (GetWorld()->OverlapMultiByObjectType(Res, Loc, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn), Sphere, Params))
	{
		for (auto& R : Res)
		{
			AActor* T = R.GetActor();
			if (!T || T == Avatar) continue;

			if (AdhesionTargetFilterTag.IsValid())
			{
				auto* ASI = Cast<IAbilitySystemInterface>(T);
				if (!ASI || !ASI->GetAbilitySystemComponent() || !ASI->GetAbilitySystemComponent()->HasMatchingGameplayTag(AdhesionTargetFilterTag))
					continue;
			}

			FVector ToT = T->GetActorLocation() - Loc; ToT.Z = 0;
			float Dist = ToT.Size();
			if (Dist > AdhesionRadius) continue;

			float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(FVector(Fwd.X,Fwd.Y,0).GetSafeNormal(), ToT.GetSafeNormal()), -1,1)));
			if (Angle > AdhesionMaxAngle) continue;

			float Score = Angle*1000 + Dist;
			if (Score < BestScore) { BestScore = Score; Best = T; }
		}
	}
	return Best;
}

void UGAS_Attack::PerformAttackAdhesion(AActor* Target)
{
	if (!Target) return;
	AActor* Avatar = GetAvatarActorFromActorInfo();
	ACharacter* Char = Cast<ACharacter>(Avatar);
	if (!Char) return;

	if (bInstantRotationToTarget)
	{
		FVector Dir = Target->GetActorLocation() - Avatar->GetActorLocation(); Dir.Z = 0;
		Avatar->SetActorRotation(Dir.GetSafeNormal().Rotation());
	}

	if (bUsePositionAdhesion)
	{
		FVector ToT = Target->GetActorLocation() - Avatar->GetActorLocation(); ToT.Z = 0;
		float D = ToT.Size();
		if (D > AdhesionDesiredDistance)
		{
			float Move = FMath::Min(D - AdhesionDesiredDistance, AdhesionMaxMoveDistance);
			if (Move > KINDA_SMALL_NUMBER)
			{
				FVector Dest = Avatar->GetActorLocation() + ToT.GetSafeNormal() * Move;
				if (AdhesionMoveDuration <= KINDA_SMALL_NUMBER)
					Avatar->SetActorLocation(Dest, false, nullptr, ETeleportType::TeleportPhysics);
				else
					Char->LaunchCharacter(ToT.GetSafeNormal() * (Move / AdhesionMoveDuration), true, false);
			}
		}
	}
}

bool UGAS_Attack::TryAttackAdhesion()
{
	if (auto* T = FindAdhesionTarget()) { PerformAttackAdhesion(T); return true; }
	return false;
}



// 板块3：连击系统 + 输入阻塞（核心连招判断）
void UGAS_Attack::SetAttackBlockState(bool bBlock)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !AttackBlockTag.IsValid()) return;

	if (bBlock) ASC->AddLooseGameplayTag(AttackBlockTag);
	else ASC->RemoveLooseGameplayTag(AttackBlockTag);
}

void UGAS_Attack::PlayComboMontage(int32 Index)
{   
	if (bEnableAttackAdhesion)
	{
		TryAttackAdhesion();
	}
	
	if (!ComboMontages.IsValidIndex(Index) || !ComboMontages[Index]) return;

	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, ComboMontages[Index], 1.f, NAME_None, true, 1.f, 0.f, false);

	CurrentMontageTask->OnCompleted.AddDynamic(this, &UGAS_Attack::OnMontageEnded);
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UGAS_Attack::OnMontageEnded);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UGAS_Attack::OnMontageEnded);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &UGAS_Attack::OnMontageEnded);
	CurrentMontageTask->ReadyForActivation();

	BP_OnComboSegmentStart(Index);

	// 监听连招窗口
	WaitComboOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GASTags::Events::Combo::ComboOpen, nullptr, false, true);
	WaitComboOpenTask->EventReceived.AddDynamic(this, &UGAS_Attack::OnComboWindowOpen);
	WaitComboOpenTask->ReadyForActivation();

	WaitComboCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GASTags::Events::Combo::ComboClose, nullptr, false, true);
	WaitComboCloseTask->EventReceived.AddDynamic(this, &UGAS_Attack::OnComboWindowClose);
	WaitComboCloseTask->ReadyForActivation();

	// 绑定伤害检测（板块4）
	if (AttackHitEventTag.IsValid())
	{
		WaitHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AttackHitEventTag, nullptr, false, true);
		WaitHitEventTask->EventReceived.AddDynamic(this, &UGAS_Attack::OnAttackHitDetect);
		WaitHitEventTask->ReadyForActivation();
	}
}

void UGAS_Attack::OnComboWindowOpen(FGameplayEventData Payload)
{
	bComboWindowOpen = true;
	bComboWindowMissed = false;
	SetAttackBlockState(false); // 窗口期解锁，可以打下一段
}

void UGAS_Attack::OnComboWindowClose(FGameplayEventData Payload)
{
	bComboWindowOpen = false;
	// 【核心修复】连击窗口关闭 = 玩家错过衔接时机，连击链断裂，下次攻击必须从第一段开始
	bComboWindowMissed = true;
	SetAttackBlockState(true); // 窗口期关闭，禁止攻击
}

void UGAS_Attack::OnMontageEnded()
{
	if (bIsTransitioningCombo) return;
	if (bComboWindowMissed || bComboWindowOpen)
	{
		CurrentComboIndex = 0;
	}
    
	EndAbilityAndReset();
}

// ==============================================
// 板块4：伤害检测与命中反馈
// ==============================================
void UGAS_Attack::OnAttackHitDetect(FGameplayEventData Payload)
{
	PerformHitBoxCheck();
}


void UGAS_Attack::PerformHitBoxCheck()
{
	// 基础合法性检查
	UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!World || !AvatarActor || !OwnerASC || !DamageEffectClass)
	{
		return;
	}

	// 获取角色网格体与武器插槽
	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	USkeletalMeshComponent* MeshComp = Character ? Character->GetMesh() : nullptr;
	if (!MeshComp || !MeshComp->DoesSocketExist(WeaponSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("武器插槽不存在！请检查插槽名称：%s"), *WeaponSocketName.ToString());
		return;
	}

	// 核心：获取武器插槽的位置和旋转
	const FVector SocketLocation = MeshComp->GetSocketLocation(WeaponSocketName);
	const FRotator SocketRotation = MeshComp->GetSocketRotation(WeaponSocketName);
	const FTransform HitBoxTransform = FTransform(SocketRotation, SocketLocation);

	// 碰撞参数（修复：标准UE写法，自动忽略自身）
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WeaponHitBox), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	// 存储重叠结果
	TArray<FOverlapResult> OverlapResults;

	// 执行武器碰撞盒检测
	const bool bHit = World->OverlapMultiByChannel(
		OverlapResults,
		SocketLocation,
		SocketRotation.Quaternion(),
		HitBoxCollisionChannel,
		FCollisionShape::MakeBox(WeaponHitBoxExtent),
		QueryParams
	);

	// 无命中直接返回
	if (!bHit || OverlapResults.IsEmpty())
	{
		if (bDrawDebugHitBox)
		{
			// 修复：替换 ToQuat() 为 GetRotation()
			DrawDebugBox(World, SocketLocation, WeaponHitBoxExtent, HitBoxTransform.GetRotation(), FColor::Red, false, 2.f, 0, 1.f);
		}
		return;
	}

	// 处理命中目标
	TArray<AActor*> FinalHitActors;
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor) continue;

		FinalHitActors.Add(HitActor);

		// 应用伤害GameplayEffect
		IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HitActor);
		if (TargetASI && TargetASI->GetAbilitySystemComponent())
		{
			UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
			FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
			EffectContext.AddInstigator(AvatarActor, AvatarActor);
			
			OwnerASC->ApplyGameplayEffectToTarget(
				DamageEffectClass->GetDefaultObject<UGameplayEffect>(),
				TargetASC,
				1.f,
				EffectContext
			);
		}

		// 发送受击事件
		SendHitReactEventToActor({HitActor});
	}

	// 调用蓝图命中事件
	if (!FinalHitActors.IsEmpty())
	{
		BP_OnHitBoxHit(FinalHitActors);
	}

	// 调试绘制
	if (bDrawDebugHitBox)
	{
		DrawDebugBox(World, SocketLocation, WeaponHitBoxExtent, HitBoxTransform.GetRotation(), 
			FinalHitActors.Num() > 0 ? FColor::Green : FColor::Red, false, 2.f, 0, 1.f);
	}
}


void UGAS_Attack::SendHitReactEventToActor(const TArray<AActor*> ActorsHit)
{
	for (auto* A : ActorsHit)
	{
		FGameplayEventData D;
		D.Instigator = GetAvatarActorFromActorInfo();
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(A, GASTags::Events::Enemy::HitReact, D);
	}
}