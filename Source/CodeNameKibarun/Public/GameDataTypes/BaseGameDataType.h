// Copyright TyniBoat 2025, All Rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BaseGameDataType.generated.h"


UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EDataBundleType : uint8
{
	None = 0 UMETA(Hidden),
	Infos = 1 << 0 UMETA(ToolTip = "For things like names, stats and all simple types"),
	UI = 1 << 1 UMETA(ToolTip = "For managed types related to UI like icons, widgets"),
	Spawn = 1 << 2 UMETA(ToolTip = "For managed types related to placement in the world like meshes, materials or anim bp"),
	Accessories = 1 << 3 UMETA(ToolTip = "For other managed types"),
};


#define BUNDLE_INFOS "Infos"
#define BUNDLE_UI "UI"
#define BUNDLE_SPAWN "Spawn" 
#define BUNDLE_ACCESSORY "Accessory"


USTRUCT(BlueprintType)
struct FCharacterStats
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100;

	// Walk, run, sprint speeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FVector OnGroundSpeeds = { 250, 450, 600 };
};


/**
 * Serve as a base class for all game Data types
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class CODENAMEKIBARUN_API UBaseGameDataType : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Data Bundles", meta = (CompactNodeTitle = "BundleFlags"))
	static TArray<FName> GetDataBundleFromFlags(UPARAM(meta = (Bitmask, BitmaskEnum = EAnimDescriptorFlags)) int32 flag);
};
