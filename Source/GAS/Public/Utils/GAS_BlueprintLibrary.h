#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAS_BlueprintLibrary.generated.h"

UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	Left,
	Right,
	Back,
	Forward
};

USTRUCT(BlueprintType)
struct FClosestActorWithTagResult
{
	GENERATED_BODY();
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> Actor;
	
	UPROPERTY(BlueprintReadWrite)
	float Distance{0.f};
};

UCLASS()
class GAS_API UGAS_BlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	//获取攻击方向
	UFUNCTION(BlueprintPure)
	static EHitDirection GetHitDirection(const FVector& TargetForward,const FVector& ToInstigator);

	//获取攻击方向名称
    UFUNCTION(BlueprintPure)
	static FName GetHitDirectionName(const EHitDirection& HitDirection);

	UFUNCTION(BlueprintPure)
	static FClosestActorWithTagResult FindClosestActorWithTag(const UObject* WorldContextObject,const FVector& Origin, const FName& Tag);

    UFUNCTION(BlueprintCallable)
    static void SendDamageEventToPlayer(AActor* Target, const TSubclassOf<UGameplayEffect>& DamageEffect, UPARAM(ref) FGameplayEventData& Payload, const FGameplayTag& DataTag, float Damage, const FGameplayTag& EventTagOverride, UObject* OptionalParticleSystem = nullptr); 

	UFUNCTION(BlueprintCallable)
	static void SendDamageEventToPlayers(TArray<AActor*> Targets, const TSubclassOf<UGameplayEffect>& DamageEffect, UPARAM(ref) FGameplayEventData& Payload, const FGameplayTag& DataTag, float Damage, const FGameplayTag& EventTagOverride, UObject* OptionalParticleSystem = nullptr); 

	
	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	static TArray<AActor*> HitBoxOverlapTest(AActor* AvatarActor, float HitBoxRadius, float HitBoxForwardOffset = 0.f, float HitBoxElevationOffset = 0.f, bool bDrawDebugs = false);

	static void DrawHitBoxOverlapDebugs(const UObject* WorldContextObject, TArray<FOverlapResult> OverlapResults,const FVector& HitBoxLocation, float HitBoxRadius); 

    UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
    static TArray<AActor*> ApplyKnockback(AActor* AvatarActor, const TArray<AActor*>& HitActors, float InnerRadius, float OtherRadius, float LaunchForceMagnitude, float RotationAngle = 45.f, bool bDrawDebugs = false);
};
