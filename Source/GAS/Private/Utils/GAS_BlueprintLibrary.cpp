#include "Utils/GAS_BlueprintLibrary.h"


#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/GAS_AttributeSet.h"
#include "Character/GAS_BaseCharacter.h"
#include "Character/GAS_EnemyCharacter.h"
#include "Engine/OverlapResult.h"
#include "GameplayTags/GASTags.h"
#include "Kismet/GameplayStatics.h"


//攻击方向判断
EHitDirection UGAS_BlueprintLibrary::GetHitDirection(const FVector& TargetForward, const FVector& ToInstigator)
{
	const float Dot = FVector::DotProduct(TargetForward, ToInstigator);
	if (Dot<-0.5f)
	{
		return  EHitDirection::Back;
	}
	if (Dot<-0.5f)
	{
		const FVector Cross = FVector::CrossProduct(TargetForward, ToInstigator);
		if (Cross.Z<0.f)
		{
			return  EHitDirection::Left;
		}
		return EHitDirection::Right;
	}
	return EHitDirection::Forward;
}

//攻击方向名称获取
FName UGAS_BlueprintLibrary::GetHitDirectionName(const EHitDirection& HitDirection)
{
	switch (HitDirection)
	{
		case EHitDirection::Back:
		return FName("Back");
		case EHitDirection::Left:
		return FName("Left");
		case EHitDirection::Right:
		return FName("Right");
		case EHitDirection::Forward:
		return FName("Forward");
		default:return FName("None");
	}
}

//搜索目标
FClosestActorWithTagResult UGAS_BlueprintLibrary::FindClosestActorWithTag(const UObject* WorldContextObject,const FVector& Origin, const FName& Tag)
{
	TArray<AActor*> ActorsWithTag;
    UGameplayStatics::GetAllActorsWithTag(WorldContextObject, Tag, ActorsWithTag);
	
	float  ClosestDistance = TNumericLimits<float>::Max();
	AActor* ClosestActor = nullptr;
	
	for (AActor* Actor : ActorsWithTag)
	{
		if (!IsValid(Actor)) continue;
		
		AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(Actor);
		if (!IsValid(BaseCharacter) || !BaseCharacter->IsAlive()) continue;
			
		const float Distance = FVector::Dist(Origin,Actor->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestActor = Actor;
		}
	}
	FClosestActorWithTagResult Result;
	Result.Actor = ClosestActor;
	Result.Distance = ClosestDistance;
	
	return Result;
}


//角色受击/死亡
void UGAS_BlueprintLibrary::SendDamageEventToPlayer(AActor* Target, const TSubclassOf<UGameplayEffect>& DamageEffect,UPARAM(ref) FGameplayEventData& Payload, const FGameplayTag& DataTag, float Damage, const FGameplayTag& EventTagOverride, UObject* OptionalParticleSystem)
{
	AGAS_BaseCharacter* PlayerCharacter = Cast<AGAS_BaseCharacter>(Target);
	if (!IsValid(PlayerCharacter)) return;
	if (!PlayerCharacter->IsAlive()) return;
	
	FGameplayTag EventTag;
	if (!EventTagOverride.MatchesTagExact(GASTags::None))
	{
		EventTag = EventTagOverride;
	}
	else
	{
		UGAS_AttributeSet* AttributeSet = Cast<UGAS_AttributeSet>(PlayerCharacter->GetAttributeSet());
		if (!IsValid(AttributeSet)) return;
	
		const bool blethal = AttributeSet->GetHealth() - Damage <= 0.0f;
		EventTag = blethal ? GASTags::Events::Player::Death : GASTags::Events::Player::HitReact;
	}
	
	
	Payload.OptionalObject = OptionalParticleSystem;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(PlayerCharacter, EventTag, Payload);
    
	UAbilitySystemComponent* TargetASC = PlayerCharacter->GetAbilitySystemComponent();
    if (!IsValid(TargetASC)) return;
	
	FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, 1.f, ContextHandle);
	
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DataTag, -Damage);

    TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}


void UGAS_BlueprintLibrary::SendDamageEventToPlayers(TArray<AActor*> Targets,const TSubclassOf<UGameplayEffect>& DamageEffect, FGameplayEventData& Payload, const FGameplayTag& DataTag,float Damage, const FGameplayTag& EventTagOverride, UObject* OptionalParticleSystem)
{
	for (AActor* Target : Targets)
	{
		SendDamageEventToPlayer(Target, DamageEffect, Payload, DataTag, Damage, EventTagOverride, OptionalParticleSystem);
	}
}


