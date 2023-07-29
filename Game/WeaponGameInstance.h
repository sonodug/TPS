// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "../Weapon/WeaponDefault.h"
#include "WeaponGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TPS_API UWeaponGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
 
public:
	//table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " WeaponSetting ")
	UDataTable* WeaponInfoTable = nullptr;
	UFUNCTION(BlueprintCallable)
	bool GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo);
};