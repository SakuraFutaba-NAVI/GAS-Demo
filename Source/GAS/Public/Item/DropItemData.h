#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DropItemData.generated.h"


UCLASS(BlueprintType)
class GAS_API UDropItemData : public UDataAsset
{
	GENERATED_BODY()
    
public: // 改成public，或者给每个变量加BlueprintReadOnly
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置")
	int32 ItemID = 0;

	// 掉落概率 0.0 ~ 1.0（1.0 = 必掉）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DropChance = 0.5f;

	// 掉落数量范围
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置", meta = (ClampMin = 1))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "掉落配置", meta = (ClampMin = 1))
	int32 MaxCount = 1;
};
