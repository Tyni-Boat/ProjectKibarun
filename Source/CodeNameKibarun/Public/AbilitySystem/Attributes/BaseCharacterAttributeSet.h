

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "BaseCharacterAttributeSet.generated.h"


#define BASE_ATTRIBUTE_ACCESSORS(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


// Change Delta: The difference newValue - LastValue. HitLimit: True only if during this change the newValue got clamped (property reached his min/max value)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBaseAttributeEvent, float, ChangeDelta, float, NewValue, bool, HitLimit);


/**
 * The Root of all character Gas Attributes
 */
UCLASS(Abstract)
class CODENAMEKIBARUN_API UBaseCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
};
