// Fill out your copyright notice in the Description page of Project Settings.
#include "../Weapon/WeaponDefault.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TPS/StateEffects/StateEffect.h"
#include "TPS/Character/InventoryComponent.h"
#include "TPS/Interfaces/GameActor.h"

// Sets default values
AWeaponDefault::AWeaponDefault()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = SceneComponent;

	SkeletalMeshWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshWeapon->SetGenerateOverlapEvents(false);
	SkeletalMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	SkeletalMeshWeapon->SetupAttachment(RootComponent);

	StaticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh "));
	StaticMeshWeapon->SetGenerateOverlapEvents(false);
	StaticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshWeapon->SetupAttachment(RootComponent);

	ShootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootLocation->SetupAttachment(RootComponent);
	
	ShellDropLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShellDropLocation"));
	ShellDropLocation->SetupAttachment(RootComponent);
	
	MagazineDropPoint = CreateDefaultSubobject<USceneComponent>(TEXT("MagazineDropPoint"));
	MagazineDropPoint->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeaponDefault::BeginPlay()
{
	Super::BeginPlay();
	
	WeaponInit();
}

// Called every frame
void AWeaponDefault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		FireTick(DeltaTime);
		ReloadTick(DeltaTime);
		DispersionTick(DeltaTime);
	}
}

void AWeaponDefault::FireTick(float DeltaTime)
{
	// if (GetWeaponMagazine() > 0)
	
	if (WeaponFiring && GetWeaponMagazine() > 0 && !WeaponReloading)
		if (FireTimer < 0.f)
		{
			if (!WeaponReloading)
				Fire();
		}
		else
			FireTimer -= DeltaTime;
	
	// else
	// {
	// 	if (!WeaponReloading && CheckCanWeaponReload())
	// 	{
	// 		InitReload();
	// 	}
	// }
}

void AWeaponDefault::ReloadTick(float DeltaTime)
{
	if (WeaponReloading)
	{
		if (ReloadTimer < 0.0f)
		{
			FinishReload();
		}
		else
		{
			ReloadTimer -= DeltaTime;
		}
	}
}

void AWeaponDefault::DispersionTick(float DeltaTime)
{
	if (!WeaponReloading)
	{
		if (!WeaponFiring)
		{
			if (ShouldReduceDispersion)
				CurrentDispersion = CurrentDispersion - CurrentDispersionReduction;
			else
				CurrentDispersion = CurrentDispersion + CurrentDispersionReduction;
		}				

		if (CurrentDispersion < CurrentDispersionMin)
		{

			CurrentDispersion = CurrentDispersionMin;

		}
		else
		{
			if (CurrentDispersion > CurrentDispersionMax)
			{
				CurrentDispersion = CurrentDispersionMax;
			}
		}
	}
	if(ShowDebug)
		UE_LOG(LogTemp, Warning, TEXT("Dispersion: MAX = %f. MIN = %f. Current = %f"), CurrentDispersionMax, CurrentDispersionMin, CurrentDispersion);
}

void AWeaponDefault::WeaponInit()
{
	if (SkeletalMeshWeapon && !SkeletalMeshWeapon->SkeletalMesh)
	{
		SkeletalMeshWeapon->DestroyComponent(true);
	}

	if (StaticMeshWeapon && !StaticMeshWeapon->GetStaticMesh())
	{
		StaticMeshWeapon->DestroyComponent();
	}
	
	UpdateStateWeapon_OnServer(EMovementState::WalkState);
}

void AWeaponDefault::SetWeaponStateFire_OnServer_Implementation(bool bIsFire)
{
	if (CheckWeaponCanFire())
		WeaponFiring = bIsFire;
	else
		WeaponFiring = false;
	FireTimer = 0.01f;
}

bool AWeaponDefault::CheckWeaponCanFire()
{
	return !BlockFire;
}

FProjectileInfo AWeaponDefault::GetProjectile()
{	
	return WeaponSetting.ProjectileSetting;
}

