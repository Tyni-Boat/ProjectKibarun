// Copyright TyniBoat 2025, All Rights reserved


#include "Actors/CombatManager.h"
#include "Engine/AssetManager.h"
#include "GameDataTypes/EnemyData.h"
#include "GameDataTypes/HeroData.h"
#include "Kismet/KismetSystemLibrary.h"
#include <SubSystems/CombatSubSystem.h>
#include <SubSystems/LevelSubsystem.h>
#include <Kismet/GameplayStatics.h>


// Sets default values
ACombatManager::ACombatManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
#if WITH_EDITORONLY_DATA
	Icon = CreateDefaultSubobject<UBillboardComponent>("Icon");
	SetRootComponent(Icon);
#endif
	LevelCameraArm = CreateDefaultSubobject<USpringArmComponent>("Camera Arm");
	LevelCameraArm->SetRelativeRotation({ -45, -90, 0 });
	LevelCameraArm->bDoCollisionTest = false;
	LevelCameraArm->TargetArmLength = 800;
	LevelCamera = CreateDefaultSubobject<UCameraComponent>("Level Camera");
	LevelCamera->SetupAttachment(LevelCameraArm);
}

// Called when the game starts or when spawned
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();
	if (auto world = GetWorld())
		if (auto subSys = world->GetSubsystem<UCombatSubSystem>())
			subSys->SetCombatManager(this);

	// Fade in the black screen
	FadeCameraCommand(0.f, 1.f, 0, FLinearColor::Black, [&](bool success) {});

	//Load Stage from level Subsystem
	auto mgr = UAssetManager::GetIfInitialized();
	if (mgr && AutoInitCombat)
	{
		if (auto gameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
		{
			if (auto levelSub = gameInstance->GetSubsystem<ULevelSubsystem>())
			{
				FPrimaryAssetId stageID = levelSub->GetCurrentStage();
				if (stageID.IsValid())
				{
					mgr->LoadPrimaryAsset(stageID, { BUNDLE_SPAWN }, FStreamableDelegate::CreateUObject(this, &ACombatManager::OnStageLoaded_Internal, stageID));
				}
			}
		}
	}
}

void ACombatManager::OnStageLoaded_Internal(FPrimaryAssetId stageID)
{
	const auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
		return;
	const auto stageData = mgr->GetPrimaryAssetObject<UCombatData>(stageID);
	InitCombat(stageData);
}

void ACombatManager::OnHeroPlayerLoaded_Internal(FPrimaryAssetId heroID, FTransform Spawn, UCharacterAnchor* charAnchor)
{
	const auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
		return;
	const auto heroData = mgr->GetPrimaryAssetObject<UHeroData>(heroID);
	if (!heroData)
		return;
	ABaseGasCharacter* heroActor = SpawnCharacter(heroData, Spawn, charAnchor);
	if (!heroActor)
		return;
	HeroesInScene.Add(heroActor);

	//Make them do the intro traversal
	bool madeTraversalIntro = false;
	if (CombatData)
	{
		auto traversal = CombatData->PlayerIntroTraversal.Get();
		//Load the traversal if not already loaded
		if (!traversal)
			traversal = CombatData->PlayerIntroTraversal.LoadSynchronous();
		//Set the traversal to the Hero
		if (traversal)
		{
			if (auto abilitySystem = heroActor->GetAbilitySystemComponent())
			{
				//Give the traversal ability to the Hero
				auto spec = FGameplayAbilitySpec(traversal, 1, -1, heroActor);
				auto handle = abilitySystem->GiveAbilityAndActivateOnce(spec);
				madeTraversalIntro = handle.IsValid();
			}
		}
	}

	//Snap to the Player's Camera
	if (auto plController = GetWorld()->GetFirstPlayerController())
	{
		plController->Possess(heroActor);
		if (!madeTraversalIntro)
			heroActor->TryMoveToAnchor(EMoveToAnchorType::Run);
	}

	// Fade in the black screen
	FadeCameraCommand(1.f, 0.f, CameraFadeTime, FLinearColor::Black, [&](bool success) {});

	OnPlayerSpawn.Broadcast(heroActor);
}

