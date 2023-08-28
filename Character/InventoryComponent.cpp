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

	MaxWeaponSlots = WeaponSlots.Num();

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

bool UInventoryComponent::SwitchWeaponToIndex(int32 ChangeToIndex, int32 OldIndex, FAdditionalWeaponInfo OldInfo, bool bIsForward)
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

	if (WeaponSlots.IsValidIndex(CorrectIndex))
	{
		if (!WeaponSlots[CorrectIndex].ItemName.IsNone())
		{
			if (WeaponSlots[CorrectIndex].AdditionalWeaponInfo.MagazineCapacity > 0)
			{
				bIsSuccess = true;
			}
			else
			{
				UWeaponGameInstance* GI = Cast<UWeaponGameInstance>(GetWorld()->GetGameInstance());
				if (GI)
				{
					FWeaponInfo MyInfo;
					GI->GetWeaponInfoByName(WeaponSlots[CorrectIndex].ItemName, MyInfo);
					bool bIsFind = false;
					int8 j = 0;
					while (j < AmmoSlots.Num() && !bIsFind)
					{
						if (AmmoSlots[j].WeaponType == MyInfo.WeaponType && AmmoSlots[j].Count > 0)
						{
							bIsSuccess = true;
							bIsFind = true;
						}
						j++;
					}
				}
			}
			if (bIsSuccess)
			{
				NewWeaponName = WeaponSlots[CorrectIndex].ItemName;
				NewAdditionalWeaponInfo = WeaponSlots[CorrectIndex].AdditionalWeaponInfo;
			}
		}
	}

	// POSHEL GOVNOKODEC FIX FIX FIX
	if (!bIsSuccess)
	{
		if (bIsForward)
		{
			int8 Iteration = 0;
			int8 SecondIteration = 0;

			while (Iteration < WeaponSlots.Num() && !bIsSuccess)
			{
				Iteration++;
				int8 TempIndex = ChangeToIndex + Iteration;
				if (WeaponSlots.IsValidIndex(TempIndex))
				{
					if (!WeaponSlots[TempIndex].ItemName.IsNone())
					{
						if (WeaponSlots[TempIndex].AdditionalWeaponInfo.MagazineCapacity > 0)
						{
							bIsSuccess = true;
							NewWeaponName = WeaponSlots[TempIndex].ItemName;
							NewAdditionalWeaponInfo = WeaponSlots[TempIndex].AdditionalWeaponInfo;
						}
						else
						{
							FWeaponInfo MyWeaponInfo;
							UWeaponGameInstance* GI = Cast<UWeaponGameInstance>(GetWorld()->GetGameInstance());
							if (GI)
							{
								GI->GetWeaponInfoByName(WeaponSlots[TempIndex].ItemName, MyWeaponInfo);
								bool bIsFind = false;
								int8 j = 0;
								while (j < AmmoSlots.Num() && !bIsFind)
								{
									if (AmmoSlots[j].WeaponType == MyWeaponInfo.WeaponType && AmmoSlots[j].Count > 0)
									{
										bIsSuccess = true;
										NewWeaponName = WeaponSlots[TempIndex].ItemName;
										NewAdditionalWeaponInfo = WeaponSlots[TempIndex].AdditionalWeaponInfo;
										bIsFind = true;
									}
									j++;
								}
							}
						}
					}
				}
				else
				{
					if (OldIndex != SecondIteration)
					{
						if (WeaponSlots.IsValidIndex(SecondIteration))
						{
							if (!WeaponSlots[SecondIteration].ItemName.IsNone())
							{
								if (WeaponSlots[SecondIteration].AdditionalWeaponInfo.MagazineCapacity > 0)
								{
									bIsSuccess = true;
									NewWeaponName = WeaponSlots[SecondIteration].ItemName;
									NewAdditionalWeaponInfo = WeaponSlots[SecondIteration].AdditionalWeaponInfo;
								}
								else
								{
									FWeaponInfo MyWeaponInfo;
									UWeaponGameInstance* GI = Cast<UWeaponGameInstance>(GetWorld()->GetGameInstance());
									if (GI)
									{
										GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].ItemName, MyWeaponInfo);
										bool bIsFind = false;
										int8 j = 0;
										while (j < AmmoSlots.Num() && !bIsFind)
										{
											if (AmmoSlots[j].WeaponType == MyWeaponInfo.WeaponType && AmmoSlots[j].Count > 0)
											{
												bIsSuccess = true;
												NewWeaponName = WeaponSlots[SecondIteration].ItemName;
												NewAdditionalWeaponInfo = WeaponSlots[SecondIteration].AdditionalWeaponInfo;
												bIsFind = true;
											}
											j++;
										}
									}
								}
							}
						}
					}
					else
					{
						if (WeaponSlots.IsValidIndex(SecondIteration))
						{
							if (!WeaponSlots[SecondIteration].ItemName.IsNone())
							{
								if (WeaponSlots[SecondIteration].AdditionalWeaponInfo.MagazineCapacity > 0)
								{
									//Same weapon
								}
								else
								{
									FWeaponInfo MyWeaponInfo;
									UWeaponGameInstance* GI = Cast<UWeaponGameInstance>(GetWorld()->GetGameInstance());
									if (GI)
									{
										GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].ItemName, MyWeaponInfo);
										bool bIsFind = false;
										int8 j = 0;
										while (j < AmmoSlots.Num() && !bIsFind)
										{
											if (AmmoSlots[j].WeaponType == MyWeaponInfo.WeaponType)
											{
												
											}
											else
											{
												// Log mb
											}
											j++;
										}
									}
								}
							}
						}
					}
					SecondIteration++;
				}
			}
		}
		else
		{
			int8 Iteration = 0;
			int8 SecondIteration = WeaponSlots.Num() - 1;

			while (Iteration < WeaponSlots.Num() && !bIsSuccess)
			{
				Iteration++;
				int8 TempIndex = ChangeToIndex - Iteration;
				if (WeaponSlots.IsValidIndex(TempIndex))
				{
					if (!WeaponSlots[TempIndex].ItemName.IsNone())
					{
						if (WeaponSlots[TempIndex].AdditionalWeaponInfo.MagazineCapacity > 0)
						{
							bIsSuccess = true;
							NewWeaponName = WeaponSlots[TempIndex].ItemName;
							NewAdditionalWeaponInfo = WeaponSlots[TempIndex].AdditionalWeaponInfo;
						}
						else
						{
							FWeaponInfo MyWeaponInfo;
							UWeaponGameInstance* GI = Cast<UWeaponGameInstance>(GetWorld()->GetGameInstance());
							if (GI)
							{
								GI->GetWeaponInfoByName(WeaponSlots[TempIndex].ItemName, MyWeaponInfo);
								bool bIsFind = false;
								int8 j = 0;
								while (j < AmmoSlots.Num() && !bIsFind)
								{
									if (AmmoSlots[j].WeaponType == MyWeaponInfo.WeaponType && AmmoSlots[j].Count > 0)
									{
										bIsSuccess = true;
										NewWeaponName = WeaponSlots[TempIndex].ItemName;
										NewAdditionalWeaponInfo = WeaponSlots[TempIndex].AdditionalWeaponInfo;
										bIsFind = true;
									}
									j++;
								}
							}
						}
					}
				}
				else
				{
					if (OldIndex != SecondIteration)
					{
						if (WeaponSlots.IsValidIndex(SecondIteration))
						{
							if (!WeaponSlots[SecondIteration].ItemName.IsNone())
							{
								if (WeaponSlots[SecondIteration].AdditionalWeaponInfo.MagazineCapacity > 0)
								{
									bIsSuccess = true;
									NewWeaponName = WeaponSlots[SecondIteration].ItemName;
									NewAdditionalWeaponInfo = WeaponSlots[SecondIteration].AdditionalWeaponInfo;
								}
								else
								{
									FWeaponInfo MyWeaponInfo;
									UWeaponGameInstance* GI = Cast<UWeaponGameInstance>(GetWorld()->GetGameInstance());
									if (GI)
									{
										GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].ItemName, MyWeaponInfo);
										bool bIsFind = false;
										int8 j = 0;
										while (j < AmmoSlots.Num() && !bIsFind)
										{
											if (AmmoSlots[j].WeaponType == MyWeaponInfo.WeaponType && AmmoSlots[j].Count > 0)
											{
												bIsSuccess = true;
												NewWeaponName = WeaponSlots[SecondIteration].ItemName;
												NewAdditionalWeaponInfo = WeaponSlots[SecondIteration].AdditionalWeaponInfo;
												bIsFind = true;
											}
											j++;
										}
									}
								}
							}
						}
					}
					else
					{
						if (WeaponSlots.IsValidIndex(SecondIteration))
						{
							if (!WeaponSlots[SecondIteration].ItemName.IsNone())
							{
								if (WeaponSlots[SecondIteration].AdditionalWeaponInfo.MagazineCapacity > 0)
								{
									//Same weapon
								}
								else
								{
									FWeaponInfo MyWeaponInfo;
									UWeaponGameInstance* GI = Cast<UWeaponGameInstance>(GetWorld()->GetGameInstance());
									if (GI)
									{
										GI->GetWeaponInfoByName(WeaponSlots[SecondIteration].ItemName, MyWeaponInfo);
										bool bIsFind = false;
										int8 j = 0;
										while (j < AmmoSlots.Num() && !bIsFind)
										{
											if (AmmoSlots[j].WeaponType == MyWeaponInfo.WeaponType)
											{
												
											}
											else
											{
												// Log mb
											}
											j++;
										}
									}
								}
							}
						}
					}
					SecondIteration--;
				}
			}
		}
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
			if (i == WeaponIndex)
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
			Result = i;
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
			if (i == WeaponIndex)
			{
				WeaponSlots[i].AdditionalWeaponInfo = NewInfo;
				bIsFind = true;
				
				OnWeaponAdditionalInfoChanged.Broadcast(WeaponIndex, NewInfo);
			}
			i++;
		}
		if (!bIsFind)
			UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::SetAdditionalWeaponInfo - No Found weapon with index - %d"), WeaponIndex);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::SetAdditionalWeaponInfo - Not Correct weapon index - %d"), WeaponIndex);
}

