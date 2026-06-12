#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GAS_Projectile.generated.h"

class UGameplayEffect;
class UProjectileMovementComponent;


UCLASS()
class GAS_API AGAS_Projectile : public AActor
{
	GENERATED_BODY()

public:
	AGAS_Projectile();
	
	virtual void NotifyActorBeginOverlap(AActor* Actor) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Dmage", meta = (ExposeOnSpawn, ClampMin = "0.0"))
	float Damage{10.f};
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|Projectile")
	void SpawnImpactEffect();
	
private:
	UPROPERTY(VisibleAnywhere, Category = "GAS|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Damage")
    TSubclassOf<UGameplayEffect> DamageEffect;
};
