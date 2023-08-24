// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPS/StateEffects/StateEffect.h"
#include "UObject/Interface.h"
#include "GameActor.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGameActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TPS_API IGameActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual EPhysicalSurface GetSurfaceType();
	virtual TArray<UStateEffect*> GetAllCurrentEffects();
	virtual void RemoveEffect(UStateEffect* Effect);
	virtual void AddEffect(UStateEffect* Effect);
	
};
