// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/WaveManager.h"
#include "Level/Crystal.h"
#include "Level/SpawnPoint.h"

// Sets default values
AWaveManager::AWaveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AWaveManager::BeginPlay()
{
	Super::BeginPlay();

	SpawnNextCrystal();
}

void AWaveManager::SpawnNextCrystal()
{
	if (!CrystalFactory || CrystalSpawnPoints.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("CrystalFactory 또는 SpawnPoint 없음"));
		return;
	}

	ASpawnPoint* SpawnPoint = CrystalSpawnPoints[0];

	if (!SpawnPoint)
		return;

	ACrystal* Crystal = GetWorld()->SpawnActor<ACrystal>(
		CrystalFactory,
		SpawnPoint->GetActorLocation(),
		SpawnPoint->GetActorRotation()
	);

	if (Crystal)
	{
		Crystals.Add(Crystal);
	}
}