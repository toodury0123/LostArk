// Fill out your copyright notice in the Description page of Project Settings.


#include "LostArkPlayerController.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "GameFramework/SpringArmComponent.h"

#include "LostArkCharacter.h"

ALostArkPlayerController::ALostArkPlayerController()
{
	// ViewPort 에서 Mouse 활성화 코드
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ALostArkPlayerController::BeginPlay()
{
    Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	SetShowMouseCursor(true);

	CurrentMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
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
		EnhancedInput->BindAction(ia_RightClick, ETriggerEvent::Triggered, this, &ALostArkPlayerController::Move);
		EnhancedInput->BindAction(ia_MouseWheel, ETriggerEvent::Triggered, this, &ALostArkPlayerController::Zoom);
		// 미구현
		// EnhancedInput->BindAction(ia_LeftClick, ETriggerEvent::Started, this, &ALostArkPlayerController::OnLeftClick);
		// EnhancedInput->BindAction(ia_RightClick, ETriggerEvent::Completed, this, &ALostArkPlayerController::Move);
	}
}

bool ALostArkPlayerController::GetMouseWorldLocation(FVector& OutLocation)
{
	FHitResult Hit;
	bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (!bHit)
	{
		return false;
	}

	OutLocation = Hit.ImpactPoint;
	return true;
}

void ALostArkPlayerController::OnLeftClick(const FInputActionValue& InputValue)
{
	// TEST
	FVector MouseLocation;

	if (!GetMouseWorldLocation(MouseLocation))
	{
		return;
	}

	ALostArkCharacter* PlayerCharacter =
		Cast<ALostArkCharacter>(GetCharacter());

	if (!PlayerCharacter)
	{
		return;
	}

	FVector Direction =
		MouseLocation - PlayerCharacter->GetActorLocation();

	Direction.Z = 0.f;

	if (Direction.IsNearlyZero())
	{
		return;
	}

	PlayerCharacter->SetActorRotation(Direction.Rotation());
	//PlayerCharacter->Attack();
}

void ALostArkPlayerController::Move(const FInputActionValue& InputValue)
{
	float AxisValue = InputValue.Get<float>();

	if (AxisValue <= 0.f)
	{
		return;
	}

	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("MOVE HOLD"));
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Hit.ImpactPoint);
	}
}

void ALostArkCharacter::SetHP(float HP)
{
	currentHP = HP;

	if (currentHP <= 0.0f)
	{
		DieProccess();
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

void ALostArkPlayerController::Zoom(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();

	ALostArkCharacter* PlayerCharacter = Cast<ALostArkCharacter>(GetCharacter());
	
	if (!PlayerCharacter)
	{
		return;
	}

	if (!PlayerCharacter->GetCameraBoom())
	{
		return;
	}

	float newLength =
		PlayerCharacter->GetCameraBoom()->TargetArmLength
		+ (-AxisValue * 100.f);

	newLength = FMath::Clamp(newLength, 500.f, 1200.f);
	PlayerCharacter->SetCameraArmLength(newLength);
}
