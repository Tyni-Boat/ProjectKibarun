#pragma once

#include "CoreMinimal.h"
#include "GameDataTypes/LevelData.h"
#include "GameDataTypes/CombatData.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/GameInstance.h"
#include "LevelSubsystem.generated.h"




/**
 * LevelSubsystem handle level stages
 */
UCLASS()
class CODENAMEKIBARUN_API ULevelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleDefaultsOnly, Category = "Level")
	FLevelStageCollection _stageCollection;

public:

	UFUNCTION(BlueprintPure, Category = "Level", meta = (CompactNodeTitle = "Stages"))
	FORCEINLINE FLevelStageCollection GetStageCollection() const { return _stageCollection; }

	UFUNCTION(BlueprintPure, Category = "Level", meta = (CompactNodeTitle = "IsLastStage"))
	FORCEINLINE bool IsLastStage() const { return StageIndex() >= _stageCollection.Combats.Num() - 1; }

	UFUNCTION(BlueprintPure, Category = "Level", meta = (CompactNodeTitle = "IsFirstStage"))
	FORCEINLINE bool IsFirstStage() const { return StageIndex() == 0; }

	UFUNCTION(BlueprintPure, Category = "Level", meta = (CompactNodeTitle = "StageIndex"))
	FORCEINLINE int32 StageIndex() const { return _stageCollection.StageIndex; }

	UFUNCTION(BlueprintPure, Category = "Level")
	FORCEINLINE FPrimaryAssetId GetCurrentStage() const { return _stageCollection.Combats.IsValidIndex(_stageCollection.StageIndex) ? _stageCollection.Combats[_stageCollection.StageIndex] : FPrimaryAssetId(); }

	// Load a level
	bool SetLevel(ULevelData* levelDatas);

	// Absolute move to stage index
	UFUNCTION(BlueprintCallable, Category = "Level")
	bool JumpToStage(int32 index);

	// Relative move to stage index: CurrentStage + increment
	UFUNCTION(BlueprintCallable, Category = "Level")
	bool MoveToStage(int32 increment);

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelLoaded, ULevelData*, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageLoaded, UCombatData*, Result);


UCLASS()
class CODENAMEKIBARUN_API UAsyncLoadLevel : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Blueprint node exposed function
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Level")
	static UAsyncLoadLevel* LoadLevel(UObject* WorldContextObject, UPARAM(meta = (AllowedTypes = "LevelData")) FPrimaryAssetId levelID, bool setAsCurrentLevel = true);

	// Delegate to notify when the task is complete
	UPROPERTY(BlueprintAssignable)
	FOnLevelLoaded OnLoadCompleted;

protected:

	virtual void Activate() override;

	// Internal function to execute the async task
	UFUNCTION()
	void ExecuteOnLoadLevelLoaded(FPrimaryAssetId levelID, bool setAsCurrentLevel = true);

private:

	UObject* WorldContextObject;
	FPrimaryAssetId LevelID;
	bool bSetAsCurrentLevel = true;
};


UCLASS()
class CODENAMEKIBARUN_API UAsyncLoadStage : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Blueprint node exposed function
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Level")
	static UAsyncLoadStage* LoadStage(UObject* WorldContextObject, UPARAM(meta = (AllowedTypes = "CombatData")) FPrimaryAssetId stageID);

	// Delegate to notify when the task is complete
	UPROPERTY(BlueprintAssignable)
	FOnStageLoaded OnLoadCompleted;

protected:

	virtual void Activate() override;

	// Internal function to execute the async task
	UFUNCTION()
	void ExecuteOnLoadStageLoaded(FPrimaryAssetId stageID);

private:

	UObject* WorldContextObject;
	FPrimaryAssetId StageID;
};