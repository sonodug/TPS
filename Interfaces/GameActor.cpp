// Fill out your copyright notice in the Description page of Project Settings.


#include "../Interfaces/GameActor.h"

// Add default functionality here for any IGameActor functions that are not pure virtual.
EPhysicalSurface IGameActor::GetSurfaceType()
{
	UE_LOG(LogTemp, Warning, TEXT("IGameActor::GetSurfaceType()"));
	return SurfaceType_Default;
}

TArray<UStateEffect*> IGameActor::GetAllCurrentEffects()
{
	TArray<UStateEffect*> Effects;
	return Effects;
}

void IGameActor::RemoveEffect(UStateEffect* Effect)
{
}

void IGameActor::AddEffect(UStateEffect* Effect)
{
}
