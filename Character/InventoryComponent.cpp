// Fill out your copyright notice in the Description page of Project Settings.


#include "../Character/InventoryComponent.h"

#include "TPS/Game/WeaponGameInstance.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	for	(int8 i = 0; i < WeaponSlots.Num(); i++)
	{
		UWeaponGameInstance* GI = Cast<UWeaponGameInstance>(GetWorld()->GetGameInstance());

		if (GI)
		{
			if (!WeaponSlots[i].ItemName.IsNone())
			{
				FWeaponInfo Info;
				if (GI->GetWeaponInfoByName(WeaponSlots[i].ItemName, Info))
					WeaponSlots[i].AdditionalWeaponInfo.MagazineCapacity = Info.MaxMagazineCapacity;
				else
				{
					WeaponSlots.RemoveAt(i);
					i--;
				}
			}
		}
	}

	if (WeaponSlots.IsValidIndex(0))
	{
		if (!WeaponSlots[0].ItemName.IsNone())
			OnSwitchWeapon.Broadcast(WeaponSlots[0].ItemName, WeaponSlots[0].AdditionalWeaponInfo);
	}
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UInventoryComponent::SwitchWeaponToIndex(int32 ChangeToIndex, int32 OldIndex, FAdditionalWeaponInfo OldInfo)
{
	bool bIsSuccess = false;
	int8 CorrectIndex = ChangeToIndex;
	if (ChangeToIndex > WeaponSlots.Num() - 1)
		CorrectIndex = 0;
	else
	{
		if (ChangeToIndex < 0)
			CorrectIndex = WeaponSlots.Num() - 1;
	}

	FName NewWeaponName;
	FAdditionalWeaponInfo NewAdditionalWeaponInfo;

	int8 i = 0;
	while (i < WeaponSlots.Num() && !bIsSuccess)
	{
		if (WeaponSlots[i].SlotIndex == CorrectIndex)
		{
			if (!WeaponSlots[i].ItemName.IsNone())
			{
				NewWeaponName = WeaponSlots[i].ItemName;
				NewAdditionalWeaponInfo = WeaponSlots[i].AdditionalWeaponInfo;
				bIsSuccess = true;
			}
		}
		i++;
	}

	if (!bIsSuccess)
	{
		
	}

	if (bIsSuccess)
	{
		SetAdditionalWeaponInfo(OldIndex, OldInfo);
		OnSwitchWeapon.Broadcast(NewWeaponName, NewAdditionalWeaponInfo);
	}

	return bIsSuccess;
}

FAdditionalWeaponInfo UInventoryComponent::GetAdditionalWeaponInfo(int32 WeaponIndex)
{
	FAdditionalWeaponInfo Result;
	if (WeaponSlots.IsValidIndex(WeaponIndex))
	{
		bool bIsFind = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFind)
		{
			if (WeaponSlots[i].SlotIndex == WeaponIndex)
			{
				Result = WeaponSlots[i].AdditionalWeaponInfo;
				bIsFind = true;
			}
			i++;
		}
		if (!bIsFind)
			UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::GetAdditionalWeaponInfo - No Found weapon with index - %d"), WeaponIndex);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::GetAdditionalWeaponInfo - Not Correct weapon index - %d"), WeaponIndex);

	return Result;
}

int32 UInventoryComponent::GetWeaponSlotIndexByName(FName IdWeaponName)
{
	int32 Result = -1;
	int i = 0;
	bool bIsFind = false;
	while (i < WeaponSlots.Num() && !bIsFind)
	{
		if (WeaponSlots[i].ItemName == IdWeaponName)
		{
			bIsFind = false;
			Result = WeaponSlots[i].SlotIndex;
		}
		i++;
	}
	return Result;
}

void UInventoryComponent::SetAdditionalWeaponInfo(int32 WeaponIndex, FAdditionalWeaponInfo NewInfo)
{
	if (WeaponSlots.IsValidIndex(WeaponIndex))
	{
		bool bIsFind = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFind)
		{
			if (WeaponSlots[i].SlotIndex == WeaponIndex)
			{
				WeaponSlots[i].AdditionalWeaponInfo = NewInfo;
				bIsFind = true;
			}
			i++;
		}
		if (!bIsFind)
			UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::SetAdditionalWeaponInfo - No Found weapon with index - %d"), WeaponIndex);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::SetAdditionalWeaponInfo - Not Correct weapon index - %d"), WeaponIndex);
}