void UInventoryComponent::WeaponChangeAmmo(EWeaponType WeaponType, int32 AmmoToTake)
{
	bool bIsFind = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bIsFind)
	{
		if (AmmoSlots[i].WeaponType == WeaponType)
		{
			AmmoSlots[i].Count -= AmmoToTake;
			if (AmmoSlots[i].Count > AmmoSlots[i].MaxCount)
				AmmoSlots[i].Count = AmmoSlots[i].MaxCount;

			OnAmmoChanged.Broadcast(AmmoSlots[i].WeaponType, AmmoSlots[i].Count);
			bIsFind = true;
		}
		i++;
	}
}

bool UInventoryComponent::CheckAmmoForWeapon(EWeaponType WeaponType, int8 &AvailableAmmoForWeapon)
{  
	AvailableAmmoForWeapon = 0;
	bool bIsFind = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bIsFind)
	{
		if (AmmoSlots[i].WeaponType == WeaponType)
		{
			bIsFind = true;
			AvailableAmmoForWeapon = AmmoSlots[i].Count;
			if (AmmoSlots[i].Count > 0)
				return true;
		}
		i++;
	}

	OnWeaponAmmoExpired.Broadcast(WeaponType);
	return false;
}

void UInventoryComponent::InitInventory(TArray<FWeaponSlot> NewWeaponSlotsInfo, TArray<FAmmoSlot> NewAmmoSlotsInfo)
{
	WeaponSlots = NewWeaponSlotsInfo;
	AmmoSlots = NewAmmoSlotsInfo;
	//Find init weaponsSlots and First Init Weapon
	for (int8 i = 0; i < WeaponSlots.Num(); i++)
	{
		UGameInstance* myGI = Cast<UGameInstance>(GetWorld()->GetGameInstance());
		if (myGI)
		{
			if (!WeaponSlots[i].ItemName.IsNone())
			{
				//FWeaponInfo Info;
				//if (myGI->GetWeaponInfoByName(WeaponSlots[i].NameItem, Info))
				//WeaponSlots[i].AdditionalInfo.Round = Info.MaxRound;
			}

		}
	}

	MaxWeaponSlots = WeaponSlots.Num();

	if (WeaponSlots.IsValidIndex(0))
	{
		if (!WeaponSlots[0].ItemName.IsNone())
			OnSwitchWeapon.Broadcast(WeaponSlots[0].ItemName, WeaponSlots[0].AdditionalWeaponInfo);
	}
}
