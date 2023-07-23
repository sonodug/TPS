// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "States.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	E_AimState UMETA(DisplayName = "Aim State"),
	E_AimCrouchState UMETA(DisplayName = "AimCrouch State"),
	E_CrouchState UMETA(DisplayName = "Crouch State"),
	E_WalkState UMETA(DisplayName = "Walk State"),
	E_RunState UMETA(DisplayName = "Run State"),
};

USTRUCT(BlueprintType)
struct FCharacterSpeedInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float AimNormalSpeed = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float CrouchSpeed = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float AimCrouchSpeed = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float WalkSpeed = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float RunSpeed = 600.f;
};

UCLASS()
class TPS_API UStates : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};