// Copyright TyniBoat 2025, All Rights reserved


#include "Actors/BaseGasCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CharacterAnchor.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include <GameDataTypes/EnemyData.h>
#include "FCTweenInstanceVector.h"


// Sets default values
ABaseGasCharacter::ABaseGasCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CommonAttributeSet = CreateDefaultSubobject<UCommonCharacterAttributeSet>(TEXT("CommonAttributeSet"));
	CommonAttributeSet->OnHealthChanged.AddDynamic(this, &ABaseGasCharacter::OnHealthChanged_Internal);
	SubMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SubMesh");
	SubMesh->SetupAttachment(GetMesh());
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	bUseControllerRotationYaw = false;
	GetCapsuleComponent()->SetCapsuleHalfHeight(96);
	GetCapsuleComponent()->SetCapsuleRadius(42);
	GetMesh()->SetRelativeTransform(FTransform({ 0,-90,0 }, { 0,0,-96 }, FVector::OneVector));
	GetMesh()->SetHiddenInGame(true);
}

void ABaseGasCharacter::OnHealthChanged_Internal(float ChangeDelta, float NewValue, bool HitLimit)
{
	if (NewValue <= 0 && (NewValue + ChangeDelta) > 0)
		OnCharacterDied.Broadcast(this);
}

// Called when the game starts or when spawned
void ABaseGasCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (Anchor)
		Anchor->SetNewOwner(this);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		//Initialize Attributes
		AbilitySystemComponent->SetNumericAttributeBase(UCommonCharacterAttributeSet::GetMaxHealthAttribute(), CharacterData ? CharacterData->Stats.MaxHealth : InitialMaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UCommonCharacterAttributeSet::GetHealthAttribute(), CharacterData ? CharacterData->Stats.MaxHealth : InitialMaxHealth);
	}
	GetCharacterMovement()->MaxWalkSpeed = CharacterData ? CharacterData->Stats.OnGroundSpeeds.X : 250;
}

bool ABaseGasCharacter::MoveToAnchor_Implementation(EMoveToAnchorType movementType, UCharacterAnchor* targetAnchor, EMoveToAnchorType& movingToAnchorType)
{
	if (!targetAnchor)
		return false;
	switch (movementType)
	{
		case EMoveToAnchorType::None:
			break;
		case EMoveToAnchorType::Walk:
			{
				float walkSpeed = GetCharacterMovement()->MaxWalkSpeed;
				GetCharacterMovement()->MaxWalkSpeed = CharacterData ? CharacterData->Stats.OnGroundSpeeds.X : 250;
				if (targetAnchor == Anchor)
					movingToAnchorType = EMoveToAnchorType::Walk;
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), targetAnchor->GetComponentLocation());
			}
			break;
		case EMoveToAnchorType::Run:
			{
				float runSpeed = GetCharacterMovement()->MaxWalkSpeed;
				GetCharacterMovement()->MaxWalkSpeed = CharacterData ? CharacterData->Stats.OnGroundSpeeds.Y : 450;
				if (targetAnchor == Anchor)
					movingToAnchorType = EMoveToAnchorType::Run;
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), targetAnchor->GetComponentLocation());
			}
			break;
		case EMoveToAnchorType::Dash:
			{
				if (_dashTween != nullptr)
				{
					if (_dashTweenVector && _dashTweenVector->bIsActive)
					{
						_dashTweenVector->Destroy();
						_dashTweenVector = nullptr;
					}
				}
				if (targetAnchor == Anchor)
					movingToAnchorType = EMoveToAnchorType::Dash;
				GetCharacterMovement()->MaxWalkSpeed = 1;
				_dashTweenVector = FCTween::Play(
					GetActorLocation(), targetAnchor->GetComponentLocation(), [&](FVector t)
					{
						if (auto movement = GetCharacterMovement())
						{
							movement->AddInputVector(_dashTweenVector->EndValue - t, false);
							movement->Velocity = (t - GetActorLocation()) / GetWorld()->DeltaTimeSeconds;
						}
					},
					DashTime,
					DashEasing);
				_dashTweenVector->SetDelay(DashDelay);
				_dashTweenVector->SetOnComplete([&]() { OnAnchorReached(); });
				_dashTweenVector->SetUseGlobalTimeDilation(true);
				_dashTween = _dashTweenVector->CreateUObject(this);
			}
			break;
		case EMoveToAnchorType::Teleport:
			{
				if (targetAnchor == Anchor)
					movingToAnchorType = EMoveToAnchorType::Teleport;
				SetActorLocation(targetAnchor->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);
				OnAnchorReached();
			}
			break;
		default:
			break;
	}
	return true;
}

// Called every frame
void ABaseGasCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABaseGasCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABaseGasCharacter::SetNewAnchor(UCharacterAnchor* newAnchor)
{
	auto oldAnchor = Anchor;
	Anchor = newAnchor;
	OnAnchorChanged.Broadcast(oldAnchor, Anchor);
}

bool ABaseGasCharacter::TryMoveToAnchor(EMoveToAnchorType movementType)
{
	MoveToAnchor(movementType, Anchor, _movingToAnchorType);
	return true;
}

void ABaseGasCharacter::OnAnchorReached()
{
	if (_movingToAnchorType == EMoveToAnchorType::None)
		return;
	GetCharacterMovement()->MaxWalkSpeed = CharacterData ? CharacterData->Stats.OnGroundSpeeds.X : 250;
	_movingToAnchorType = EMoveToAnchorType::None;
}

void ABaseGasCharacter::OnAnchorLeft()
{
}

