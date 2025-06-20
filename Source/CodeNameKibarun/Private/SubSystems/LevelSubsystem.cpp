#include "SubSystems/LevelSubsystem.h"
#include <Engine/AssetManager.h>
#include <Kismet/GameplayStatics.h>

bool ULevelSubsystem::SetLevel(ULevelData* levelDatas)
{
	if (!levelDatas)
		return false;
	_stageCollection = levelDatas->Stages;
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Set level: %s"), *levelDatas->GetName()), true, false, FLinearColor::Green, 5.f);
	return true;
}

bool ULevelSubsystem::JumpToStage(int32 index)
{
	if (!_stageCollection.Combats.IsValidIndex(index))
		return false;
	_stageCollection.StageIndex = index;
	return true;
}

bool ULevelSubsystem::MoveToStage(int32 increment)
{
	int32 newIndex = _stageCollection.StageIndex + increment;
	return JumpToStage(newIndex);
}


//------------------------------------------------------------------------------------------------------------------------------------------


UAsyncLoadLevel* UAsyncLoadLevel::LoadLevel(UObject* WorldContextObject, FPrimaryAssetId levelID, bool setAsCurrentLevel)
{
	UAsyncLoadLevel* Node = NewObject<UAsyncLoadLevel>();
	Node->WorldContextObject = WorldContextObject;
	Node->LevelID = levelID;
	Node->bSetAsCurrentLevel = setAsCurrentLevel;
	// Register with the game instance to avoid being garbage collected
	Node->RegisterWithGameInstance(WorldContextObject);
	return Node;
}

void UAsyncLoadLevel::Activate()
{
	auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
	{
		OnLoadCompleted.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}
	mgr->LoadPrimaryAsset(LevelID, { BUNDLE_INFOS }, FStreamableDelegate::CreateUObject(this, &UAsyncLoadLevel::ExecuteOnLoadLevelLoaded, LevelID, bSetAsCurrentLevel));
}

void UAsyncLoadLevel::ExecuteOnLoadLevelLoaded(FPrimaryAssetId levelID, bool setAsCurrentLevel)
{
	const auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
	{
		OnLoadCompleted.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}
	const auto levelData = mgr->GetPrimaryAssetObject<ULevelData>(levelID);
	if (setAsCurrentLevel)
	{
		if (auto gameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
			if (auto levelSub = gameInstance->GetSubsystem<ULevelSubsystem>())
				levelSub->SetLevel(levelData);
	}
	if (!levelData)
	{
		OnLoadCompleted.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}
	OnLoadCompleted.Broadcast(levelData);
	SetReadyToDestroy();
}


//------------------------------------------------------------------------------------------------------------------------------------------


UAsyncLoadStage* UAsyncLoadStage::LoadStage(UObject* WorldContextObject, FPrimaryAssetId stageID)
{
	UAsyncLoadStage* Node = NewObject<UAsyncLoadStage>();
	Node->WorldContextObject = WorldContextObject;
	Node->StageID = stageID;
	// Register with the game instance to avoid being garbage collected
	Node->RegisterWithGameInstance(WorldContextObject);
	return Node;
}

void UAsyncLoadStage::Activate()
{
	auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
	{
		OnLoadCompleted.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}
	mgr->LoadPrimaryAsset(StageID, { BUNDLE_INFOS, BUNDLE_SPAWN }, FStreamableDelegate::CreateUObject(this, &UAsyncLoadStage::ExecuteOnLoadStageLoaded, StageID));
}

void UAsyncLoadStage::ExecuteOnLoadStageLoaded(FPrimaryAssetId stageID)
{
	const auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
	{
		OnLoadCompleted.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}
	const auto levelData = mgr->GetPrimaryAssetObject<UCombatData>(stageID);
	if (!levelData)
	{
		OnLoadCompleted.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}
	OnLoadCompleted.Broadcast(levelData);
	SetReadyToDestroy();
}
