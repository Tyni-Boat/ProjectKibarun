// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterAnchor.h"


void UCharacterAnchor::OnOwnerDestroyed_internal(AActor* actor)
{
	if (_linkedCharacter.IsValid() && actor == _linkedCharacter)
	{
		_linkedCharacter->OnDestroyed.RemoveDynamic(this, &UCharacterAnchor::OnOwnerDestroyed_internal);
		_linkedCharacter->OnCharacterDied.RemoveDynamic(this, &UCharacterAnchor::OnOwnerDestroyed_internal);
	}
	_linkedCharacter = nullptr;
}

void UCharacterAnchor::OnBeginOverlap_internal(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor != _linkedCharacter)
		return;
}

void UCharacterAnchor::OnEndOverlap_internal(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != _linkedCharacter)
		return;
}

UCharacterAnchor::UCharacterAnchor()
{
	OnComponentBeginOverlap.AddDynamic(this, &UCharacterAnchor::OnBeginOverlap_internal);
	OnComponentEndOverlap.AddDynamic(this, &UCharacterAnchor::OnEndOverlap_internal);
	BoxExtent = { 32, 32, 100 };
}

bool UCharacterAnchor::UpdateAnchor()
{
	bool response = false;
	auto lastState = _state;
	if (const auto actor = _linkedCharacter.Get())
	{
		bool isOverlapping = OverlappingComponents.ContainsByPredicate([actor](const FOverlapInfo& overlap)-> bool { return overlap.OverlapInfo.GetActor() == actor; });
		_state = isOverlapping ? EAnchorState::Occupied : EAnchorState::Reserved;
	}
	else
	{
		_state = EAnchorState::Free;
	}
	if (lastState != _state)
	{
		response = true;
		OnAnchorStateChanged.Broadcast(this, lastState, _state);
		if (_linkedCharacter.IsValid())
		{
			if (lastState == EAnchorState::Occupied && _state == EAnchorState::Reserved)
			{
				// The owner just left
				_linkedCharacter->OnAnchorLeft();
			}
			else if (_state == EAnchorState::Occupied)
			{
				// The owner just reached
				_linkedCharacter->OnAnchorReached();
			}
		}
	}

	return response;
}

void UCharacterAnchor::SetNewOwner(ABaseGasCharacter* NewOwner)
{
	if (_linkedCharacter.IsValid())
	{
		_linkedCharacter->OnDestroyed.RemoveDynamic(this, &UCharacterAnchor::OnOwnerDestroyed_internal);
		_linkedCharacter->OnCharacterDied.RemoveDynamic(this, &UCharacterAnchor::OnOwnerDestroyed_internal);
	}
	_linkedCharacter = NewOwner;
	if (_linkedCharacter.IsValid())
	{
		_linkedCharacter->OnDestroyed.AddDynamic(this, &UCharacterAnchor::OnOwnerDestroyed_internal);
		_linkedCharacter->OnCharacterDied.AddDynamic(this, &UCharacterAnchor::OnOwnerDestroyed_internal);
	}
	if (_linkedCharacter.IsValid())
		_linkedCharacter->SetNewAnchor(this);
}
