// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Weapon/WeaponDefault.h"
#include "GameFramework/Character.h"
#include "TPS/States.h"
#include "TPSCharacter.generated.h"

UCLASS(Blueprintable)
class ATPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATPSCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	//FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;

public:
	//Cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cursor")
	UMaterialInterface* CursorMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cursor")
	FVector CursorSize = FVector(20.f, 40.f, 20.f);

	//Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	EMovementState MovementState = EMovementState::E_WalkState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	FCharacterSpeedInfo SpeedInfo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	bool RunEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	bool AimEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	bool CrouchEnabled = false;

	//Weapon	
	AWeaponDefault* CurrentWeapon = nullptr;

	//Debug demo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	FName InitWeaponName;
	
	UDecalComponent* CurrentCursor = nullptr;

	//InputHandlers
	UFUNCTION()
	void InputAxisX(float Value);
	UFUNCTION()
	void InputAxisY(float Value);
	UFUNCTION()
	void InputAttackPressed();
	UFUNCTION()
	void InputAttackReleased();

	float AxisX = 0.f;
	float AxisY = 0.f;
	
	//Functions
	UFUNCTION()
	void MovementTick(float DeltaTime);
	
	UFUNCTION(BlueprintCallable)
	void AttackCharEvent(bool bIsFiring);
	UFUNCTION(BlueprintCallable)
	void CharacterUpdate();
	UFUNCTION(BlueprintCallable)
	void ChangeMovementState();

	UFUNCTION(BlueprintCallable)
	AWeaponDefault* GetCurrentWeapon();
	UFUNCTION(BlueprintCallable)
	void InitWeapon(FName IdWeaponName);
	UFUNCTION(BlueprintCallable)
	void TryReloadWeapon();
	UFUNCTION()
	void WeaponReloadStart(UAnimMontage* Anim);
	UFUNCTION()
	void WeaponReloadEnd();
	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadStart_BP(UAnimMontage* Anim);
	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadEnd_BP();

	UFUNCTION(BlueprintCallable)
	UDecalComponent* GetCursorToWorld();
};

