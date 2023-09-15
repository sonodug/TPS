// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "../Weapon/ProjectileDefault.h"
#include "../States.h"
#include "WeaponDefault.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFireStart);//ToDo Delegate on event weapon fire - Anim char, state char...
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponReloadStart, UAnimMontage*, AnimReloadHip, UAnimMontage*, AnimReloadIronsight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponReloadEnd, bool, bIsSucces, int32, AmmoLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFireStart, UAnimMontage*, AnimFireHip, UAnimMontage*, AnimFireIronsight);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponFireStart, UAnimMontage*, AnimFire);

UCLASS()
class TPS_API AWeaponDefault : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponDefault();

	FOnWeaponReloadEnd OnWeaponReloadEnd;
	FOnWeaponReloadStart OnWeaponReloadStart;
	FOnWeaponFireStart OnWeaponFireStart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	USceneComponent* SceneComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	USkeletalMeshComponent* SkeletalMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	UStaticMeshComponent* StaticMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	UArrowComponent* ShootLocation = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	UArrowComponent* ShellDropLocation = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	USceneComponent* MagazineDropPoint = nullptr;

	UPROPERTY()
	FWeaponInfo WeaponSetting;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	FAdditionalWeaponInfo AdditionalWeaponInfo;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Tick func
	virtual void Tick(float DeltaTime) override;

	void FireTick(float DeltaTime);
	void ReloadTick(float DeltaTime);
	void DispersionTick(float DeltaTime);

	void WeaponInit();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLogic")
	bool WeaponFiring = false;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
	bool WeaponReloading = false;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetWeaponStateFire_OnServer(bool bIsFire);

	bool CheckWeaponCanFire();

	FProjectileInfo GetProjectile();

	void Fire();

	UFUNCTION(Server, Reliable)
	void UpdateStateWeapon_OnServer(EMovementState NewMovementState);
	
	void ChangeDispersionByShot();
	float GetCurrentDispersion() const;
	FVector ApplyDispersionToShoot(FVector DirectionShoot) const;

	FVector GetFireEndLocation() const;
	int8 GetNumberProjectileByShot() const;

	//Timers
	float FireTimer = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
	float ReloadTimer = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic Debug")	//Remove !!! Debug
	float ReloadTime = 0.0f;
	
	//Flags
	bool BlockFire = false;
	bool DropClip = false;
	
	//Dispersion params
	bool ShouldReduceDispersion = false;
	float CurrentDispersion = 0.0f;
	float CurrentDispersionMax = 1.0f;
	float CurrentDispersionMin = 0.1f;
	float CurrentDispersionRecoil = 0.1f;
	float CurrentDispersionReduction = 0.1f;

	UPROPERTY(Replicated)
	FVector ShootEndLocation = FVector(0);

	UFUNCTION(BlueprintCallable)
	int32 GetWeaponMagazine();
	void InitReload();
	void FinishReload();
	void CancelReload();

	bool CheckCanWeaponReload();
	int8 GetAvailableAmmoForReload();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float SizeVectorToChangeShootDirectionLogic = 100.0f;

	UFUNCTION(Server, Unreliable)
	void UpdateWeaponByCharacterMovementState_OnServer(FVector NewShootEndLocation, bool NewShouldReduceDispersion);
	UFUNCTION(NetMulticast, Unreliable)
	void AnimWeaponFire_Multicast(UAnimMontage* AnimFire);
	UFUNCTION(NetMulticast, Unreliable)
	void AnimWeaponReload_Multicast(UAnimMontage* AnimReload);
	UFUNCTION(NetMulticast, Unreliable)
	void ShellDropFire_Multicast();
	UFUNCTION(NetMulticast, Unreliable)
	void MagazineDropReload_Multicast();
	UFUNCTION(NetMulticast, Unreliable)
	void WeaponFireFX_Multicast(UParticleSystem* FireFX, USoundBase* SoundFire);
};