// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSCharacter.h"

#include "AIController.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "TimerManager.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Game/WeaponGameInstance.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TPS/TPS.h"

#define START_SPEED 300.f;

ATPSCharacter::ATPSCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	CharacterHealthComponent = CreateDefaultSubobject<UCharacterHealthComponent>(TEXT("CharacterHealthComponent"));

	if (CharacterHealthComponent)
		CharacterHealthComponent->OnDead.AddDynamic(this, &ATPSCharacter::Dead);
	
	if (InventoryComponent)
		InventoryComponent->OnSwitchWeapon.AddDynamic(this, &ATPSCharacter::InitWeapon);

	// Create a decal in the world to show the cursor's location
	/*CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/Blueprint/Character/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());*/

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
}

void ATPSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	/*if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}*/

	if (CurrentCursor)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());

		if (PlayerController && PlayerController->IsLocalPlayerController())
		{
			FHitResult TraceHitResult;
			PlayerController->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();

			CurrentCursor->SetWorldLocation(TraceHitResult.Location);
			CurrentCursor->SetWorldRotation(CursorR);
		}
	}

	MovementTick(DeltaSeconds);
}

void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitWeapon(InitWeaponNameDebug, InitWeaponInfoDebug);
	CurrentWeaponNameDebug = InitWeaponNameDebug;

	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));	
	}
}

void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis("MoveForward", this, &ATPSCharacter::InputAxisX);
	InputComponent->BindAxis("MoveRight", this, &ATPSCharacter::InputAxisY);
	
	InputComponent->BindAction(TEXT("FireEvent"), IE_Pressed, this, &ATPSCharacter::InputAttackPressed);
	InputComponent->BindAction(TEXT("FireEvent"), IE_Released, this, &ATPSCharacter::InputAttackReleased);
	InputComponent->BindAction(TEXT("ReloadEvent"), IE_Released, this, &ATPSCharacter::TryReloadWeapon);
	
	InputComponent->BindAction(TEXT("SwitchToNextWeapon"), IE_Pressed, this, &ATPSCharacter::TrySwitchToNextWeapon);
	InputComponent->BindAction(TEXT("SwitchToPreviousWeapon"), IE_Pressed, this, &ATPSCharacter::TrySwitchToPreviousWeapon);
	
	InputComponent->BindAction(TEXT("AbilityAction"), IE_Pressed, this, &ATPSCharacter::TryAbilityEnabled);
}

void ATPSCharacter::InputAxisX(float Value)
{
	AxisX = Value;
}

void ATPSCharacter::InputAxisY(float Value)
{
	AxisY = Value;
}

void ATPSCharacter::InputAttackPressed()
{
	AttackCharEvent(true);
}

void ATPSCharacter::InputAttackReleased()
{
	AttackCharEvent(false);
}

void ATPSCharacter::MovementTick(float DeltaTime)
{
	if (bIsAlive)
	{
		if (GetController() && GetController()->IsLocalPlayerController())
		{
			AddMovementInput(FVector(1.f, 0, 0), AxisX);
            AddMovementInput(FVector(0, 1.f, 0), AxisY);

			FString Enum = UEnum::GetValueAsString(GetMovementState());
			UE_LOG(LogTPS_Net, Warning, TEXT("Movement state - %s"), *Enum);
			
            if (MovementState == EMovementState::RunState)
            {
            	FVector RotationVector = FVector(AxisX,AxisY,0.0f);
            	FRotator Rotator = RotationVector.ToOrientationRotator();
            	SetActorRotation((FQuat(Rotator)));

            	//server -> multicast -> variable is replicated -> therefore state updated to all
            	SetActorRotationByYaw_OnServer(Rotator.Yaw);
            }
            else
            {
            	APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            	if (MyController)
            	{
            		FHitResult ResultHit;
            		//myController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);// bug was here Config\DefaultEngine.Ini
            		MyController->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);
    
            		float FindRotaterResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
            		SetActorRotation(FQuat(FRotator(0.0f, FindRotaterResultYaw, 0.0f)));
					SetActorRotationByYaw_OnServer(FindRotaterResultYaw);
            		
            		if (CurrentWeapon)
            		{
            			FVector Displacement = FVector(0);
            			bool bShouldReduce = false;
            			switch (MovementState)
            			{
            			case EMovementState::AimState:
            				Displacement = FVector(0.0f, 0.0f, 160.0f);
            				CurrentWeapon->ShouldReduceDispersion = true;
            				break;
            			case EMovementState::AimCrouchState:
            				CurrentWeapon->ShouldReduceDispersion = true;
            				Displacement = FVector(0.0f, 0.0f, 160.0f);
            				break;
            			case EMovementState::CrouchState:
            				Displacement = FVector(0.0f, 0.0f, 120.0f);
            				CurrentWeapon->ShouldReduceDispersion = false;
            				break;
            			case EMovementState::WalkState:
            				Displacement = FVector(0.0f, 0.0f, 120.0f);
            				CurrentWeapon->ShouldReduceDispersion = false;
            				break;
            			case EMovementState::RunState:
            				break;
            			default:
            				break;
            			}
            			
            			CurrentWeapon->UpdateWeaponByCharacterMovementState_OnServer(
            				ResultHit.Location + Displacement, bShouldReduce);
            			
            			//aim cursor like 3d Widget?
            		}
            	}
            }
		}
	}
}

void ATPSCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponDefault* Weapon = nullptr;
	Weapon = GetCurrentWeapon();
	if (Weapon)
	{
		//ToDo Check melee or range
		Weapon->SetWeaponStateFire_OnServer(bIsFiring);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::AttackCharEvent - CurrentWeapon -NULL"));
}

void ATPSCharacter::CharacterUpdate()
{
	float ResultSpeed = START_SPEED;
	
	switch (MovementState)
	{
		case EMovementState::WalkState:
			ResultSpeed = SpeedInfo.WalkSpeed;
			break;
		case EMovementState::AimCrouchState:
			ResultSpeed = SpeedInfo.AimCrouchSpeed;
			break;
		case EMovementState::AimState:
			ResultSpeed = SpeedInfo.AimNormalSpeed;
			break;
		case EMovementState::CrouchState:
			ResultSpeed = SpeedInfo.CrouchSpeed;
			break;
		case EMovementState::RunState:
			ResultSpeed = SpeedInfo.RunSpeed;
			break;
		default:
			break;
	}

	GetCharacterMovement()->MaxWalkSpeed = ResultSpeed;
}

void ATPSCharacter::ChangeMovementState()
{
	EMovementState NewState = EMovementState::WalkState;
	
	if (!CrouchEnabled && !RunEnabled && !AimEnabled)
		NewState = EMovementState::WalkState;
	else if (RunEnabled)
	{
		CrouchEnabled = false;
		AimEnabled = false;
		NewState = EMovementState::RunState;
	}
	else if (CrouchEnabled && !RunEnabled && AimEnabled)
	{
		NewState = EMovementState::AimCrouchState;
	}
	else if (CrouchEnabled && !RunEnabled && !AimEnabled)
	{
		NewState = EMovementState::CrouchState;
	}
	else if (!CrouchEnabled && !RunEnabled && AimEnabled)
	{
		NewState = EMovementState::AimState;
	}

	SetMovementState_OnServer(NewState);

	//Weapon state update
	AWeaponDefault* MyCurrentWeapon = GetCurrentWeapon();
	if (MyCurrentWeapon)
	{
		MyCurrentWeapon->UpdateStateWeapon_OnServer(NewState);
	}
}

EMovementState ATPSCharacter::GetMovementState()
{
	return MovementState;
}

