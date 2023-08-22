// Fill out your copyright notice in the Description page of Project Settings.


#include "StateEffect.h"


bool UStateEffect::InitObject()
{
	UE_LOG(LogTemp, Warning, TEXT("UStateEffect::InitObject()"));
	return true;
}

bool UStateEffect::ExecuteObject(float DeltaTime)
{
	return true;
}

void UStateEffect::DestroyObject()
{
	
}

bool UStateEffect::CheckCanStack()
{
	return true;
}
