// Copyright Epic Games, Inc. All Rights Reserved.

#include "LostArkGameMode.h"
#include "Player/LostArkCharacter.h"
#include "UObject/ConstructorHelpers.h"

#include "Monster/Crystal.h"

ALostArkGameMode::ALostArkGameMode()
{
    UE_LOG(LogTemp, Warning, TEXT("LostArkGameMode 1"));

    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
        TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter")
    );

    UE_LOG(LogTemp, Warning, TEXT("LostArkGameMode 2"));

    if (PlayerPawnBPClass.Succeeded())
    {
        UE_LOG(LogTemp, Warning, TEXT("LostArkGameMode 3"));
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    UE_LOG(LogTemp, Warning, TEXT("LostArkGameMode 4"));
}