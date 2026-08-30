// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/LostArkHUD.h"
#include "Kismet/GameplayStatics.h"

#include "Level/WaveManager.h"
#include "Components/ProgressBar.h"

void ULostArkHUD::Init()
{
    AWaveManager* WaveManager = Cast<AWaveManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AWaveManager::StaticClass())
    );

    if (WaveManager)
    {
        TotalMonsterCount = WaveManager->GetTotalMonsterCount();
    }


    UE_LOG(LogTemp, Warning,
        TEXT("TotalMonsterCount : %d"),
        TotalMonsterCount); 
}

void ULostArkHUD::UpdateMonsterCount(int32 CurrentMonsterCount)
{
    UE_LOG(LogTemp, Warning,
        TEXT("HUD Update / Current: %d / Total: %d"),
        CurrentMonsterCount,
        TotalMonsterCount);

    if (MonsterCount && TotalMonsterCount > 0)
    {
        const float Percent =
            static_cast<float>(CurrentMonsterCount) /
            static_cast<float>(TotalMonsterCount);

        MonsterCount->SetPercent(Percent);

        UE_LOG(LogTemp, Warning,
            TEXT("Monster Gauge Percent: %f"),
            Percent);
    }
}

void ULostArkHUD::UpdateBombCooldown(float Percent)
{
    if (!Skill_1_CoolDownBar)
    {
        return;
    }

    Skill_1_CoolDownBar->SetPercent(Percent);
}