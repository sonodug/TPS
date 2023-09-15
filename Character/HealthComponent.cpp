// Fill out your copyright notice in the Description page of Project Settings.


#include "../Character/HealthComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UHealthComponent::GetCurrentHealth()
{
	return Health;
}

void UHealthComponent::SetCurrentHealth(float NewHealth)
{
	Health = NewHealth;
}

void UHealthComponent::ChangeHealthValue_OnServer_Implementation(float Value)
{
	Health += Value;
	HealthChangedEvent_Multicast(Health, -Value);

	if (Health > 100.0f)
		Health = 100.0f;
	
	if (Health <= 0.0f)
		DeadEvent_Multicast();
}

void UHealthComponent::DeadEvent_Multicast_Implementation()
{
	OnDead.Broadcast();
}

void UHealthComponent::HealthChangedEvent_Multicast_Implementation(float HealthValue, float Value)
{
	OnHealthChanged.Broadcast(HealthValue, -Value);
}

void UHealthComponent::Dead_Implementation()
{
	
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, Health);
}
