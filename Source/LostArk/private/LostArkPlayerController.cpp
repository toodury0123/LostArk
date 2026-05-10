// Fill out your copyright notice in the Description page of Project Settings.


#include "LostArkPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "InputAction.h"
#include "InputMappingContext.h"

ALostArkPlayerController::ALostArkPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void ALostArkPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->AddMappingContext(IMC_LostArk, 0);
        }
    }
}

void ALostArkPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(
			ia_RightClick,
			ETriggerEvent::Triggered,
			this,
			&ALostArkPlayerController::Move
		);

		EnhancedInput->BindAction(
			ia_LeftClick,
			ETriggerEvent::Started,
			this,
			&ALostArkPlayerController::OnLeftClick
		);
	}
}

void ALostArkPlayerController::OnLeftClick(const FInputActionValue& InputValue)
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (!Hit.bBlockingHit)
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();

	if (IsMonster(HitActor))
	{
		Attack(InputValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Clicked Actor : %s"),
			HitActor ? *HitActor->GetName() : TEXT("None"));
	}
}

void ALostArkPlayerController::Move(const FInputActionValue& InputValue)
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Hit.ImpactPoint);
	}
}

void ALostArkPlayerController::Attack(const FInputActionValue& InputValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Attack"));
}

bool ALostArkPlayerController::IsMonster(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	return Actor->ActorHasTag(TEXT("Monster"));
}
//void ALostArkPlayerController::OnLeftClick(const FInputActionValue& InputValue)
//{
//    FHitResult Hit;
//
//    GetHitResultUnderCursor(
//        ECC_Visibility,
//        false,
//        Hit
//    );
//
//    if (Hit.bBlockingHit)
//    {
//        AActor* HitActor = Hit.GetActor();
//
//        if (HitActor)
//        {
//            UE_LOG(LogTemp, Warning, TEXT("Hit Actor : %s"), *HitActor->GetName());
//        }
//    }
//}
//
//bool ALostArkPlayerController::isMonster(const FInputActionValue& inputValue)
//{
//    return false;
//}
//
//void ALostArkPlayerController::Move(const FInputActionValue& inputValue)
//{
//}
//
//void ALostArkPlayerController::Attack(const FInputActionValue& inputValue)
//{
//}
