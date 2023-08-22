// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentStructure.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

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

EPhysicalSurface AEnvironmentStructure::GetSurfaceType()
{
	EPhysicalSurface Result = EPhysicalSurface::SurfaceType_Default;
	UStaticMeshComponent* myMesh = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
	if (myMesh)
	{
		UMaterialInterface* myMaterial = myMesh->GetMaterial(0);
		if (myMaterial)
		{
			Result = myMaterial->GetPhysicalMaterial()->SurfaceType;
		}
	}

	return Result;
}