void ACombatManager::OnHeroLoaded_Internal(FPrimaryAssetId heroID, FTransform Spawn, UCharacterAnchor* charAnchor)
{
	const auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
		return;
	const auto heroData = mgr->GetPrimaryAssetObject<UHeroData>(heroID);
	if (!heroData)
		return;
	auto hero = SpawnCharacter(heroData, Spawn, charAnchor);
	if (!hero)
		return;
	HeroesInScene.Add(hero);
	hero->TryMoveToAnchor(EMoveToAnchorType::Teleport);
}

void ACombatManager::OnEnemyLoaded_Internal(FPrimaryAssetId enemyID, FTransform Spawn, UCharacterAnchor* charAnchor)
{
	bIsSpawning = false;
	const auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
		return;
	const auto enemyData = mgr->GetPrimaryAssetObject<UEnemyData>(enemyID);
	if (!enemyData)
		return;
	auto enemyActor = SpawnCharacter(enemyData, Spawn, charAnchor);
	if (enemyActor)
	{
		EnemiesInScene.Add(enemyActor);
		enemyActor->OnDestroyed.AddDynamic(this, &ACombatManager::OnEnemyDestroyed_Internal);
	}
}

void ACombatManager::OnEnemyDestroyed_Internal(AActor* actor)
{
	int index = EnemiesInScene.IndexOfByKey(actor);
	if (EnemiesInScene.IsValidIndex(index))
	{
		EnemiesInScene.RemoveAt(index);
	}
}

void ACombatManager::OnCameraMoveCommand_Internal()
{
	if (OnCameraMoveEnded)
	{
		OnCameraMoveEnded(true);
		OnCameraMoveEnded.Reset();
	}
}

void ACombatManager::OnCameraFadeCommand_Internal()
{
	if (OnCameraFadeEnded)
	{
		OnCameraFadeEnded(true);
		OnCameraFadeEnded.Reset();
	}
}

void ACombatManager::SetCombatPhase(ECombatPhase phase)
{
	const auto lastPhase = CurrentPhase;
	CurrentPhase = phase;
	if (lastPhase != phase)
		OnStateChanged.Broadcast(CurrentPhase);
}

// Called every frame
void ACombatManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAnchors(DeltaTime);

	for (auto anchor : HeroAnchors)
	{
		FLinearColor color = anchor->GetAnchorState() == EAnchorState::Free ? FLinearColor::Green : (anchor->GetAnchorState() == EAnchorState::Occupied ? FLinearColor::Red : FLinearColor::Blue);
		UKismetSystemLibrary::DrawDebugSphere(this, GetActorTransform().TransformPosition(anchor->GetRelativeLocation()), 100.f, 12, color, 0, 1.f);
	}

	for (auto anchor : EnemyAnchors)
	{
		FLinearColor color = anchor->GetAnchorState() == EAnchorState::Free ? FLinearColor::Green : (anchor->GetAnchorState() == EAnchorState::Occupied ? FLinearColor::Red : FLinearColor::Blue);
		UKismetSystemLibrary::DrawDebugSphere(this, GetActorTransform().TransformPosition(anchor->GetRelativeLocation()), 100.f, 12, color, 0, 1.f);
	}
}

