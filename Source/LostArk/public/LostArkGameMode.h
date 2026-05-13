// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LostArkGameMode.generated.h"

UCLASS(minimalapi)
class ALostArkGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALostArkGameMode();

	UPROPERTY(EditAnywhere, Category = "Chaos")
	TSubclassOf<class ACrystal> CrystalFactory;

	UPROPERTY(EditAnywhere, Category = "Chaos")
	TArray<AActor*> CrystalSpawnPoints;

	int32 CurrentKillCount = 0;

	UPROPERTY(EditAnywhere, Category = "Chaos")
	int32 TargetKillCount = 50;
};