void AWeaponDefault::Fire()
{
	FireTimer = WeaponSetting.RateOfFire;
	AdditionalWeaponInfo.MagazineCapacity = AdditionalWeaponInfo.MagazineCapacity - 1;
	ChangeDispersionByShot();
	
	WeaponFireFX_Multicast(WeaponSetting.EffectFireWeapon, WeaponSetting.SoundFireWeapon);
	
	//This is fragment shouldn't be here
	if(WeaponSetting.AnimFireHip && WeaponSetting.AnimFireIronsight)
	{
		OnWeaponFireStart.Broadcast(WeaponSetting.AnimFireHip, WeaponSetting.AnimFireIronsight);
		//AnimWeaponFire_Multicast(WeaponSetting.AnimFireIronsight);
	}
	
	int8 NumberProjectile = GetNumberProjectileByShot();
	
	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();
		FProjectileInfo ProjectileInfo;
		ProjectileInfo = GetProjectile();

		FVector EndLocation;
		for (int8 i = 0; i < NumberProjectile; i++)//Shotgun
		{
			EndLocation = GetFireEndLocation(); 

			FVector Direction = EndLocation - SpawnLocation;

			Direction.Normalize();

			FMatrix RotationMatrix(Direction, FVector(0, 1, 0), FVector(0, 0, 1), FVector::ZeroVector);
			SpawnRotation = RotationMatrix.Rotator();

			if (ProjectileInfo.Projectile)
			{
				//Projectile Init ballistic fire

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AProjectileDefault* myProjectile = Cast<AProjectileDefault>(GetWorld()->SpawnActor(ProjectileInfo.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myProjectile)
				{													
					myProjectile->InitProjectile(WeaponSetting.ProjectileSetting);

					AActor* SpawnedShellBullets = GetWorld()->SpawnActor<AActor>(
						WeaponSetting.DroppedShellBullets,
						ShellDropLocation->GetComponentLocation(),
						ShellDropLocation->GetComponentRotation(),
						SpawnParams);

					if (SpawnedShellBullets)
					{
						if (ShellDropLocation)
						{
							FVector ImpulseDirection = ShellDropLocation->GetForwardVector();
							float TestImpulseStrength = WeaponSetting.ImpulseStrength;

							// process exception (actor root may not has UMeshComponent)
							Cast<UMeshComponent>(SpawnedShellBullets->GetRootComponent())->AddImpulse(ImpulseDirection * TestImpulseStrength);
							
							FTimerHandle TimerHandle;
							GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, SpawnedShellBullets]()
							{
								if (SpawnedShellBullets)
								{
									SpawnedShellBullets->Destroy();
								}
							}, WeaponSetting.ShellBulletsDestroyTime, false);
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("ShellDropLocation is not set! Cannot apply impulse."));
							SpawnedShellBullets->Destroy();
						}
					}
				}
			}
			else
			{
				//Multicast trace
				FHitResult Hit;
				TArray<AActor*> Actors;				

				EDrawDebugTrace::Type DebugTrace;
				if (ShowDebug)
					{
						DrawDebugLine(GetWorld(), SpawnLocation, SpawnLocation + ShootLocation->GetForwardVector()*WeaponSetting.DistacneTrace, FColor::Black, false, 5.f, (uint8)'\000', 0.5f);
						DebugTrace = EDrawDebugTrace::ForDuration;
					}
				else
					DebugTrace = EDrawDebugTrace::None;
				
				UKismetSystemLibrary::LineTraceSingle(GetWorld(), SpawnLocation, EndLocation * WeaponSetting.DistacneTrace,
					ETraceTypeQuery::TraceTypeQuery4, false, Actors, DebugTrace, Hit, true, FLinearColor::Red,FLinearColor::Green, 5.0f);
		
				if (Hit.GetActor() && Hit.PhysMaterial.IsValid())
				{
					EPhysicalSurface mySurfacetype = UGameplayStatics::GetSurfaceType(Hit);

					if (WeaponSetting.ProjectileSetting.HitDecals.Contains(mySurfacetype))
					{
						UMaterialInterface* myMaterial = WeaponSetting.ProjectileSetting.HitDecals[mySurfacetype];

						if (myMaterial && Hit.GetComponent())
						{
							UGameplayStatics::SpawnDecalAttached(myMaterial, FVector(20.0f), Hit.GetComponent(), NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 10.0f);
						}
					}
					if (WeaponSetting.ProjectileSetting.HitFXs.Contains(mySurfacetype))
					{
						UParticleSystem* myParticle = WeaponSetting.ProjectileSetting.HitFXs[mySurfacetype];
						if (myParticle)
						{
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), myParticle, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, FVector(1.0f)));
						}
					}

					if (WeaponSetting.ProjectileSetting.HitSound)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponSetting.ProjectileSetting.HitSound, Hit.ImpactPoint);
					}
					
					UStates::AddEffectBySurfaceType(ProjectileInfo.Effect, Hit.BoneName, mySurfacetype, Hit.GetActor());							
												
					UGameplayStatics::ApplyPointDamage(Hit.GetActor(), WeaponSetting.ProjectileSetting.ProjectileDamage, Hit.TraceStart,Hit, GetInstigatorController(),this,NULL);				
				}
			}
		}				
	}

	if (GetWeaponMagazine() <= 0 && !WeaponReloading)
	{
		if (CheckCanWeaponReload())
			InitReload();
	}
}

