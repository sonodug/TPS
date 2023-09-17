// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterHealthComponent.h"
#include "HealthComponent.h"
#include "InventoryComponent.h"
#include "../Weapon/WeaponDefault.h"
#include "GameFramework/Character.h"
#include "TPS/States.h"
#include "TPS/Interfaces/GameActor.h"
#include "TPSCharacter.generated.h"

UCLASS(Blueprintable)
class ATPSCharacter : public ACharacter, public IGameActor
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
	UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UDecalComponent* CursorToWorld;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	UCharacterHealthComponent* CharacterHealthComponent;

public:
	//Dead
	FTimerHandle RagdollTimerHandle;
	
	//Cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cursor")
	UMaterialInterface* CursorMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cursor")
	FVector CursorSize = FVector(20.f, 40.f, 20.f);

	//Movement
	UPROPERTY(Replicated)
	EMovementState MovementState = EMovementState::WalkState; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement") 
	FCharacterSpeedInfo SpeedInfo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	bool RunEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	bool AimEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	bool CrouchEnabled = false;

	//Dead
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OtherStates")
	bool bIsAlive = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OtherStates")
	TArray<UAnimMontage*> DeadAnimMontages;

	//Effects
	UPROPERTY(Replicated)
	TArray<UStateEffect*> Effects;
	UPROPERTY(ReplicatedUsing=EffectAdd_OnRep)
	UStateEffect* AddedEffect = nullptr;
	UPROPERTY(ReplicatedUsing=EffectRemove_OnRep)
	UStateEffect* RemovedEffect = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AblilityEffect")
	TSubclassOf<UStateEffect> AbilityEffect;
	 
	//Weapon
	UPROPERTY(Replicated)
	AWeaponDefault* CurrentWeapon = nullptr;
	int32 CurrentWeaponIndex = 0;

	//Debug demo
	// DEBUG ONLY
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	FName InitWeaponNameDebug;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	FAdditionalWeaponInfo InitWeaponInfoDebug;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Demo")
	FName CurrentWeaponNameDebug;
	
	UDecalComponent* CurrentCursor = nullptr;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Debug")
	TArray<UParticleSystemComponent*> ParticleSystemEffects;
	
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
	UFUNCTION()
	EMovementState GetMovementState();

	UFUNCTION(BlueprintCallable)
	AWeaponDefault* GetCurrentWeapon();
	UFUNCTION(BlueprintCallable)
	void InitWeapon(FName WeaponName, FAdditionalWeaponInfo WeaponAdditionalInfo);
	UFUNCTION(BlueprintCallable)
	void TryReloadWeapon();

	//Inventory
	UFUNCTION()
	void TrySwitchToNextWeapon();
	UFUNCTION()
	void TrySwitchToPreviousWeapon();
	
	void TryAbilityEnabled();
	
	UFUNCTION()
	void WeaponReloadStart(UAnimMontage* AnimReloadHip, UAnimMontage* AnimReloadIronsight);
	UFUNCTION()
	void WeaponReloadEnd(bool bIsSuccess, int32 AmmoLeft);
	UFUNCTION()
	void WeaponFireStart(UAnimMontage* AnimFireHip, UAnimMontage* AnimFireIronsight);
	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadStart_BP(UAnimMontage* AnimReloadHip, UAnimMontage* AnimReloadIronsight);
	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadEnd_BP(bool bIsSucces, int32 AmmoLeft);
	UFUNCTION(BlueprintNativeEvent)
	void WeaponFireStart_BP(UAnimMontage* AnimFireHip, UAnimMontage* AnimFireIronsight);
	UFUNCTION(BlueprintNativeEvent)
	void CharDead_BP();

	UFUNCTION(BlueprintCallable)
	UDecalComponent* GetCursorToWorld();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetAliveState();

	// BlueprintCallable macros parameter - default
	UFUNCTION(BlueprintCallable)
	void Dead();
	void EnableRagdoll();
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	//Interface
	virtual EPhysicalSurface GetSurfaceType() override;
	virtual TArray<UStateEffect*> GetAllCurrentEffects() override;
	virtual void AddEffect(UStateEffect* Effect) override;
	virtual void RemoveEffect(UStateEffect* Effect) override;

	UFUNCTION(Server, Unreliable)
	void SetActorRotationByYaw_OnServer(float Yaw);
	UFUNCTION(NetMulticast, Unreliable)
	void SetActorRotationByYaw_Multicast(float Yaw);
	UFUNCTION(Server, Reliable)
	void SetMovementState_OnServer(EMovementState NewState);
	UFUNCTION(NetMulticast, Reliable)
	void SetMovementState_Multicast(EMovementState NewState);
	UFUNCTION(Server, Reliable)
	void TryReloadWeapon_OnServer();

	UFUNCTION()
	void EffectAdd_OnRep();
	UFUNCTION()
	void EffectRemove_OnRep();
	UFUNCTION()
	void SwitchEffect(UStateEffect* Effect, bool bIsCanAdded);
};

