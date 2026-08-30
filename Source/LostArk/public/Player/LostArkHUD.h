// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"

#include "LostArkHUD.generated.h"

UCLASS()
class LOSTARK_API ULostArkHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init();
	void UpdateMonsterCount(int32 CurrentMonsterCount);
	void UpdateBombCooldown(float Percent);

public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* MonsterCount;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* Skill_1_CoolDownBar;

	UPROPERTY(BlueprintReadOnly, Category = "Chaos")
	int32 TotalMonsterCount = 0;
};