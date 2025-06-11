// Copyright TyniBoat 2025, All Rights reserved

#pragma once

#include "CoreMinimal.h"
#include "BaseGasCharacter.h"
#include "Components/CharacterAnchor.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BillboardComponent.h"
#include "GameDataTypes/CombatData.h"
#include "CombatManager.generated.h"


UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
	Opening UMETA(ToolTip = "The players are spawned, doing intro traversal and moving to theirs anchors"),
	Begining UMETA(ToolTip = "The Enemy first wave is spawned and moving to theirs anchors"),
	Passive UMETA(ToolTip = "Disable AIs. Usefull for inGame cinematics or mini games"),
	Active UMETA(ToolTip = "The Combat is OnGoing"),
	Ending UMETA(ToolTip = "All enemies are died and the players are waiting to do the outro traversal"),
	Ended UMETA(ToolTip = "During the outro traversal, before the map is unloaded"),
};

class ACombatManager;
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnPhaseChangedSignature, ACombatManager, OnStateChanged, ECombatPhase, NewCombatPhase);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnPhaseEndedSignature, ACombatManager, OnPhaseEnded, ECombatPhase, EndedCombatPhase);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnPlayerSpawnSignature, ACombatManager, OnPlayerSpawn, ABaseGasCharacter*, PlayerActor);


/**
 * Manage the combat actors and state
 */
UCLASS(BlueprintType, Blueprintable)
class CODENAMEKIBARUN_API ACombatManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACombatManager();

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UBillboardComponent> Icon;
#endif

protected:

	// Anchors ____________________________________________________________________________________________________________________________________

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anchors", meta = (MakeEditWidget = true))
	TArray<FVector> HeroAnchorLocations = { {-150, 0, 0}, {-250, -125, 0} };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anchors", meta = (MakeEditWidget = true))
	TArray<FVector> EnemyAnchorLocations = { {150, 0, 0}, {320, -175, 0}, {420, 125, 0} };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchors")
	TArray<TObjectPtr<UCharacterAnchor>>  HeroAnchors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchors")
	TArray<TObjectPtr<UCharacterAnchor>>  EnemyAnchors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchors")
	float AnchorSnapDistance = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchors")
	float AnchorSnapElevation = 99;


	// Level ____________________________________________________________________________________________________________________________________

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Level")
	TObjectPtr<USpringArmComponent> LevelCameraArm;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Level")
	TObjectPtr<UCameraComponent> LevelCamera;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Level", meta = (MakeEditWidget = true))
	TArray<FTransform> HeroSpawns;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Level", meta = (MakeEditWidget = true))
	TArray<FTransform> EnemySpawns;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	ECombatPhase CurrentPhase = ECombatPhase::Opening;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	TObjectPtr<UCombatData> CombatData;


	FEnemyWave CurrentWave;
	TQueue<FEnemyWave> WaveQueue;
	TQueue<FPrimaryAssetId> SpawnQueue;
	float SpawnTimer;
	bool bIsSpawning = false;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TEST", meta = (AllowedTypes = "HeroData"))
	FPrimaryAssetId TestPlayerData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TEST", meta = (AllowedTypes = "HeroData"))
	FPrimaryAssetId TestPlayerCombiData;


	// Actor References ____________________________________________________________________________________________________________________________________

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actors References")
	TSubclassOf<ABaseGasCharacter> CharacterClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actors References")
	TArray<TObjectPtr<ABaseGasCharacter>> HeroesInScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actors References")
	TArray<TObjectPtr<ABaseGasCharacter>> EnemiesInScene;



	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHeroPlayerLoaded_Internal(FPrimaryAssetId heroID, FTransform Spawn, UCharacterAnchor* charAnchor = nullptr);

	UFUNCTION()
	void OnHeroLoaded_Internal(FPrimaryAssetId heroID, FTransform Spawn, UCharacterAnchor* charAnchor = nullptr);

	UFUNCTION()
	void OnEnemyLoaded_Internal(FPrimaryAssetId enemyID, FTransform Spawn, UCharacterAnchor* charAnchor = nullptr);

	UFUNCTION()
	void OnEnemyDestroyed_Internal(AActor* actor);

	void SetCombatPhase(ECombatPhase phase);

public:

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhaseChangedSignature OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhaseEndedSignature OnPhaseEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerSpawnSignature OnPlayerSpawn;




	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Place anchors on the ground
	UFUNCTION(BlueprintCallable, Category = "Anchors")
	void PlaceAnchors();

	// Initialize the combat
	UFUNCTION(BlueprintCallable, Category = "Level")
	void InitCombat(UCombatData* data);

	// Spawn Allies
	UFUNCTION(BlueprintCallable, Category = "Level")
	void SpawnAllies();

	// Spawn a Character
	UFUNCTION(BlueprintImplementableEvent, Category = "Level")
	ABaseGasCharacter* SpawnCharacter(UBaseCharacterData* data, const FTransform spawnTr, UCharacterAnchor* charAnchor = nullptr);

	// Select Next Wave and try to spwn it; or end the combat if no more waves are available
	UFUNCTION(BlueprintCallable, Category = "Level")
	void CreateNextWave();

	// Try Spawn Enemy from the current wave
	UFUNCTION(BlueprintCallable, Category = "Level")
	bool TrySpawnEnemy(float DeltaTime);

	// Spawn Enemy
	UFUNCTION(BlueprintCallable, Category = "Level")
	void SpawnEnemy(UPARAM(meta = (AllowedTypes = "EnemyData")) FPrimaryAssetId enemyID);
};
