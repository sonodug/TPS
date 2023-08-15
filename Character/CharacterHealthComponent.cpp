// Fill out your copyright notice in the Description page of Project Settings.


#include "../Character/CharacterHealthComponent.h"

void UCharacterHealthComponent::ChangeHealthValue(float Value)
{
	Super::ChangeHealthValue(Value);
}

float UCharacterHealthComponent::GetCurrentShield()
{
	return 0.0f;
}

void UCharacterHealthComponent::ChangeShieldValue(float Value)
{
}

void UCharacterHealthComponent::CoolDownShieldEnd()
{
}

void UCharacterHealthComponent::RecoveryShield()
{
}

float UCharacterHealthComponent::GetShieldValue()
{
	return 0.0f;
}
