// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Particles/ParticleEmitter.h"
#include "UObject/NoExportTypes.h"
#include "StateEffect.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TPS_API UStateEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual bool InitObject(AActor* Actor, FName NameBoneHit);
	virtual void DestroyObject();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	TArray<TEnumAsByte<EPhysicalSurface>> InteractableSurfaces;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	bool bCanStack = false;
	
	AActor* TargetActor = nullptr;
};

UCLASS()
class TPS_API UStateEffect_Single : public UStateEffect
{
	GENERATED_BODY()
public:
	virtual bool InitObject(AActor* Actor, FName NameBoneHit) override;
	virtual void DestroyObject() override;

	virtual void ExecuteOnce();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings SingleEffect")
	float Power = 20.0f;
};

UCLASS()
class TPS_API UStateEffect_Timer : public UStateEffect
{
	GENERATED_BODY()
public:
	virtual bool InitObject(AActor* Actor, FName NameBoneHit) override;
	virtual void DestroyObject() override;

	virtual void Execute();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings TimerEffect")
	float Power = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings TimerEffect")
	float Timer = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings TimerEffect")
	float RateTime = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings TimerEffect")
	UParticleSystem* ParticleEffect = nullptr;

	UParticleSystemComponent* ParticleEmitter = nullptr;
	
	FTimerHandle TimerHandle_ExecuteTimer;
	FTimerHandle TimerHandle_EffectTimer;
	
	
};