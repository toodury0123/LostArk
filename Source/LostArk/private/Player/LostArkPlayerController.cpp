// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LostArkPlayerController.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "GameFramework/SpringArmComponent.h"

#include "Player/LostArkCharacter.h"

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
	LockMove(5.0f);
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
		EnhancedInput->BindAction(ia_LeftClick, ETriggerEvent::Started, this, &ALostArkPlayerController::OnLeftClick);
		EnhancedInput->BindAction(ia_RightClick, ETriggerEvent::Completed, this, &ALostArkPlayerController::Move);
		EnhancedInput->BindAction(ia_Bomb, ETriggerEvent::Started, this, &ALostArkPlayerController::OnBomb
		);
	}
}

void ALostArkPlayerController::LockMove(float Duration)
{
	bCanMove = false;

	CancelCurrentMove();

	GetWorldTimerManager().SetTimer(
		MoveLockTimerHandle,
		this,
		&ALostArkPlayerController::UnlockMove,
		Duration,
		false
	);
}

void ALostArkPlayerController::UnlockMove()
{
	bCanMove = true;
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

	FHitResult Hit;

	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return;
	}

	if (!Hit.bBlockingHit)
	{
		return;
	}

	ALostArkCharacter* PlayerCharacter = Cast<ALostArkCharacter>(GetCharacter());

	if (!PlayerCharacter)
	{
		return;
	}

	CancelCurrentMove();

	FVector Direction =	Hit.ImpactPoint - PlayerCharacter->GetActorLocation();

	Direction.Z = 0.f;

	if (!Direction.IsNearlyZero())
	{
		PlayerCharacter->SetActorRotation(Direction.Rotation());
	}

	Attack(Hit);
}

void ALostArkPlayerController::Attack(const FHitResult& Hit)
{
	ALostArkCharacter* PlayerCharacter = Cast<ALostArkCharacter>(GetCharacter());

	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->Attack(Hit);
}

void ALostArkPlayerController::Move(const FInputActionValue& InputValue)
{
	if (!bCanMove)
	{
		return;
	}

	float AxisValue = InputValue.Get<float>();

	if (AxisValue <= 0.f)
	{
		return;
	}

	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MOVE HOLD"));
		UE_LOG(LogTemp, Warning, TEXT("Move To: %s"), *Hit.ImpactPoint.ToString());

		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Hit.ImpactPoint);
	}
}

void ALostArkPlayerController::Zoom(const FInputActionValue& inputValue)
{
	float AxisValue = inputValue.Get<float>();

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

void ALostArkPlayerController::OnBomb()
{
	UE_LOG(LogTemp, Warning, TEXT("OnBomb Called"));


	FHitResult Hit;

	GetHitResultUnderCursor(
		ECC_Visibility,
		false,
		Hit
	);

	if (ALostArkCharacter* LostArkCharacter = Cast<ALostArkCharacter>(GetPawn()))
	{
		LostArkCharacter->BombAttack(Hit);
	}
}

void ALostArkPlayerController::CancelCurrentMove()
{
	StopMovement();
}

bool ALostArkPlayerController::IsMonster(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	return Actor->ActorHasTag(TEXT("Monster"));
}

void ALostArkCharacter::SetHP(float HP)
{
	currentHP = HP;

	if (currentHP <= 0.0f)
	{
		DieProccess();
	}
}
