// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/GameActor.h"
#include "EnvironmentStructure.generated.h"

UCLASS()
class TPS_API AEnvironmentStructure : public AActor, public IGameActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AEnvironmentStructure();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool AvaialableForEffects_Implementation() override;
	bool AvailableForEffectsOnlyCPP() override;
};