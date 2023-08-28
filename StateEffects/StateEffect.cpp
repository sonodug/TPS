// Fill out your copyright notice in the Description page of Project Settings.

   
#include "../StateEffects/StateEffect.h"

#include "../Character/CharacterHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "TPS/Interfaces/GameActor.h"

bool UStateEffect::InitObject(AActor* Actor, FName NameBoneHit)
{
	UE_LOG(LogTemp, Warning, TEXT("BaseSE"));
	TargetActor = Actor;
	
	IGameActor* myInterface = Cast<IGameActor>(TargetActor);

	if (myInterface)
		myInterface->AddEffect(this);
	
	return true;
}

void UStateEffect::DestroyObject()
{
	IGameActor* myInterface = Cast<IGameActor>(TargetActor);

	if (myInterface)
		myInterface->RemoveEffect(this);
	
	TargetActor = nullptr;
	
	if (this && this->IsValidLowLevel())
	{
		this->ConditionalBeginDestroy();
	}		
}

//Childs methods

//Single

bool UStateEffect_Single::InitObject(AActor* Actor, FName NameBoneHit)
{
	UE_LOG(LogTemp, Warning, TEXT("UStateEffect_Single"));
	Super::InitObject(Actor, NameBoneHit);
	ExecuteOnce();
	return true;
}

void UStateEffect_Single::DestroyObject()
{
	Super::DestroyObject();
}

void UStateEffect_Single::ExecuteOnce()
{
	if (TargetActor)
	{
		UHealthComponent* MyHealthComponent = Cast<UHealthComponent>(
	TargetActor->GetComponentByClass(UHealthComponent::StaticClass()));

		if (MyHealthComponent)
		{
			MyHealthComponent->ChangeHealthValue(Power);
		}	
	}
	
	DestroyObject();
}

//Timer

bool UStateEffect_Timer::InitObject(AActor* Actor, FName NameBoneHit)
{
	UE_LOG(LogTemp, Warning, TEXT("UStateEffect_Timer"));

	Super::InitObject(Actor, NameBoneHit);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_EffectTimer, this, &UStateEffect_Timer::DestroyObject, Timer, false);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ExecuteTimer, this, &UStateEffect_Timer::Execute, RateTime, true);

	if (ParticleEffect)
	{
		FName NameBoneToAttached = NameBoneHit;
		FVector Location = FVector(0);

		USceneComponent* TargetMesh = Cast<USceneComponent>(TargetActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
		if (TargetMesh)
			ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(
				ParticleEffect,
				TargetMesh,
				NameBoneToAttached,
				Location,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false);
		else
		{
			ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(
				ParticleEffect,
				TargetActor->GetRootComponent(),
				NameBoneToAttached,
				Location,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false);
		}
	}

	return true;
}

void UStateEffect_Timer::DestroyObject()
{
	ParticleEmitter->DestroyComponent();
	ParticleEmitter = nullptr;
	Super::DestroyObject();
}

void UStateEffect_Timer::Execute()
{
	if (TargetActor)
	{
		UHealthComponent* MyHealthComponent = Cast<UHealthComponent>(
	TargetActor->GetComponentByClass(UHealthComponent::StaticClass()));

		if (MyHealthComponent)
		{
			MyHealthComponent->ChangeHealthValue(Power);
		}	
	}
}