AWeaponDefault* ATPSCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void ATPSCharacter::InitWeapon(FName WeaponName, FAdditionalWeaponInfo WeaponAdditionalInfo)
{
	//OnServer
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}
	
	UWeaponGameInstance* GameInstance = Cast<UWeaponGameInstance>(GetGameInstance());
	FWeaponInfo WeaponInfo;
	if (GameInstance)
	{
		if (GameInstance->GetWeaponInfoByName(WeaponName, WeaponInfo))
		{
			CurrentWeaponNameDebug = WeaponName;
			
			if (WeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();

				AWeaponDefault* MyWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(WeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (MyWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					MyWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = MyWeapon;
					MyWeapon->WeaponSetting = WeaponInfo;
					MyWeapon->AdditionalWeaponInfo.MagazineCapacity = WeaponInfo.MaxMagazineCapacity;
					//Remove !!! Debug
					MyWeapon->ReloadTime = WeaponInfo.ReloadTime;
					MyWeapon->UpdateStateWeapon_OnServer(MovementState);

					MyWeapon->AdditionalWeaponInfo = WeaponAdditionalInfo;
					if (InventoryComponent)
						CurrentWeaponIndex = InventoryComponent->GetWeaponSlotIndexByName(WeaponName);
					
					MyWeapon->OnWeaponReloadStart.AddDynamic(this, &ATPSCharacter::WeaponReloadStart);
					MyWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATPSCharacter::WeaponReloadEnd);
					
					MyWeapon->OnWeaponFireStart.AddDynamic(this, &ATPSCharacter::WeaponFireStart);

					if (CurrentWeapon->GetWeaponMagazine() <= 0 && CurrentWeapon->CheckCanWeaponReload())
						CurrentWeapon->InitReload();

					// OnWeaponAmmoAvailable
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::InitWeapon - Weapon not found in table -NULL"));
		}
	}
}

void ATPSCharacter::TrySwitchToNextWeapon()
{
	if (InventoryComponent->WeaponSlots.Num() > 1)
	{
		int8 OldIndex = CurrentWeaponIndex;
		FAdditionalWeaponInfo OldAdditionalInfo;
		if (CurrentWeapon)
		{
			OldAdditionalInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		// ToDo Bug with reset reload when switch
		if (InventoryComponent)
		{
			if (InventoryComponent->SwitchWeaponToIndex(CurrentWeaponIndex + 1, OldIndex, OldAdditionalInfo, true)) { }
		}
	}
}

void ATPSCharacter::TrySwitchToPreviousWeapon()
{
	if (InventoryComponent->WeaponSlots.Num() > 1)
	{
		int8 OldIndex = CurrentWeaponIndex;
		FAdditionalWeaponInfo OldAdditionalInfo;
		if (CurrentWeapon)
		{
			OldAdditionalInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		if (InventoryComponent)
		{
			if (InventoryComponent->SwitchWeaponToIndex(CurrentWeaponIndex - 1, OldIndex, OldAdditionalInfo, false)) { }
		}
	}
}

void ATPSCharacter::TryAbilityEnabled()
{
	if (AbilityEffect)
	{
		UStateEffect* NewEffect = NewObject<UStateEffect>(this, AbilityEffect);
		if (NewEffect)
			NewEffect->InitObject(this, NAME_None);
	}
}

void ATPSCharacter::TryReloadWeapon()
{
	if (bIsAlive && CurrentWeapon && !CurrentWeapon->WeaponReloading)
	{
		TryReloadWeapon_OnServer();
	}
}

void ATPSCharacter::WeaponReloadStart(UAnimMontage* AnimReloadHip, UAnimMontage* AnimReloadIronsight)
{
	WeaponReloadStart_BP(AnimReloadHip, AnimReloadIronsight);
}

void ATPSCharacter::WeaponReloadEnd(bool bIsSuccess, int32 AmmoLeft)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->WeaponChangeAmmo(CurrentWeapon->WeaponSetting.WeaponType, AmmoLeft);
		InventoryComponent->SetAdditionalWeaponInfo(CurrentWeaponIndex, CurrentWeapon->AdditionalWeaponInfo);
	}
	
	WeaponReloadEnd_BP(bIsSuccess, AmmoLeft);
}

void ATPSCharacter::WeaponFireStart(UAnimMontage* AnimFireHip, UAnimMontage* AnimFireIronsight)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->SetAdditionalWeaponInfo(CurrentWeaponIndex, CurrentWeapon->AdditionalWeaponInfo);
	}
	
	WeaponFireStart_BP(AnimFireHip, AnimFireIronsight);
}

void ATPSCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* AnimReloadHip, UAnimMontage* AnimReloadIronsight)
{
	// in BP
}

void ATPSCharacter::WeaponReloadEnd_BP_Implementation(bool bIsSucces, int32 AmmoLeft)
{
	// in BP
}

void ATPSCharacter::WeaponFireStart_BP_Implementation(UAnimMontage* AnimFireHip, UAnimMontage* AnimFireIronsight)
{
}

void ATPSCharacter::CharDead_BP_Implementation()
{
	//BP implement
}

UDecalComponent* ATPSCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}

bool ATPSCharacter::GetAliveState()
{
	return bIsAlive;
}

void ATPSCharacter::Dead()
{
	int32 RandomAnimNumber = FMath::RandHelper(DeadAnimMontages.Num());
	float AnimRate = 0.0f;
	
	if (DeadAnimMontages.IsValidIndex(RandomAnimNumber) && DeadAnimMontages[RandomAnimNumber] && GetMesh() && GetMesh()->GetAnimInstance())
	{
		AnimRate = DeadAnimMontages[RandomAnimNumber]->GetPlayLength();
		GetMesh()->GetAnimInstance()->Montage_Play(DeadAnimMontages[RandomAnimNumber]);
	}
	bIsAlive = false;

	if (GetController())
		GetController()->UnPossess();


	//Ragdoll
	GetWorldTimerManager().SetTimer(RagdollTimerHandle, this, &ATPSCharacter::EnableRagdoll, AnimRate - 0.1f, false);
	GetCursorToWorld()->SetVisibility(false);

	AttackCharEvent(false);
	CharDead_BP();
}

