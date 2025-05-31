

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CommonCharacterAttributeSet.generated.h"

/**
 * Attribute set possessed by almost all characters
 */
UCLASS(BlueprintType, Blueprintable)
class CODENAMEKIBARUN_API UCommonCharacterAttributeSet : public UBaseCharacterAttributeSet
{
	GENERATED_BODY()

public:

	void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;


	// Max Health _________________________________________________________________
	
	UPROPERTY(BlueprintAssignable)
	FBaseAttributeEvent OnMaxHealthChanged;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxHealth;
	
	BASE_ATTRIBUTE_ACCESSORS(UCommonCharacterAttributeSet, MaxHealth);

	// Health _________________________________________________________________
	
	UPROPERTY(BlueprintAssignable)
	FBaseAttributeEvent OnHealthChanged;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Health;
	
	BASE_ATTRIBUTE_ACCESSORS(UCommonCharacterAttributeSet, Health);
	
};
