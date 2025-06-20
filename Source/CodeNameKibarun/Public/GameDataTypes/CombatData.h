// Copyright TyniBoat 2025, All Rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameDataTypes/BaseGameDataType.h"
#include "CombatData.generated.h"


UENUM(BlueprintType)
enum class EWaveSpawnMode: uint8
{
	None UMETA(Hidden),
	OnAllAnchorsFree UMETA(ToolTip="Spawn Wave when all the anchors are free"),
	OnTwoAnchorsFree UMETA(ToolTip="Spawn Wave when 2 anchors are free"),
	OnOneAnchorsFree UMETA(ToolTip="Spawn Wave when 1 anchors is free"),
};



USTRUCT(BlueprintType)
struct FEnemyWave
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wave")
	EWaveSpawnMode SpawnMode = EWaveSpawnMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wave")
	float SpawnDelay = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wave", meta=(AllowedTypes = "EnemyData"))
	TArray<FPrimaryAssetId> WaveEnemies;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Wave")
	bool HasBeganSpawn = false;
	
};



/**
 * Data of a combat
 */
UCLASS(BlueprintType, Blueprintable)
class CODENAMEKIBARUN_API UCombatData : public UBaseGameDataType
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BUNDLE_INFOS, meta = (AssetBundles = BUNDLE_INFOS))
	TArray<FEnemyWave> EnemyWaves;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BUNDLE_SPAWN, meta = (AssetBundles = BUNDLE_SPAWN))
	TSoftClassPtr<class UGameplayAbility> PlayerIntroTraversal;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BUNDLE_SPAWN, meta = (AssetBundles = BUNDLE_SPAWN))
	TSoftClassPtr<class UGameplayAbility> PlayerOutroTraversal;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BUNDLE_SPAWN, meta = (AssetBundles = BUNDLE_SPAWN))
	TSoftObjectPtr<UWorld> CombatMapArea;
};
