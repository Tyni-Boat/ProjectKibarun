// Copyright TyniBoat 2025, All Rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CommonCharacterAttributeSet.h"
#include "GameDataTypes/BaseCharacterData.h"
#include "Camera/CameraComponent.h"
#include "FCTween.h"
#include "FCTweenInstance.h"
#include "FCTweenUObject.h"
#include "BaseGasCharacter.generated.h"



UENUM(BlueprintType)
enum class EMoveToAnchorType : uint8
{
	None,
	Walk,
	Run,
	Dash,
	Teleport
};


class UCharacterAnchor;
class ABaseGasCharacter;
class ACombatManager;
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FOnAnchorChangedSignature, ABaseGasCharacter, OnAnchorChanged, UCharacterAnchor*, OldAnchor, UCharacterAnchor*, NewAnchor);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FCharacterDiedSignature, ABaseGasCharacter, OnCharacterDied, AActor*, DiedCharacter);

/**
 * Base class for characters in the game
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class CODENAMEKIBARUN_API ABaseGasCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

private:
	
	/** The secondary skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SubMesh;
	
	/** The Camera of this character */
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CharacterCamera;

	// Active when the character is moving to an anchor
	EMoveToAnchorType _movingToAnchorType = EMoveToAnchorType::None;

	//UPROPERTY()
	UFCTweenUObject* _dashTween;

public:
	// Sets default values for this character's properties
	ABaseGasCharacter(); 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Datas", meta=(ExposeOnSpawn=true))
	UBaseCharacterData* CharacterData;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category="Ability System", meta=(CompactNodeTitle="Alive"))
	FORCEINLINE bool IsCharacterAlive() const { return CommonAttributeSet ? CommonAttributeSet->GetHealth() > 0 : false; }

	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle="CommonAttributes"))
	FORCEINLINE UCommonCharacterAttributeSet* GetCommonAttributeSet() const { return CommonAttributeSet ? CommonAttributeSet : nullptr; }

	UPROPERTY()
	TObjectPtr<UCommonCharacterAttributeSet> CommonAttributeSet;

	UPROPERTY(EditAnywhere, Category="Ability System|Attributes")
	float InitialMaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, Category="Ability System|Attributes")
	float DashTime = 1.35f;

	UPROPERTY(EditAnywhere, Category="Ability System|Attributes")
	EFCEase DashEasing = EFCEase::InOutExpo;

	UFUNCTION()
	void OnHealthChanged_Internal(float ChangeDelta, float NewValue, bool HitLimit);


	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat", meta=(ExposeOnSpawn=true))
	TObjectPtr<UCharacterAnchor> Anchor;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat", meta=(ExposeOnSpawn=true))
	TObjectPtr<ACombatManager> CombatMgr;
	
	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnAnchorChangedSignature OnAnchorChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Combat")
	FCharacterDiedSignature OnCharacterDied;

	
	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle="MovingToAnchor"))
	FORCEINLINE EMoveToAnchorType CurrentAnchorMovementType() const { return _movingToAnchorType; }
	
	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle="Submesh"))
	FORCEINLINE USkeletalMeshComponent* GetSubmesh() const { return SubMesh; }
	
	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle="Character Camera"))
	FORCEINLINE UCameraComponent* GetCamera() const { return CharacterCamera; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Move to Anchor
	UFUNCTION(BlueprintNativeEvent, Category = "Character|Anchor")
	bool MoveToAnchor(EMoveToAnchorType movementType, UCharacterAnchor* targetAnchor, EMoveToAnchorType& movingToAnchorType);
	

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Set a new anchor
	void SetNewAnchor(UCharacterAnchor* newAnchor);

	// Try Move to Anchor
	UFUNCTION(BlueprintCallable, Category = "Character|Anchor")
	bool TryMoveToAnchor(EMoveToAnchorType movementType);

	UFUNCTION()
	void OnAnchorReached();

	UFUNCTION()
	void OnAnchorLeft();
};
