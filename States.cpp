// Fill out your copyright notice in the Description page of Project Settings.


#include "States.h"
#include "TPS.h"
#include "Interfaces/GameActor.h"


void UStates::AddEffectBySurfaceType(TSubclassOf<UStateEffect> Effect, EPhysicalSurface SurfaceType, AActor* HitActor)
{
	if (SurfaceType != EPhysicalSurface::SurfaceType_Default && HitActor && Effect)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddEffectBySurfaceType1"));
		UStateEffect* myEffect = Cast<UStateEffect>(Effect.GetDefaultObject());

		if (myEffect)
		{
			bool bIsHavingInteractableSurface = false;
			int8 i = 0;
			while (i < myEffect->InteractableSurfaces.Num() && !bIsHavingInteractableSurface)
			{
				if (myEffect->InteractableSurfaces[i] == SurfaceType)
				{
					bIsHavingInteractableSurface = true;

					bool bCanAddEffect = false;
					if (!myEffect->bCanStack)
					{
						int8 j = 0;
						TArray<UStateEffect*> CurrentEffects;
						IGameActor* myInterface = Cast<IGameActor>(HitActor);
						if (myInterface)
							CurrentEffects = myInterface->GetAllCurrentEffects();

						if (CurrentEffects.Num() > 0)
						{
							while (j < CurrentEffects.Num() && !bCanAddEffect)
							{
								if (CurrentEffects[j]->GetClass() != Effect)
									bCanAddEffect = true;
								
								j++;
							}
						}
						else
							bCanAddEffect = true;
					}
					else
						bCanAddEffect = true;

					if (bCanAddEffect)
					{
						UStateEffect* NewEffect = NewObject<UStateEffect>(HitActor, Effect);
					
						if (NewEffect)
							NewEffect->InitObject(HitActor);
					}
				}
				i++;
			}
		}
	}
}
