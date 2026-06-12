#include "Notify/GAS_MeleeAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "KismetTraceUtils.h"
#include "Character/GAS_PlayerCharacter.h"
#include "GameplayTags/GASTags.h"
#include "Kismet/KismetMathLibrary.h"

void UGAS_MeleeAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!IsValid(MeshComp)) return;
	if (!IsValid(MeshComp->GetOwner())) return;
	
	TArray<FHitResult> Hits = PerformSphereTrace(MeshComp);
    SendEventsToActors(MeshComp, Hits);
}


//执行球形追踪
TArray<FHitResult> UGAS_MeleeAttack::PerformSphereTrace(USkeletalMeshComponent* MeshComp) const
{
	TArray<FHitResult> OutHits;
	
	const FTransform SocketTransform = MeshComp->GetSocketTransform(SocketName);
	const FVector Start = SocketTransform.GetLocation();
	const FVector ExtendedSocketDirection = UKismetMathLibrary::GetForwardVector(SocketTransform.GetRotation().Rotator()) * SocketExtensionOffset;
	const FVector End = Start - ExtendedSocketDirection;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(MeshComp->GetOwner());
	
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
	
	UWorld* World = GEngine->GetWorldFromContextObject(MeshComp, EGetWorldErrorMode::LogAndReturnNull);
    if (!IsValid(World)) return OutHits;
	bool const bHit = World->SweepMultiByChannel(
		OutHits, 
		Start,
		End, 
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(SphereTraceRadius),
		Params,
		ResponseParams);
    
	if (bDrawDebug)
	{
		DrawDebugSphereTraceMulti(World, Start, End, SphereTraceRadius, EDrawDebugTrace::ForDuration, bHit, OutHits, FColor::Red, FColor::Green, 5.f);
	}
	
	return OutHits;
}


//将事件发送给角色
void UGAS_MeleeAttack::SendEventsToActors(USkeletalMeshComponent* MeshComp, const TArray<FHitResult>& Hits) const
{
	for (const FHitResult& Hit : Hits)
	{
		AGAS_PlayerCharacter* PlayerCharacter = Cast<AGAS_PlayerCharacter>(Hit.GetActor());
		if (!IsValid(PlayerCharacter)) continue;
		if (!PlayerCharacter->IsAlive()) continue;
		const UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
		if (!IsValid(ASC)) continue;
		
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddHitResult(Hit);
		
		FGameplayEventData Payload;
		Payload.Target = PlayerCharacter;
		Payload.ContextHandle = ContextHandle;
		Payload.Instigator = MeshComp->GetOwner();
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), GASTags::Events::Enemy::MeleeTraceHit, Payload);
	}
}
