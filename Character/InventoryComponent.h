// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../States.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSwitchWeapon, FName, WeaponIdName, FAdditionalWeaponInfo, WeaponAdditionalInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, EWeaponType, AmmoType, int32, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAdditionalInfoChanged, int32, SlotIndex, FAdditionalWeaponInfo, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAmmoExpired, EWeaponType, WeaponType);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();
	FOnSwitchWeapon OnSwitchWeapon;
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnAmmoChanged OnAmmoChanged;
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnWeaponAdditionalInfoChanged OnWeaponAdditionalInfoChanged;
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnWeaponAmmoExpired OnWeaponAmmoExpired;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Weapons");
	TArray<FWeaponSlot> WeaponSlots;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Weapons");
	TArray<FAmmoSlot> AmmoSlots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons");
	int32 MaxWeaponSlots = 0;

	bool SwitchWeaponToIndex(int32 ChangeToIndex, int32 OldIndex, FAdditionalWeaponInfo OldInfo, bool bIsForward);
	FAdditionalWeaponInfo GetAdditionalWeaponInfo(int32 WeaponIndex);
	int32 GetWeaponSlotIndexByName(FName IdWeaponName);
	void SetAdditionalWeaponInfo(int32 WeaponIndex, FAdditionalWeaponInfo NewInfo);

	// govno -> rename
	void WeaponChangeAmmo(EWeaponType WeaponType, int32 AmmoToTake);
	bool CheckAmmoForWeapon(EWeaponType WeaponType, int8 &AvailableAmmoForWeapon);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inv")
	void InitInventory_OnServer(const TArray<FWeaponSlot>& NewWeaponSlotsInfo, const TArray<FAmmoSlot>& NewAmmoSlotsInfo);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Inv")
	void AmmoChangedEvent_Multicast(EWeaponType WeaponType, int32 Count);
};
