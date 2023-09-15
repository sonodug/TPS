// Fill out your copyright notice in the Description page of Project Settings.

#include "../Weapon/ProjectileDefault.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Damage.h"
#include "TPS/Interfaces/GameActor.h"

// Sets default values
AProjectileDefault::AProjectileDefault()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

	BulletCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));

	BulletCollisionSphere->SetSphereRadius(16.f);

	BulletCollisionSphere->bReturnMaterialOnMove = true;//hit event return physMaterial

	BulletCollisionSphere->SetCanEverAffectNavigation(false);//collision not affect navigation (P keybord on editor)

	RootComponent = BulletCollisionSphere;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Projectile Mesh"));
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetCanEverAffectNavigation(false);

	BulletFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Bullet FX"));
	BulletFX->SetupAttachment(RootComponent);

	//BulletSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Bullet Audio"));
	//BulletSound->SetupAttachment(RootComponent);

	BulletProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Bullet ProjectileMovement"));
	BulletProjectileMovement->UpdatedComponent = RootComponent;
	BulletProjectileMovement->InitialSpeed = 1.f;
	BulletProjectileMovement->MaxSpeed = 0.f;

	BulletProjectileMovement->bRotationFollowsVelocity = true;
	BulletProjectileMovement->bShouldBounce = true;
}

// Called when the game starts or when spawned
void AProjectileDefault::BeginPlay()
{
	Super::BeginPlay();

	BulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereHit);
	BulletCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereBeginOverlap);
	BulletCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereEndOverlap);
}

// Called every frame
void AProjectileDefault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectileDefault::InitProjectile(FProjectileInfo InitParam)
{
	BulletProjectileMovement->InitialSpeed = InitParam.ProjectileInitSpeed;
	BulletProjectileMovement->MaxSpeed = InitParam.ProjectileMaxSpeed;
	this->SetLifeSpan(InitParam.ProjectileLifeTime);
	
	if (BulletMesh && !BulletMesh->GetStaticMesh())
		BulletMesh->DestroyComponent(true);

	if (!BulletFX->Template)
		BulletFX->DestroyComponent(true);

	ProjectileSetting = InitParam;
}

void AProjectileDefault::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && Hit.PhysMaterial.IsValid())
	{
		EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
		
		if (ProjectileSetting.HitDecals.Contains(SurfaceType))
		{
			UMaterialInterface* MyMaterial = ProjectileSetting.HitDecals[SurfaceType];

			if (MyMaterial && OtherComp)
			{
				SpawnHitDecal_Multicast(MyMaterial, OtherComp, Hit);
			}
		}
		if (ProjectileSetting.HitFXs.Contains(SurfaceType))
		{
			UParticleSystem* myParticle = ProjectileSetting.HitFXs[SurfaceType];
			if (myParticle)
			{
				SpawnHitFX_Multicast(myParticle, Hit);
			}
		}
			
		if (ProjectileSetting.HitSound)
		{
			SpawnHitSound_Multicast(ProjectileSetting.HitSound, Hit);
		}
		
		//IGameActor* myInterface = Cast<IGameActor>(Hit.GetActor());
		UStates::AddEffectBySurfaceType(ProjectileSetting.Effect, Hit.BoneName, SurfaceType, Hit.GetActor());
	}
	
	UGameplayStatics::ApplyDamage(OtherActor, ProjectileSetting.ProjectileDamage, GetInstigatorController(), this, NULL);
	UAISense_Damage::ReportDamageEvent(GetWorld(), Hit.GetActor(), GetInstigator(), ProjectileSetting.ProjectileDamage, Hit.Location, Hit.Location);
	ImpactProjectile();	
	//UGameplayStatics::ApplyRadialDamageWithFalloff()
	//Apply damage cast to if char like bp? //OnAnyTakeDmage delegate
	//UGameplayStatics::ApplyDamage(OtherActor, ProjectileSetting.ProjectileDamage, GetOwner()->GetInstigatorController(), GetOwner(), NULL);
	//or custom damage by health component
	
}

void AProjectileDefault::BulletCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AProjectileDefault::BulletCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AProjectileDefault::ImpactProjectile()
{
	this->Destroy();
}

void AProjectileDefault::SpawnHitSound_Multicast_Implementation(USoundBase* HitSound, FHitResult HitResult)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitResult.ImpactPoint);
}

void AProjectileDefault::SpawnHitFX_Multicast_Implementation(UParticleSystem* FxTemplate, FHitResult HitResult)
{
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		FxTemplate,
		FTransform(HitResult.ImpactNormal.Rotation(), HitResult.ImpactPoint, FVector(1.0f)));
}

void AProjectileDefault::SpawnHitDecal_Multicast_Implementation(UMaterialInterface* DecalMaterial,
                                                           UPrimitiveComponent* OtherComp, FHitResult HitResult)
{
	UGameplayStatics::SpawnDecalAttached(
		DecalMaterial,
		FVector(15.0f),
		OtherComp,
		NAME_None,
		HitResult.ImpactPoint,
		HitResult.ImpactNormal.Rotation(),
		EAttachLocation::KeepWorldPosition,10.0f);
}

void AProjectileDefault::InitVisualTrailProjectile_Multicast_Implementation(UStaticMesh* newMesh,
                                                                            FTransform MeshRelative)
{
	//set bullet trail by projectileinfo (now he's gone) dynamically
}

void AProjectileDefault::InitVisualMeshProjectile_Multicast_Implementation(UStaticMesh* newMesh,
                                                                           FTransform MeshRelative)
{
	//set bullet static mesh by projectileinfo (now he's gone) dynamically
}
