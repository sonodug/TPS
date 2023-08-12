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
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Game/WeaponGameInstance.h"

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

	if (InventoryComponent)
	{
		InventoryComponent->OnSwitchWeapon.AddDynamic(this, &ATPSCharacter::InitWeapon);
	}

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

		if (PlayerController)
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
	AddMovementInput(FVector(1.f, 0, 0), AxisX);
	AddMovementInput(FVector(0, 1.f, 0), AxisY);
	
	if (MovementState == EMovementState::RunState)
	{
		FVector RotationVector = FVector(AxisX,AxisY,0.0f);
		FRotator Rotator = RotationVector.ToOrientationRotator();
		SetActorRotation((FQuat(Rotator)));
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

			if (CurrentWeapon)
			{
				FVector Displacement = FVector(0);
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
				
				CurrentWeapon->ShootEndLocation = ResultHit.Location + Displacement;
				//aim cursor like 3d Widget?
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
		Weapon->SetWeaponStateFire(bIsFiring);
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
	if (!CrouchEnabled && !RunEnabled && !AimEnabled)
		MovementState = EMovementState::WalkState;
	else if (RunEnabled)
	{
		CrouchEnabled = false;
		AimEnabled = false;
		MovementState = EMovementState::RunState;
	}
	else if (CrouchEnabled && !RunEnabled && AimEnabled)
	{
		MovementState = EMovementState::AimCrouchState;
	}
	else if (CrouchEnabled && !RunEnabled && !AimEnabled)
	{
		MovementState = EMovementState::CrouchState;
	}
	else if (!CrouchEnabled && !RunEnabled && AimEnabled)
	{
		MovementState = EMovementState::AimState;
	}
	
	CharacterUpdate();

	//Weapon state update
	AWeaponDefault* MyCurrentWeapon = GetCurrentWeapon();
	if (MyCurrentWeapon)
	{
		MyCurrentWeapon->UpdateStateWeapon(MovementState);
	}
}

AWeaponDefault* ATPSCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void ATPSCharacter::InitWeapon(FName WeaponName, FAdditionalWeaponInfo WeaponAdditionalInfo)
{
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
					MyWeapon->UpdateStateWeapon(MovementState);

					MyWeapon->AdditionalWeaponInfo = WeaponAdditionalInfo;
					if (InventoryComponent)
						CurrentWeaponIndex = InventoryComponent->GetWeaponSlotIndexByName(WeaponName);
					
					MyWeapon->OnWeaponReloadStart.AddDynamic(this, &ATPSCharacter::WeaponReloadStart);
					MyWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATPSCharacter::WeaponReloadEnd);
					
					MyWeapon->OnWeaponFireStart.AddDynamic(this, &ATPSCharacter::WeaponFireStart);
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

		if (InventoryComponent)
		{
			if (InventoryComponent->SwitchWeaponToIndex(CurrentWeaponIndex + 1, OldIndex, OldAdditionalInfo))
			{
				
			}
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
			if (InventoryComponent->SwitchWeaponToIndex(CurrentWeaponIndex - 1, OldIndex, OldAdditionalInfo))
			{
				
			}
		}
	}
}

void ATPSCharacter::TryReloadWeapon()
{
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading)
	{
		if (CurrentWeapon->GetWeaponMagazine() < CurrentWeapon->WeaponSetting.MaxMagazineCapacity && CurrentWeapon->CheckCanWeaponReload())
			CurrentWeapon->InitReload();
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

UDecalComponent* ATPSCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}

