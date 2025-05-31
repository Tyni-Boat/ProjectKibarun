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
	UpdateAnchor();
}

void UCharacterAnchor::OnLinkedAnchorStateChanged_internal(UCharacterAnchor* Anchor, EAnchorState lastState, EAnchorState newState)
{
	if (newState == EAnchorState::Free)
		return;
	if (GetAnchorState() != EAnchorState::Free)
		return;
	if (_linkedAnchor.IsValid() && _linkedAnchor->_linkedCharacter.IsValid())
	{
		auto character = _linkedAnchor->_linkedCharacter.Get();
		_linkedAnchor->SetNewOwner(nullptr);
		SetNewOwner(character);
	}
}

void UCharacterAnchor::OnBeginOverlap_internal(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
											   const FHitResult& SweepResult)
{
	if (OtherActor != _linkedCharacter)
		return;
	if (UpdateAnchor() && GetAnchorState() == EAnchorState::Occupied)
	{
		if (_linkedCharacter.IsValid())
			_linkedCharacter->OnAnchorReached();
	}
}

void UCharacterAnchor::OnEndOverlap_internal(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != _linkedCharacter)
		return;
	if (UpdateAnchor() && GetAnchorState() == EAnchorState::Reserved)
	{
		if (_linkedCharacter.IsValid())
			_linkedCharacter->OnAnchorLeft();
	}
}

UCharacterAnchor::UCharacterAnchor()
{
	OnComponentBeginOverlap.AddDynamic(this, &UCharacterAnchor::OnBeginOverlap_internal);
	OnComponentEndOverlap.AddDynamic(this, &UCharacterAnchor::OnEndOverlap_internal);
	BoxExtent = { 32, 32, 100 };
}

bool UCharacterAnchor::UpdateAnchor(bool silent)
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
		if (!silent)
		{
			OnAnchorStateChanged.Broadcast(this, lastState, _state);
			//if (auto world = GetWorld())
			//	world->GetTimerManager().SetTimerForNextTick([this, lastState]()->void { OnAnchorStateChanged.Broadcast(this, lastState, _state); });
		}
	}
	if (_state == EAnchorState::Free)
	{
		if (_linkedAnchor.IsValid())
		{
			if (_linkedAnchor->_linkedCharacter.IsValid())
			{
				auto character = _linkedAnchor->_linkedCharacter.Get();
				_linkedAnchor->SetNewOwner(nullptr);
				SetNewOwner(character);
			}
		}
		else if (!silent)
		{
			if (auto world = GetWorld())
			{
				world->GetTimerManager().SetTimerForNextTick([this]()->void { OnAnchorRequestNewOwner.Broadcast(this); });
			}
		}
	}
	return response;
}

void UCharacterAnchor::SetNewOwner(ABaseGasCharacter* NewOwner, bool silent)
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
	UpdateAnchor(silent);
	if (_linkedCharacter.IsValid())
		_linkedCharacter->SetNewAnchor(this, silent);
}

void UCharacterAnchor::LinkAnchor(UCharacterAnchor* NewLink)
{
	if (_linkedAnchor.IsValid())
		_linkedAnchor->OnAnchorStateChanged.RemoveDynamic(this, &UCharacterAnchor::OnLinkedAnchorStateChanged_internal);
	_linkedAnchor = NewLink;
	if (_linkedAnchor.IsValid())
		_linkedAnchor->OnAnchorStateChanged.AddDynamic(this, &UCharacterAnchor::OnLinkedAnchorStateChanged_internal);
}
