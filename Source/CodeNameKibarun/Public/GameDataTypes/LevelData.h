#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameDataTypes/BaseGameDataType.h"
#include "LevelData.generated.h"





USTRUCT(BlueprintType)
struct FLevelStageCollection
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level", meta=(AllowedTypes = "CombatData"))
	TArray<FPrimaryAssetId> Combats;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Level", meta=(AllowedTypes = "EnemyData"))
	int32 StageIndex = 0;
	
};


/**
 * Data of a combat
 */
UCLASS(BlueprintType, Blueprintable)
class CODENAMEKIBARUN_API ULevelData : public UBaseGameDataType
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BUNDLE_INFOS, meta = (AssetBundles = BUNDLE_INFOS))
	FLevelStageCollection Stages;

};
