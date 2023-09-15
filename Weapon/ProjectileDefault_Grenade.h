// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Weapon/ProjectileDefault.h"
#include "ProjectileDefault_Grenade.generated.h"

/**
 * 
 */
UCLASS()
class TPS_API AProjectileDefault_Grenade : public AProjectileDefault
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void TimerExplose(float DeltaTime);

	virtual void BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	
	virtual void ImpactProjectile() override;

	void Explose();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Granade")
	bool TimerEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Granade")
	float TimerToExplose = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Granade")
	float TimeToExplose = 5.0f;
};
