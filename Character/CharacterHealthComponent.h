// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Character/HealthComponent.h"
#include "CharacterHealthComponent.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldChanged, float, Shield, float, Damage);

UCLASS()
class TPS_API UCharacterHealthComponent : public UHealthComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnShieldChanged OnShieldChanged;

	FTimerHandle TimerHandle_CollDownShieldTimer;
	FTimerHandle TimerHandle_ShieldRecoveryRateTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield")
	float CooldownShieldRecoverTime = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield")
	float ShieldRecoverValue = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield")
	float ShieldRecoverRate = 0.1f;
	
protected:
	float Shield = 100.0f;

public:
	virtual void ChangeHealthValue_OnServer(float Value) override;
	float GetCurrentShield();
	void ChangeShieldValue(float Value);
	void CoolDownShieldEnd();
	void RecoveryShield();
	
	UFUNCTION(BlueprintCallable)
	float GetShieldValue();
	UFUNCTION(NetMulticast, Reliable)
	void ShieldChangedEvent_Multicast(float ShieldValue, float Damage);
};