void AWeaponDefault::UpdateStateWeapon_OnServer_Implementation(EMovementState NewMovementState)
{
	//ToDo Dispersion
	BlockFire = false;

	switch (NewMovementState)
	{
	case EMovementState::AimState:
		
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case EMovementState::AimCrouchState:
		
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case EMovementState::CrouchState:
		
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case EMovementState::WalkState:
		
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Run_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Run_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Run_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case EMovementState::RunState:
		BlockFire = true;
		SetWeaponStateFire_OnServer(false);//set fire trigger to false
		//Block Fire
		break;
	default:
		break;
	}
}

void AWeaponDefault::ChangeDispersionByShot()
{
	CurrentDispersion = CurrentDispersion + CurrentDispersionRecoil;
}

float AWeaponDefault::GetCurrentDispersion() const
{
	return CurrentDispersion;
}

FVector AWeaponDefault::ApplyDispersionToShoot(FVector DirectionShoot) const
{		
	return FMath::VRandCone(DirectionShoot, GetCurrentDispersion() * PI / 180.f);
}

FVector AWeaponDefault::GetFireEndLocation() const
{
	bool bShootDirection = false;
	FVector EndLocation = FVector(0.f);
	
	FVector tmpV = (ShootLocation->GetComponentLocation() - ShootEndLocation);
	//UE_LOG(LogTemp, Warning, TEXT("Vector: X = %f. Y = %f. Size = %f"), tmpV.X, tmpV.Y, tmpV.Size());

	if(tmpV.Size() > SizeVectorToChangeShootDirectionLogic)
	{
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot((ShootLocation->GetComponentLocation() - ShootEndLocation).GetSafeNormal()) * -20000.0f;
		if(ShowDebug)
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), -(ShootLocation->GetComponentLocation() - ShootEndLocation), WeaponSetting.DistacneTrace, GetCurrentDispersion()* PI / 180.f, GetCurrentDispersion()* PI / 180.f, 32, FColor::Emerald, false, .1f, (uint8)'\000', 1.0f);
	}	
	else
	{
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(ShootLocation->GetForwardVector()) * 20000.0f;
		if (ShowDebug)
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), ShootLocation->GetForwardVector(), WeaponSetting.DistacneTrace, GetCurrentDispersion()* PI / 180.f, GetCurrentDispersion()* PI / 180.f, 32, FColor::Emerald, false, .1f, (uint8)'\000', 1.0f);
	}
	
	if (ShowDebug)
	{
		//direction weapon look
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector() * 500.0f, FColor::Cyan, false, 5.f, (uint8)'\000', 0.5f);
		//direction projectile must fly
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), ShootEndLocation, FColor::Red, false, 5.f, (uint8)'\000', 0.5f);
		//Direction Projectile Current fly
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), EndLocation, FColor::Black, false, 5.f, (uint8)'\000', 0.5f);

		//DrawDebugSphere(GetWorld(), ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector()*SizeVectorToChangeShootDirectionLogic, 10.f, 8, FColor::Red, false, 4.0f);
	}
	
	return EndLocation;
}

int8 AWeaponDefault::GetNumberProjectileByShot() const
{
	return WeaponSetting.NumberProjectileByShot;
}

int32 AWeaponDefault::GetWeaponMagazine()
{
	return AdditionalWeaponInfo.MagazineCapacity;
}

