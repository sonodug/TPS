// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StateEffect.generated.h"

/**
 * 
 */
UCLASS()
class TPS_API UStateEffect : public UObject
{
	GENERATED_BODY()

	virtual bool InitObject(APawn* Pawn);
	virtual bool ExecuteObject(float DeltaTime);
	virtual void DestroyObject();
};
