// Copyright TyniBoat 2025, All Rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/CombatManager.h"
#include "CombatSubSystem.generated.h"


/**
 * CombatSubSystem is responsible for managing combat-related functionalities in the game.
 * It handles combat mechanics, player interactions, and combat state management.
 */
UCLASS()
class CODENAMEKIBARUN_API UCombatSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	
	UPROPERTY(VisibleDefaultsOnly, Category="Combat")
	TObjectPtr<ACombatManager> _combatManager;

public:

	UFUNCTION(BlueprintPure, Category = "Combat", meta = (CompactNodeTitle = "CombatManager"))
	FORCEINLINE ACombatManager* GetCombatManager() const { return _combatManager; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetCombatManager(ACombatManager* NewCombatManager);
};
