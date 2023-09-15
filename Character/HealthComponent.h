// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, Health, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);

USTRUCT(BlueprintType)
struct FStatsParam
{
	GENERATED_BODY()
	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category="Health")
	FOnHealthChanged OnHealthChanged;
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category="Health")
	FOnDead OnDead;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Health")
	float Health = 400.0f;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
	float CoefDamage;
	
	UFUNCTION(BlueprintCallable, Category="Heath")
	float GetCurrentHealth();
	UFUNCTION(BlueprintCallable, Category="Heath")
	void SetCurrentHealth(float NewHealth);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Heath")
	virtual void ChangeHealthValue_OnServer(float Value);
	UFUNCTION(BlueprintNativeEvent)
	void Dead();

	UFUNCTION(NetMulticast, Reliable)
	void HealthChangedEvent_Multicast(float HealthValue, float Value);
	UFUNCTION(NetMulticast, Reliable)
	void DeadEvent_Multicast();
};
