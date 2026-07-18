// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveManager.generated.h"

class ACrystal;
class ABaseMonster;
class ASpawnPoint;

UCLASS()
class LOSTARK_API AWaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaveManager();

	void OnCrystalDestroyed();
	void OnMonsterKilled();

protected:
	virtual void BeginPlay() override;

private:
	int32 CurrentWave = 0;
	int32 AliveMonsterCount = 0;

	void SpawnMonsterWave();
	void SpawnNextCrystal();

public:
	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	int MaxNormalMonsterCount = 5;

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	int NamedNormalMonsterCount = 1;

	UPROPERTY(EditAnywhere, Category = "Chaos")
	TSubclassOf<class ACrystal> CrystalFactory;

	UPROPERTY(EditAnywhere, Category = "Chaos")
	TArray<ACrystal*> Crystals;

	UPROPERTY(EditAnywhere, Category = "Chaos")
	TArray<ASpawnPoint*> CrystalSpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Chaos")
	int32 CrystalCount = 2;

public:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ABaseMonster> NormalMonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ABaseMonster> NamedMonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 CurrentKillCount = 0;
};
