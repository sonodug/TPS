// Fill out your copyright notice in the Description page of Project Settings.


#include "States.h"
#include "TPS.h"


void UStates::AddEffectBySurfaceType(TSubclassOf<UStateEffect> Effect, EPhysicalSurface SurfaceType, AActor* HitActor)
{
	if (SurfaceType != EPhysicalSurface::SurfaceType_Default && HitActor && Effect)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddEffectBySurfaceType1"));
		UStateEffect* myEffect = Cast<UStateEffect>(Effect.GetDefaultObject());

		if (myEffect)
		{
			bool bIsCanBeAdded = false;
			int8 i = 0;
			while (i < myEffect->InteractableSurfaces.Num() && !bIsCanBeAdded)
			{
				if (myEffect->InteractableSurfaces[i] == SurfaceType)
				{
					bIsCanBeAdded = true;
					UStateEffect* NewEffect = NewObject<UStateEffect>(HitActor, Effect);
					
					if (NewEffect)
						NewEffect->InitObject(HitActor);
				}
				i++;
			}
		}
	}
}
