// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	virtual bool InitObject();
	virtual bool ExecuteObject(float DeltaTime);
	virtual void DestroyObject();

	virtual bool CheckCanStack();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	TArray<TEnumAsByte<EPhysicalSurface>> InteractableSurfaces;
};
