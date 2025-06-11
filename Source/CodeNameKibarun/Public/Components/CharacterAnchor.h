// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/BaseGasCharacter.h"
#include "Components/BoxComponent.h"
#include "CharacterAnchor.generated.h"


UENUM(BlueprintType)
enum class EAnchorState : uint8
{
	Free UMETA(ToolTip = "The anchor has no owner at all"),
	Reserved UMETA(ToolTip = "The anchor has an owner but it's not present"),
	Occupied UMETA(ToolTip = "The anchor has an owner and it's present"),
};

class UCharacterAnchor;

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams(FOnAnchorStateChangedSignature, UCharacterAnchor, OnAnchorStateChanged, UCharacterAnchor*, Anchor, EAnchorState, LastState, EAnchorState, NewState);


/**
 * Define An Anchor on the ground where a character will return to
 */
UCLASS(ClassGroup = "Placement", HideCategories = (Object, LOD, Lighting, TextureStreaming),
	   EditInlineNew, Meta = (DisplayName = "Anchor", BlueprintSpawnableComponent))
	class CODENAMEKIBARUN_API UCharacterAnchor : public UBoxComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleInstanceOnly)
	TWeakObjectPtr<ABaseGasCharacter> _linkedCharacter;

	UPROPERTY()
	EAnchorState _state;

	UFUNCTION()
	void OnOwnerDestroyed_internal(AActor* actor);

	UFUNCTION()
	void OnBeginOverlap_internal(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap_internal(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:

	UCharacterAnchor();

	UPROPERTY(BlueprintAssignable)
	FOnAnchorStateChangedSignature OnAnchorStateChanged;

	UFUNCTION(BlueprintPure, Category = "Character Anchor", meta = (CompactNodeTitle = "State"))
	FORCEINLINE EAnchorState GetAnchorState() const { return _state; }

	UFUNCTION(BlueprintPure, Category = "Character Anchor", meta = (CompactNodeTitle = "Owner"))
	FORCEINLINE ABaseGasCharacter* GetBaseOwnwer() { return _linkedCharacter.IsValid()? _linkedCharacter.Get() : nullptr; }


	// Return true if the state changed. Silent avoid broadcasting the state change event.
	UFUNCTION(BlueprintCallable, Category = "Character Anchor")
	bool UpdateAnchor();

	UFUNCTION(BlueprintCallable, Category = "Character Anchor")
	void SetNewOwner(ABaseGasCharacter* NewOwner);
};