void AWeaponDefault::InitReload()
{
	//On server
	WeaponReloading = true;
	
	ReloadTimer = WeaponSetting.ReloadTime;

	// ToDo add sound reload location 
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundReloadWeapon, ShootLocation->GetComponentLocation());
	
	if(WeaponSetting.AnimReloadHip && WeaponSetting.AnimReloadIronsight)
		OnWeaponReloadStart.Broadcast(WeaponSetting.AnimReloadHip, WeaponSetting.AnimReloadIronsight);
	
	if (WeaponSetting.DroppedMagazine)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = GetInstigator();
		
		AActor* SpawnedMagazine = GetWorld()->SpawnActor<AActor>(
			 WeaponSetting.DroppedMagazine,
			 MagazineDropPoint->GetComponentLocation(),
			 MagazineDropPoint->GetComponentRotation(),
			 SpawnParams);
		
		if (SpawnedMagazine)
		{
			//Actor Lifetime timer
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, SpawnedMagazine]()
			{
				if (SpawnedMagazine)
				{
					SpawnedMagazine->Destroy();
				}
			}, WeaponSetting.MagazineDestroyTime, false);
		}
	}
}

void AWeaponDefault::FinishReload()
{
	WeaponReloading = false;
	int8 AvailableAmmoFromInventory = GetAvailableAmmoForReload();
	int8 AmmoToTakeFromInventory;
	int8 AmmoToReload = WeaponSetting.MaxMagazineCapacity - AdditionalWeaponInfo.MagazineCapacity;

	if (AmmoToReload > AvailableAmmoFromInventory)
	{
		AdditionalWeaponInfo.MagazineCapacity = AvailableAmmoFromInventory;
		AmmoToTakeFromInventory = AvailableAmmoFromInventory;
	}
	else
	{
		AdditionalWeaponInfo.MagazineCapacity += AmmoToReload;
		AmmoToTakeFromInventory = AmmoToReload;
	}
	
	OnWeaponReloadEnd.Broadcast(true, AmmoToTakeFromInventory);
}

void AWeaponDefault::CancelReload()
{
	WeaponReloading = false;
	if (SkeletalMeshWeapon && SkeletalMeshWeapon->GetAnimInstance())
		SkeletalMeshWeapon->GetAnimInstance()->StopAllMontages(0.15f);

	OnWeaponReloadEnd.Broadcast(false, 0);
	DropClip = false;
}

bool AWeaponDefault::CheckCanWeaponReload()
{
	bool bResult = true;
	if (GetOwner())
	{
		UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(GetOwner()->GetComponentByClass(UInventoryComponent::StaticClass()));
		if (InventoryComponent)
		{
			int8 ResultAmmo;
			if (!InventoryComponent->CheckAmmoForWeapon(WeaponSetting.WeaponType, ResultAmmo))
			{
				bResult = false;
			}
		}
	}

	return bResult;
}

int8 AWeaponDefault::GetAvailableAmmoForReload()
{
	int8 ResultAmmo = WeaponSetting.MaxMagazineCapacity;

	if (GetOwner())
	{
		UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(GetOwner()->GetComponentByClass(UInventoryComponent::StaticClass()));
		if (InventoryComponent)
		{
			if (InventoryComponent->CheckAmmoForWeapon(WeaponSetting.WeaponType, ResultAmmo))
			{
				// ResultAmmo = Result.Ammo;
			}
		}
	}

	return ResultAmmo;
}

void AWeaponDefault::WeaponFireFX_Multicast_Implementation(UParticleSystem* FireFX, USoundBase* SoundFire)
{
	if (SoundFire)
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SoundFire, ShootLocation->GetComponentLocation());
	
	if (FireFX)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireFX, ShootLocation->GetComponentTransform());
}

void AWeaponDefault::MagazineDropReload_Multicast_Implementation()
{
	//pohui
}

void AWeaponDefault::ShellDropFire_Multicast_Implementation()
{
	//pohui
}

void AWeaponDefault::AnimWeaponReload_Multicast_Implementation(UAnimMontage* AnimReload)
{
	//this logic in BP (rework)
}

void AWeaponDefault::AnimWeaponFire_Multicast_Implementation(UAnimMontage* AnimFire)
{
	//this logic in BP (rework)
}

void AWeaponDefault::UpdateWeaponByCharacterMovementState_OnServer_Implementation(FVector NewShootEndLocation, bool NewShouldReduceDispersion)
{
	ShootEndLocation = NewShootEndLocation;
	ShouldReduceDispersion = NewShouldReduceDispersion;
}

void AWeaponDefault::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponDefault, AdditionalWeaponInfo);
	DOREPLIFETIME(AWeaponDefault, ShootEndLocation);
	DOREPLIFETIME(AWeaponDefault, WeaponReloading);
}
