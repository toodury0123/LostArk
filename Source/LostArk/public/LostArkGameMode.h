// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Monster/BaseMonster.h"
#include "Monster/Crystal.h"

#include "LostArkGameMode.generated.h"


UCLASS(minimalapi)
class ALostArkGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALostArkGameMode();

	UPROPERTY(EditAnywhere, Category = "Chaos")
	TSubclassOf<class ACrystal> CrystalFactory;

	UPROPERTY(VisibleAnywhere, Category = "Chaos")
	TArray<ACrystal*> Crystals;

	UPROPERTY(EditAnywhere, Category = "Chaos")
	TArray<AActor*> CrystalSpawnPoints;

	UPROPERTY(VisibleAnywhere, Category = "Chaos")
	int CrystalCount = 2;

public:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ABaseMonster> NormalMonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ABaseMonster> NamedMonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 CurrentKillCount = 0;

public:

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	int MaxNormalMonsterCount = 5;

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	int NamedNormalMonsterCount = 1;

	//virtual void Tick(float deltaTime) override;
};



