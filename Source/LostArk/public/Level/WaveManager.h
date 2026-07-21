// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveManager.generated.h"

class ACrystal;
class ABaseMonster;
class ASpawnPoint;
class UUserWidget;

UCLASS()
class LOSTARK_API AWaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaveManager();

	void OnMonsterKilled();
	void AllMonsterKilled();

protected:
	virtual void BeginPlay() override;

private:
	int32 CurrentWave = 0;
	int32 AliveMonsterCount = 0;

	int32 FinalWave = 3;
	int32 GetCurrentWave() { return CurrentWave; }

public:
	void SpawnMonsterWave();
	void SpawnNextCrystal();

public:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

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

public:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ABaseMonster> NormalMonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ABaseMonster> NamedMonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float MonsterSpawnRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 CurrentKillCount = 0;

	void OnCrystalDestroyed();
	
	void GameOver();
};
