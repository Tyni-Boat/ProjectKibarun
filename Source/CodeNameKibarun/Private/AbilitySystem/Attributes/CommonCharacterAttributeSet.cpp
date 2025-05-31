


#include "AbilitySystem/Attributes/CommonCharacterAttributeSet.h"
#include "GameplayEffectExtension.h"




void UCommonCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float diff = Data.EvaluatedData.Magnitude;
		const bool reachedLimit = (GetHealth() <= 0 && (GetHealth() - diff) > 0) || (GetHealth() >= GetMaxHealth() && (GetHealth() - diff) < GetMaxHealth());
		OnHealthChanged.Broadcast(Data.EvaluatedData.Magnitude, GetHealth(), reachedLimit);
	}
	if(Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		OnMaxHealthChanged.Broadcast(Data.EvaluatedData.Magnitude, GetMaxHealth(), false);
	}
}
