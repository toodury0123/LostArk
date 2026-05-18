// Copyright Epic Games, Inc. All Rights Reserved.

#include "LostArkGameMode.h"
#include "Player/LostArkCharacter.h"
#include "UObject/ConstructorHelpers.h"

#include "Monster/Crystal.h"

ALostArkGameMode::ALostArkGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