void ACombatManager::UpdateAnchors(float DeltaTime)
{
	// Update Anchors
	for (int i = 0; i < HeroAnchors.Num(); i++)
	{
		bool changed = HeroAnchors[i]->UpdateAnchor();
		if (changed)
		{
			if (i == 0)
			{
				if (CurrentPhase == ECombatPhase::Opening && HeroAnchors[i]->GetAnchorState() == EAnchorState::Occupied)
				{
					SpawnAllies();
					CreateNextWave();
				}
			}
		}
	}

	for (int i = 0; i < EnemyAnchors.Num(); i++)
	{
		bool changed = EnemyAnchors[i]->UpdateAnchor();
		bool isLast = i >= EnemyAnchors.Num() - 1;
		switch (EnemyAnchors[i]->GetAnchorState())
		{
			case EAnchorState::Free:
				{
					if (isLast)
					{
						if (CurrentPhase == ECombatPhase::Active || CurrentPhase == ECombatPhase::Opening)
						{
							TrySpawnEnemy(DeltaTime);
						}
					}
					else
					{
						if (auto owner = EnemyAnchors[i + 1]->GetBaseOwnwer())
						{
							EnemyAnchors[i + 1]->SetNewOwner(nullptr);
							EnemyAnchors[i]->SetNewOwner(owner);
						}
					}
				}
				break;
			case EAnchorState::Occupied: {
					if (isLast && changed)
					{
						if (CurrentPhase == ECombatPhase::Opening)
						{
							OnPhaseEnded.Broadcast(ECombatPhase::Opening);
						}
					}
				}break;
		}
	}
}

void ACombatManager::MoveCameraCommand(AActor* target, const float blendTime, TFunction<void(bool)> onCameraMoveEnded)
{
	if (OnCameraMoveEnded)
	{
		OnCameraMoveEnded(false);
		OnCameraMoveEnded.Reset();
	}
	if (!target || !GetWorld() || !GetWorld()->GetFirstPlayerController())
		return;
	if (auto plController = GetWorld()->GetFirstPlayerController())
	{
		plController->SetViewTargetWithBlend(target, blendTime, VTBlend_Cubic);
		OnCameraMoveEnded = onCameraMoveEnded;
		FTimerHandle hdl;
		GetWorld()->GetTimerManager().SetTimer(hdl, [&]()->void {OnCameraMoveCommand_Internal(); }, blendTime, false);
	}
}

void ACombatManager::FadeCameraCommand(const float fromAlpha, const float toAlpha, const float fadeTime, FLinearColor color, TFunction<void(bool)> onCameraFadeEnded)
{
	if (OnCameraFadeEnded)
	{
		OnCameraFadeEnded(false);
		OnCameraFadeEnded.Reset();
	}
	if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
		return;
	if (auto plController = GetWorld()->GetFirstPlayerController())
	{
		if (auto camMgr = plController->PlayerCameraManager)
		{
			camMgr->StartCameraFade(fromAlpha, toAlpha, fadeTime, color, true, true);
			OnCameraFadeEnded = onCameraFadeEnded;
			FTimerHandle hdl;
			GetWorld()->GetTimerManager().SetTimer(hdl, [&]()->void {OnCameraFadeCommand_Internal(); }, fadeTime, false);
		}
	}
}

