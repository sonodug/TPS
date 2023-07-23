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
	
	MovementTick(DeltaSeconds);

	if (CurrentCursor)
	{
		APlayerController* MyPlayerController = Cast<APlayerController>(GetController());

		if (MyPlayerController)
		{
			FHitResult TraceHitResult;
			MyPlayerController->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();

			CurrentCursor->SetWorldLocation(TraceHitResult.Location);
			CurrentCursor->SetWorldRotation(CursorR);
		}
	}
}

void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitWeapon();

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
}

void ATPSCharacter::MovementTick(float DeltaTime)
{
	AddMovementInput(FVector(1.f, 0, 0), AxisX);
	AddMovementInput(FVector(0, 1.f, 0), AxisY);

	
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(PlayerController))
	{
		FHitResult HitResult;
		//Custon ETraceTypeQuery in UE C++ start with 6
		//PlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, HitResult);
		PlayerController->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, HitResult);
		float RotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HitResult.Location).Yaw;
		SetActorRotation(FQuat(FRotator(0, RotatorResultYaw,0)));
	}
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
}

void ATPSCharacter::InputAttackReleased()
{
}

void ATPSCharacter::CharacterUpdate()
{
	float ResultSpeed = START_SPEED;
	
	switch (MovementState)
	{
		case EMovementState::E_WalkState:
			ResultSpeed = SpeedInfo.WalkSpeed;
			break;
		case EMovementState::E_AimCrouchState:
			ResultSpeed = SpeedInfo.AimCrouchSpeed;
			break;
		case EMovementState::E_AimState:
			ResultSpeed = SpeedInfo.AimNormalSpeed;
			break;
		case EMovementState::E_CrouchState:
			ResultSpeed = SpeedInfo.CrouchSpeed;
			break;
		case EMovementState::E_RunState:
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
		MovementState = EMovementState::E_WalkState;
	else if (RunEnabled)
	{
		CrouchEnabled = false;
		AimEnabled = false;
		MovementState = EMovementState::E_RunState;
	}
	else if (CrouchEnabled && !RunEnabled && AimEnabled)
	{
		MovementState = EMovementState::E_AimCrouchState;
	}
	else if (CrouchEnabled && !RunEnabled && !AimEnabled)
	{
		MovementState = EMovementState::E_CrouchState;
	}
	else if (!CrouchEnabled && !RunEnabled && AimEnabled)
	{
		MovementState = EMovementState::E_AimState;
	}
	
	CharacterUpdate();
}

void ATPSCharacter::InitWeapon()
{
}

// AWeaponDefault* ATPSCharacter::GetCurrentWeapon()
// {
// }