void ATPSCharacter::EnableRagdoll()
{
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		GetMesh()->SetSimulatePhysics(true);
	}
}

float ATPSCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                AActor* DamageCauser)
{
	if (bIsAlive)
		CharacterHealthComponent->ChangeHealthValue_OnServer(-DamageAmount);

	//AddEffectBySurfaceType (Radial Damage) by grenade launcher selfharm
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

EPhysicalSurface ATPSCharacter::GetSurfaceType()
{
	EPhysicalSurface Result = EPhysicalSurface::SurfaceType_Default;
	
	if (CharacterHealthComponent)
	{
		if (CharacterHealthComponent->GetCurrentShield() <= 0)
		{
			if (GetMesh())
			{
				UMaterialInterface* myMaterial = GetMesh()->GetMaterial(0);
				if (myMaterial)
					Result = myMaterial->GetPhysicalMaterial()->SurfaceType;
			}
		}
	}

	return Result;
}

TArray<UStateEffect*> ATPSCharacter::GetAllCurrentEffects()
{
	return Effects;
}

void ATPSCharacter::AddEffect(UStateEffect* Effect)
{
	Effects.Add(Effect);
	SwitchEffect(AddedEffect, true);
	AddedEffect = Effect;
}

void ATPSCharacter::RemoveEffect(UStateEffect* Effect)
{
	Effects.Remove(Effect);
	SwitchEffect(RemovedEffect, false);
	RemovedEffect = Effect;
}

void ATPSCharacter::EffectAdd_OnRep()
{
	if (AddedEffect)
	{
		SwitchEffect(AddedEffect, true);
	}
}

void ATPSCharacter::EffectRemove_OnRep()
{
	if (RemovedEffect)
	{
		SwitchEffect(RemovedEffect, true);	
	}
}

void ATPSCharacter::SwitchEffect(UStateEffect* Effect, bool bIsCanAdded)
{
	if (bIsCanAdded)
	{
		if (Effect && Effect->ParticleEffect)
		{
			FName NameBoneToAttached = Effect->BoneName;
			FVector Location = FVector(0);
			USkeletalMeshComponent* myMesh = GetMesh();
			
			if (myMesh)
			{
				UParticleSystemComponent* newParticleComp = UGameplayStatics::SpawnEmitterAttached(
					Effect->ParticleEffect,
					myMesh,
					NameBoneToAttached,
					Location,
					FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget,
					false);
				
				ParticleSystemEffects.Add(newParticleComp);
			}
		}		
	}
	else
	{
		int32 i = 0;
		bool bIsFind = false;
		while (i < ParticleSystemEffects.Num(), !bIsFind)
		{
			if (ParticleSystemEffects[i]->Template && Effect->ParticleEffect && Effect->ParticleEffect == ParticleSystemEffects[i]->Template)
			{
				bIsFind = true;
				ParticleSystemEffects[i]->DeactivateSystem();
				ParticleSystemEffects[i]->DestroyComponent();
				ParticleSystemEffects.RemoveAt(i);
			}
			i++;
		}
	}
}

void ATPSCharacter::TryReloadWeapon_OnServer_Implementation()
{
	if (CurrentWeapon->GetWeaponMagazine() < CurrentWeapon->WeaponSetting.MaxMagazineCapacity && CurrentWeapon->CheckCanWeaponReload())
		CurrentWeapon->InitReload();
}

void ATPSCharacter::SetMovementState_OnServer_Implementation(EMovementState NewState)
{
	SetMovementState_Multicast(NewState);
}

void ATPSCharacter::SetMovementState_Multicast_Implementation(EMovementState NewState)
{
	MovementState = NewState;
	CharacterUpdate();
}

void ATPSCharacter::SetActorRotationByYaw_OnServer_Implementation(float Yaw)
{
	SetActorRotationByYaw_Multicast(Yaw);
}

void ATPSCharacter::SetActorRotationByYaw_Multicast_Implementation(float Yaw)
{
	//SetActorRotation(FQuat(FRotator(0.0f, Yaw, 0.0f)));
	if (Controller && !Controller->IsLocalPlayerController())
	{
		SetActorRotation(FQuat(FRotator(0.0f, Yaw, 0.0f)));
	}
}

void ATPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// should be replicated
	DOREPLIFETIME(ATPSCharacter, MovementState);
	DOREPLIFETIME(ATPSCharacter, CurrentWeapon);
	DOREPLIFETIME(ATPSCharacter, AddedEffect);
	DOREPLIFETIME(ATPSCharacter, RemovedEffect);
	DOREPLIFETIME(ATPSCharacter, Effects);
}


