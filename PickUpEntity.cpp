// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpEntity.h"

#include "Character/TPSCharacter.h"

// Sets default values
APickUpEntity::APickUpEntity()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetCanEverAffectNavigation(false);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionSphere->SetCanEverAffectNavigation(false);
	
	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));

	RootComponent = SceneComponent;
	StaticMeshComponent->SetupAttachment(RootComponent);
	CollisionSphere->SetupAttachment(RootComponent);
	ParticleSystemComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickUpEntity::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickUpEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
