// Copyright TyniBoat 2025, All Rights reserved

#pragma once

#include "CoreMinimal.h"
#include "BaseGameDataType.h"
#include "BaseTools/BaseBrushTool.h"
#include "UObject/Object.h"
#include "BaseCharacterData.generated.h"

/**
 * Base Data for all characters
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class CODENAMEKIBARUN_API UBaseCharacterData : public UBaseGameDataType
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BUNDLE_INFOS, meta = (AssetBundles = BUNDLE_INFOS))
	FText Name = FText();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BUNDLE_INFOS, meta = (AssetBundles = BUNDLE_INFOS))
	FCharacterStats Stats;
};