void ACombatManager::PlaceAnchors()
{
	if (HeroAnchorLocations.IsEmpty() || EnemyAnchorLocations.IsEmpty())
		return;
	// Spawn Heroes anchors
	const bool isHeroesSync = HeroAnchors.Num() == HeroAnchorLocations.Num();
	if (!isHeroesSync)
	{
		for (auto anchor : HeroAnchors)
			if (anchor) anchor->DestroyComponent();
		HeroAnchors.Empty();
	}
	for (int i = 0; i < HeroAnchorLocations.Num(); i++)
	{
		auto location = GetActorTransform().TransformPosition(HeroAnchorLocations[i]);
		FHitResult hit;
		if (GetWorld() && GetWorld()->LineTraceSingleByChannel(hit, location, location - GetActorUpVector() * AnchorSnapDistance, ECC_WorldStatic))
			location = hit.ImpactPoint + (hit.TraceStart - hit.TraceEnd).GetSafeNormal() * AnchorSnapElevation;
		if (isHeroesSync)
		{
			if (HeroAnchors.IsValidIndex(i) && HeroAnchors[i])
				HeroAnchors[i]->SetRelativeLocation(GetActorTransform().InverseTransformPosition(location));
			continue;
		}
		auto anchor = NewObject<UCharacterAnchor>(this, FName(FString::Printf(TEXT("HeroAnchor_%d"), i + 1)));
		anchor->SetupAttachment(RootComponent);
		anchor->RegisterComponent();
		anchor->SetRelativeLocation(GetActorTransform().InverseTransformPosition(location));
		HeroAnchors.Add(anchor);
	}

	// Spawn Enemies anchors
	const bool isEnemiesSync = EnemyAnchors.Num() == EnemyAnchorLocations.Num();
	if (!isEnemiesSync)
	{
		for (auto anchor : EnemyAnchors)
			if (anchor) anchor->DestroyComponent();
		EnemyAnchors.Empty();
	}
	for (int i = EnemyAnchorLocations.Num() - 1; i >= 0; i--)
	{
		auto location = GetActorTransform().TransformPosition(EnemyAnchorLocations[i]);
		FHitResult hit;
		if (GetWorld() && GetWorld()->LineTraceSingleByChannel(hit, location, location - GetActorUpVector() * AnchorSnapDistance, ECC_WorldStatic))
			location = hit.ImpactPoint + (hit.TraceStart - hit.TraceEnd).GetSafeNormal() * AnchorSnapElevation;
		if (isEnemiesSync)
		{
			if (EnemyAnchors.IsValidIndex(i) && EnemyAnchors[i])
				EnemyAnchors[i]->SetRelativeLocation(GetActorTransform().InverseTransformPosition(location));
			continue;
		}
		auto anchor = NewObject<UCharacterAnchor>(this, FName(FString::Printf(TEXT("EnemyAnchor_%d"), i + 1)));
		anchor->SetupAttachment(RootComponent);
		anchor->RegisterComponent();
		anchor->SetRelativeLocation(GetActorTransform().InverseTransformPosition(location));
		EnemyAnchors.Insert(anchor, 0);
	}
}

void ACombatManager::InitCombat(UCombatData* data)
{
	if (!data)
		return;
	if (HeroSpawns.IsEmpty())
		return;
	if (EnemySpawns.IsEmpty())
		return;
	auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
		return;
	auto combatId = data->GetPrimaryAssetId();
	if (!combatId.IsValid())
		return;
	if (auto gameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (auto levelSub = gameInstance->GetSubsystem<ULevelSubsystem>())
		{
			_wasLastStage = levelSub->IsLastStage();
		}
	}
	CombatDataID = combatId;
	CombatData = data;

	// Place anchors on the ground
	PlaceAnchors();

	// Clear previous heroes and enemies
	for (auto hero : HeroesInScene)
		if (hero) hero->Destroy();
	HeroesInScene.Empty();
	for (auto enemy : EnemiesInScene)
		if (enemy) enemy->Destroy();
	EnemiesInScene.Empty();

	//Set state to Opening
	SetCombatPhase(ECombatPhase::Opening);

	//Enqueue Enemy Waves from Combat data
	WaveQueue.Empty();
	for (auto wave : CombatData->EnemyWaves)
		WaveQueue.Enqueue(wave);

	//Spawn Player hero
	FTransform heroSpawn = FTransform(GetActorTransform().TransformRotation(HeroSpawns[0].GetRotation()), GetActorTransform().TransformPosition(HeroSpawns[0].GetLocation()), FVector::OneVector);
	mgr->LoadPrimaryAsset(TestPlayerData, { BUNDLE_SPAWN }, FStreamableDelegate::CreateUObject(this, &ACombatManager::OnHeroPlayerLoaded_Internal, TestPlayerData, heroSpawn, HeroAnchors.IsValidIndex(0) ? HeroAnchors[0].Get() : nullptr));
}

