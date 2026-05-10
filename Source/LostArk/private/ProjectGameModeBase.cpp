// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGameModeBase.h"
#include "LostArk.h"

AProjectGameModeBase::AProjectGameModeBase()
{
	//PRINT_CALLINFO();
	UE_LOG(LogTemp, Warning, TEXT("%s"), *CALLINFO);
}