// Copyright TyniBoat 2025, All Rights reserved


#include "GameDataTypes/BaseGameDataType.h"

TArray<FName> UBaseGameDataType::GetDataBundleFromFlags(int32 flag)
{
	TArray<FName> result;
	if(flag & static_cast<int32>(EDataBundleType::Infos))
		result.Add(BUNDLE_INFOS);
	if(flag & static_cast<int32>(EDataBundleType::UI))
		result.Add(BUNDLE_UI);
	if(flag & static_cast<int32>(EDataBundleType::Spawn))
		result.Add(BUNDLE_SPAWN);
	if(flag & static_cast<int32>(EDataBundleType::Accessories))
		result.Add(BUNDLE_ACCESSORY);
	return result;
}
