// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentStructure.h"

// Sets default values
AEnvironmentStructure::AEnvironmentStructure()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnvironmentStructure::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnvironmentStructure::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AEnvironmentStructure::AvaialableForEffects_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ATestInterface::AvaialableForEffects_Implementation()"));
	return true;
}

bool AEnvironmentStructure::AvailableForEffectsOnlyCPP()
{
	UE_LOG(LogTemp, Warning, TEXT("ATestInterface::AvailableForEffectsOnlyCPP()"));
	return true;
}