//碰撞框检测
TArray<AActor*> UGAS_BlueprintLibrary::HitBoxOverlapTest(AActor* AvatarActor, float HitBoxRadius, float HitBoxForwardOffset, float HitBoxElevationOffset, bool bDrawDebugs)
{
	if (!IsValid(AvatarActor)) return TArray<AActor*>();
	
	//配置碰撞查询参数，将自身加入忽略列表
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);
	
	//默认忽略所有通道，仅对Pawn通道产生Block响应，这意味着只有带有Pawn碰撞配置的Actor会被检测到
	//创建球形碰撞体，半径由HitBoxRadius决定。计算命中盒的前向偏移向量（基于角色朝向）
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn,ECR_Block);
	
	//准备存储重叠检测结果的数组
	TArray<FOverlapResult> OverlapResults;
	//创建球形碰撞体，半径由HitBoxRadius决定
	FCollisionShape Sphere = FCollisionShape::MakeSphere(HitBoxRadius);
	//计算命中盒的前向偏移向量（基于角色朝向）
	const FVector Forward = AvatarActor->GetActorForwardVector() * HitBoxForwardOffset;
	const FVector HitBoxLocation = AvatarActor->GetActorLocation() + Forward + FVector(0.f,0.f,HitBoxElevationOffset);
	
	
	//执行球形重叠检测（多对象），使用Visibility通道
	UWorld* World = GEngine->GetWorldFromContextObject(AvatarActor, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World)) return TArray<AActor*>();
	World->OverlapMultiByChannel(
		OverlapResults,          //输出参数，存储所有检测到的重叠结果
		HitBoxLocation,             //检测球体的中心位置
		FQuat::Identity,            //无旋转（球体旋转不影响结果）
		ECC_Visibility,             //碰撞检测通道（Visibility通道）
		Sphere,                     //球形碰撞体配置（半径等）
		QueryParams,                //查询参数（如忽略自身等）
		ResponseParams);            //执行球形重叠检测（多对象），使用Visibility通道

	TArray<AActor*>ActorsHit;
	for (const FOverlapResult& Result : OverlapResults)
	{
		AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(Result.GetActor());
		if (!IsValid(BaseCharacter)) continue;
		if (!BaseCharacter->IsAlive()) continue;
		ActorsHit.Add(Result.GetActor());
	}
	
	if (bDrawDebugs)
	{
		DrawHitBoxOverlapDebugs(AvatarActor,OverlapResults, HitBoxLocation, HitBoxRadius);
	}
	
	return ActorsHit;
}


//绘制调试框
void UGAS_BlueprintLibrary::DrawHitBoxOverlapDebugs(const UObject* WorldContextObject,const TArray<FOverlapResult> OverlapResults,const FVector& HitBoxLocation, float HitBoxRadius)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World)) return;
	
	DrawDebugSphere(World,HitBoxLocation,HitBoxRadius,16,FColor::Red,false,3.f);
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		if (IsValid(Result.GetActor()))
		{
			FVector DebugLocation = Result.GetActor()->GetActorLocation();
			DebugLocation.Z += 100.f;
			DrawDebugSphere(World, DebugLocation,30.f,10,FColor::Green,false,3.f);
		}
	}
}


//击退效果
TArray<AActor*> UGAS_BlueprintLibrary::ApplyKnockback(AActor* AvatarActor, const TArray<AActor*>& HitActors, float InnerRadius,float OtherRadius, float LaunchForceMagnitude, float RotationAngle, bool bDrawDebugs)
{
	for (AActor* HitActor : HitActors)
	{
		ACharacter* HitCharacter = Cast<ACharacter>(HitActor);
		if (!IsValid(HitCharacter) || !IsValid(AvatarActor)) return TArray<AActor*>();
		
		const FVector HitCharacterLocation = HitActor->GetActorLocation();
		const FVector AvatarLocation = AvatarActor->GetActorLocation();
		
		const FVector ToHitActor = HitCharacterLocation - AvatarLocation;
		const float Distance = FVector::Dist(AvatarLocation, HitCharacterLocation);
		
		float LaunchForce = 0.f;
		if (Distance > OtherRadius) continue;
		if (Distance <= InnerRadius)
		{
			LaunchForce = LaunchForceMagnitude;
		}
		else
		{
			const FVector2D FalloffRange(InnerRadius,OtherRadius);
			const FVector2D LaunchForceRange (LaunchForceMagnitude, 0.f);
			LaunchForce = FMath::GetMappedRangeValueClamped(FalloffRange, LaunchForceRange, Distance);
		}
		
		if (bDrawDebugs) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("LaunchForce: %f"), LaunchForce));
	
	    FVector KnockbackForce = ToHitActor.GetSafeNormal();
		KnockbackForce.Z = 0.f;
		
		const FVector Right = KnockbackForce.RotateAngleAxis(90.f, FVector::UpVector);
		KnockbackForce = KnockbackForce.RotateAngleAxis(-RotationAngle, Right) * LaunchForce;
		
		if (bDrawDebugs)
		{
			UWorld* World = GEngine->GetWorldFromContextObject(AvatarActor, EGetWorldErrorMode::LogAndReturnNull);
			DrawDebugDirectionalArrow(World, HitCharacterLocation, HitCharacterLocation + KnockbackForce, 100.f, FColor::Green, false, 3.f);
		}
		
		if (AGAS_EnemyCharacter* EnemyCharacter = Cast<AGAS_EnemyCharacter>(HitActor); IsValid(EnemyCharacter))
		{
			EnemyCharacter->StopMovement();
		}
		
		HitCharacter->LaunchCharacter(KnockbackForce, true, true);
	}
	return HitActors;
}
