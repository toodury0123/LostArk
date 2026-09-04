// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/WaveManager.h"
#include "Level/Crystal.h"
#include "Level/SpawnPoint.h"

#include "Player/LostArkCharacter.h"
#include "Player/LostArkHUD.h"

#include "Monster/BaseMonster.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AWaveManager::AWaveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AWaveManager::BeginPlay()
{
	Super::BeginPlay();

	TotalMonsterCount =
		(MaxNormalMonsterCount * FinalWave)
		+ NamedNormalMonsterCount;

	CurrentKillCount = 0;

	SpawnNextCrystal();
}

void AWaveManager::OnMonsterKilled()
{
	AliveMonsterCount--;
	CurrentKillCount++;

	UE_LOG(LogTemp, Warning,
		TEXT("Monster Killed / CurrentWave: %d / RemainingTotal: %d"),
		CurrentWave,
		AliveMonsterCount);

	if (LostArkHUD)
	{
		LostArkHUD->UpdateMonsterCount(CurrentKillCount);
	}

	if (AliveMonsterCount <= 0)
	{
		AllMonsterKilled();
	}
}

void AWaveManager::AllMonsterKilled()
{
	CurrentWave++;

	if (CurrentWave > FinalWave)
	{
		GameOver();
		return;
	}

	SpawnNextCrystal();
}


void AWaveManager::SpawnMonsterWave()
{
	// ---------- Crystal Setup ----------
	const int32 CrystalIndex = CurrentWave - 1;

	UE_LOG(LogTemp, Warning,
		TEXT("SpawnMonsterWave / CurrentWave: %d / FinalWave: %d"),
		CurrentWave,
		FinalWave);

	if (!Crystals.IsValidIndex(CrystalIndex) || !Crystals[CrystalIndex])
	{
		return;
	}

	ACrystal* CurrentCrystal = Crystals[CrystalIndex];


	// ---------- Spawn Settings ----------
	const FVector CrystalLocation = CurrentCrystal->GetActorLocation();

	AliveMonsterCount = 0;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;



	// ---------- Normal Monsters ----------

	for (int32 i = 0; i < MaxNormalMonsterCount; ++i)
	{
		const FVector SpawnLocation = CrystalLocation + FVector(
			FMath::RandRange(-MonsterSpawnRadius, MonsterSpawnRadius),
			FMath::RandRange(-MonsterSpawnRadius, MonsterSpawnRadius),
			0.f
		);

		const FRotator SpawnRotation(
			0.f,
			FMath::RandRange(0.f, 360.f),
			0.f
		);

		ABaseMonster* SpawnedMonster = GetWorld()->SpawnActor<ABaseMonster>(
			NormalMonsterClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (SpawnedMonster)
		{
			SpawnedMonster->SetWaveManager(this);
			AliveMonsterCount++;
		}
	}
	
	// ---------- Named Monsters ----------

	if (CurrentWave == FinalWave)
	{
		for (int32 i = 0; i < NamedNormalMonsterCount; ++i)
		{
			const FVector SpawnLocation = CrystalLocation + FVector(
				FMath::RandRange(-MonsterSpawnRadius, MonsterSpawnRadius),
				FMath::RandRange(-MonsterSpawnRadius, MonsterSpawnRadius),
				0.f
			);

			const FRotator SpawnRotation(
				0.f,
				FMath::RandRange(0.f, 360.f),
				0.f
			);

			ABaseMonster* SpawnedMonster = GetWorld()->SpawnActor<ABaseMonster>(
				NamedMonsterClass,
				SpawnLocation,
				SpawnRotation,
				SpawnParams
			);

			if (SpawnedMonster)
			{

				UE_LOG(LogTemp, Error, TEXT("RARE MONSTER SPAWN"));

				SpawnedMonster->SetWaveManager(this);
				AliveMonsterCount++;
			}
		}
	}
}

void AWaveManager::SpawnNextCrystal()
{
	UE_LOG(LogTemp, Warning,
		TEXT("CurrentWave = %d, FinalWave = %d, SpawnPoint Num = %d"),
		CurrentWave,
		FinalWave,
		CrystalSpawnPoints.Num());

	if (CurrentWave > FinalWave)
	{
		UE_LOG(LogTemp, Error, TEXT("CurrentWave > FinalWave"));
		return;
	}
	const int32 SpawnPointIndex = CurrentWave - 1;

	if (!CrystalSpawnPoints.IsValidIndex(SpawnPointIndex))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Invalid SpawnPoint Index: %d"),
			SpawnPointIndex);
		return;
	}

	ASpawnPoint* SpawnPoint = CrystalSpawnPoints[SpawnPointIndex];

	if (!SpawnPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnPoint is nullptr"));
		return;
	}

	if (!CrystalFactory)
	{
		UE_LOG(LogTemp, Error, TEXT("CrystalFactory is nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Using SpawnPoint: %s, Location: %s"),
		*GetNameSafe(SpawnPoint),
		*SpawnPoint->GetActorLocation().ToString());

	ACrystal* Crystal = GetWorld()->SpawnActor<ACrystal>(
		CrystalFactory,
		SpawnPoint->GetActorLocation(),
		SpawnPoint->GetActorRotation()
	);

	if (!Crystal)
	{
		UE_LOG(LogTemp, Error, TEXT("Crystal Spawn Failed"));
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Crystal Spawn Success: %s"),
		*GetNameSafe(Crystal));

	AliveMonsterCount = 0;

	if (Crystal)
	{
		Crystal->SetWaveManager(this);
		Crystals.Add(Crystal);
	}
}

void AWaveManager::OnCrystalDestroyed()
{
	SpawnMonsterWave();

	UE_LOG(LogTemp, Warning, TEXT("Crystal Destroyed"));
}


void AWaveManager::GameOver()
{
	UE_LOG(LogTemp, Error, TEXT("GameOver Called"));

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	UE_LOG(LogTemp, Error,
		TEXT("PC: %s / WidgetClass: %s"),
		*GetNameSafe(PC),
		*GetNameSafe(GameOverWidgetClass));

	if (PC && GameOverWidgetClass)
	{
		UUserWidget* Widget =
			CreateWidget<UUserWidget>(PC, GameOverWidgetClass);

		if (Widget)
		{
			UE_LOG(LogTemp, Error, TEXT("GameOver Widget Created"));
			Widget->AddToViewport(100);

			PC->SetShowMouseCursor(true);

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Widget->TakeWidget());
			PC->SetInputMode(InputMode);

			//UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
}

void AWaveManager::SetLostArkHUD(ULostArkHUD* HUD)
{
	LostArkHUD = HUD;

	if (LostArkHUD)
	{
		LostArkHUD->Init();
		LostArkHUD->UpdateMonsterCount(CurrentKillCount);
	}
}