// Fill out your copyright notice in the Description page of Project Settings.


#include "../Character/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

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

void UHealthComponent::ChangeHealthValue(float Value)
{
	Health += Value;
	OnHealthChanged.Broadcast(Health, -Value);

	if (Health > 100.0f)
		Health = 100.0f;
	
	if (Health <= 0.0f)
		OnDead.Broadcast();
}

void UHealthComponent::Dead_Implementation()
{
	
}