void ACombatManager::SpawnAllies()
{
	auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
		return;
	if (HeroSpawns.IsValidIndex(1) && TestPlayerCombiData.IsValid())
	{
		FTransform heroCombiSpawn = FTransform(GetActorTransform().TransformRotation(HeroSpawns[1].GetRotation()), GetActorTransform().TransformPosition(HeroSpawns[1].GetLocation()), FVector::OneVector);
		mgr->LoadPrimaryAsset(TestPlayerCombiData, { BUNDLE_SPAWN }, FStreamableDelegate::CreateUObject(this, &ACombatManager::OnHeroLoaded_Internal, TestPlayerCombiData, heroCombiSpawn, HeroAnchors.IsValidIndex(1) ? HeroAnchors[1].Get() : nullptr));
	}
}

void ACombatManager::BeginCombat()
{
	// Pan the camera to the combat area
	SetCombatPhase(ECombatPhase::Begining);
	MoveCameraCommand(this, CameraBlendTime, [&](bool success)
					  {
						  if (success)
						  {
							  if (CurrentPhase == ECombatPhase::Begining)
							  {
								  OnPhaseEnded.Broadcast(ECombatPhase::Begining);
							  }
							  SetCombatPhase(ECombatPhase::Active);
						  }
					  });

	// Wait for the camera to be ready
	// Set the combat phase to Active
}

void ACombatManager::EndCombat()
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Combat Ended")));
	SetCombatPhase(ECombatPhase::Ended);
	// Fade in the black screen
	FadeCameraCommand(0.f, 1.f, CameraFadeTime, FLinearColor::Black, [&](bool success) { OnPhaseEnded.Broadcast(ECombatPhase::Ended); });
}

void ACombatManager::CreateNextWave()
{
	// Set the current wave
	if (WaveQueue.Dequeue(CurrentWave))
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Switch to new Wave with %d enemies. Is last wave? %d"), CurrentWave.WaveEnemies.Num(), WaveQueue.IsEmpty()));

		//Set the current wave to the first enemy in the queue
		if (CurrentWave.WaveEnemies.IsEmpty())
		{
			CreateNextWave(); //No enemies in this wave, try next one
			return;
		}

		//Set the spawn queue to the current wave enemies
		for (const auto& enemy : CurrentWave.WaveEnemies)
		{
			if (enemy.IsValid())
				SpawnQueue.Enqueue(enemy);
		}

		if (SpawnQueue.IsEmpty())
		{
			//No enemies to spawn, try next wave
			CreateNextWave();
			return;
		}

		//Start the spawn timer
		if (CurrentWave.SpawnDelay > 0.f)
			SpawnTimer = CurrentWave.SpawnDelay;
	}
	else if (CurrentPhase == ECombatPhase::Active)
	{
		int aliveCount = 0;
		for (auto enemy : EnemiesInScene)
		{
			if (enemy && enemy->IsCharacterAlive())
				aliveCount++;
		}
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("No More Waves: %d still alives"), aliveCount));
		if (aliveCount > 0)
			return; //No more waves to create, but enemies are still alive

		// No more waves to create, end the combat
		SetCombatPhase(ECombatPhase::Ending);

		if (HeroesInScene.IsValidIndex(0))
		{
			if (auto heroActor = HeroesInScene[0])
			{
				if (auto gameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
				{
					if (auto levelSub = gameInstance->GetSubsystem<ULevelSubsystem>())
					{
						if (!_wasLastStage)
							levelSub->MoveToStage(1);
					}
				}

				//Make the player do the outro traversal
				bool madeTraversalOutro = false;
				if (CombatData && !_wasLastStage)
				{
					auto traversal = CombatData->PlayerOutroTraversal.Get();
					//Load the traversal if not already loaded
					if (!traversal)
						traversal = CombatData->PlayerOutroTraversal.LoadSynchronous();
					//Set the traversal to the Hero
					if (traversal)
					{
						if (auto abilitySystem = heroActor->GetAbilitySystemComponent())
						{
							//Give the traversal ability to the Hero
							auto spec = FGameplayAbilitySpec(traversal, 1, -1, heroActor);
							auto handle = abilitySystem->GiveAbilityAndActivateOnce(spec);
							madeTraversalOutro = handle.IsValid();
						}
					}
				}

				//Snap to the Player's Camera
				MoveCameraCommand(heroActor, CameraBlendTime, [&, madeTraversalOutro](bool success)
								  {
									  OnPhaseEnded.Broadcast(ECombatPhase::Ending);
									  if (!madeTraversalOutro)
									  {
										  //If the outro traversal is not set, just execute end combat
										  EndCombat();
									  }
								  });

			}
		}
	}
}

bool ACombatManager::TrySpawnEnemy(float DeltaTime)
{
	if (CurrentWave.SpawnMode == EWaveSpawnMode::None || bIsSpawning)
		return false;
	if (SpawnTimer > 0)
	{
		SpawnTimer -= DeltaTime;
		if (SpawnTimer > 0)
			return false; //Cannot spawn if a timer is active
		else
		{
			//if (CurrentWave.SpawnDelay > 0)
			//	SpawnTimer = CurrentWave.SpawnDelay;
		}
	}
	int existingEnemies = EnemiesInScene.Num();
	int aliveCount = 0;
	for (auto enemy : EnemiesInScene)
	{
		if (enemy && enemy->IsCharacterAlive())
			aliveCount++;
	}
	int freeAnchorCount = 0;
	for (auto anchor : EnemyAnchors)
	{
		if (anchor && anchor->GetAnchorState() == EAnchorState::Free)
			freeAnchorCount++;
	}
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Try to Spwn Enemy -> in scene: %d. Alives: %d. Free Anchors: %d"), existingEnemies, aliveCount, freeAnchorCount));
	switch (CurrentWave.SpawnMode)
	{
		case EWaveSpawnMode::OnAllAnchorsFree:
			if (freeAnchorCount < 3 && !CurrentWave.HasBeganSpawn)
				return false; //Cannot spawn if there are no free anchors
			break;
		case EWaveSpawnMode::OnTwoAnchorsFree:
			if (freeAnchorCount < 2 && !CurrentWave.HasBeganSpawn)
				return false; //Cannot spawn if there are not enough anchors are free
			break;
		case EWaveSpawnMode::OnOneAnchorsFree:
			if (freeAnchorCount < 1)
				return false; //Cannot spawn if there are not enough anchors are free
			break;
	}

	// Get the next enemy to spawn
	FPrimaryAssetId enemyID;
	if (!SpawnQueue.Dequeue(enemyID))
	{
		CreateNextWave(); //No enemies to spawn, try next wave
		return false;
	}

	// Check if the enemy ID is valid
	if (!enemyID.IsValid())
	{
		//Invalid enemy ID, try next enemy
		return false;
	}

	// Spawn the enemy
	CurrentWave.HasBeganSpawn = true;
	SpawnEnemy(enemyID);
	return true;
}

void ACombatManager::SpawnEnemy(FPrimaryAssetId enemyID)
{
	if (EnemySpawns.IsEmpty())
		return;
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Spawning Enemy -> ID: %s"), *enemyID.PrimaryAssetName.ToString()));
	auto spawnLocation = GetActorTransform().TransformPosition(EnemySpawns[(int)(FMath::RandRange(0, EnemySpawns.Num() - 1))].GetLocation());
	auto spawnRotation = GetActorTransform().TransformRotation(EnemySpawns[(int)(FMath::RandRange(0, EnemySpawns.Num() - 1))].GetRotation());
	auto mgr = UAssetManager::GetIfInitialized();
	if (!mgr)
		return;
	if (enemyID.IsValid())
	{
		bIsSpawning = true;
		FTransform heroCombiSpawn = FTransform(spawnRotation, spawnLocation, FVector::OneVector);
		mgr->LoadPrimaryAsset(enemyID, { BUNDLE_SPAWN }, FStreamableDelegate::CreateUObject(this, &ACombatManager::OnEnemyLoaded_Internal, enemyID, heroCombiSpawn, EnemyAnchors.Last().Get()));
	}
}
