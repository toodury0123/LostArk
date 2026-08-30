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
		EnhancedInput->BindAction(ia_LeftClick, ETriggerEvent::Triggered, this, &ALostArkPlayerController::OnLeftClick);
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
	ALostArkCharacter* PlayerCharacter =
		Cast<ALostArkCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	// 스킬 사용 중이면 좌클릭 입력 차단
	if (PlayerCharacter->IsActionLocked())
	{
		UE_LOG(LogTemp, Error, TEXT("LEFT CLICK BLOCKED BY ACTION LOCK"));
		return;
	}
	
	FHitResult Hit;

	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return;
	}

	if (!Hit.bBlockingHit)
	{
		return;
	}

	CancelCurrentMove();

	FVector Direction =
		Hit.ImpactPoint - PlayerCharacter->GetActorLocation();

	Direction.Z = 0.f;

	if (!Direction.IsNearlyZero())
	{
		PlayerCharacter->SetActorRotation(
			Direction.Rotation()
		);
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
	UE_LOG(LogTemp, Error,
		TEXT("MOVE CALLED / VALUE = %f"),
		InputValue.Get<float>());

	ALostArkCharacter* PlayerCharacter =
		Cast<ALostArkCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Error,
		TEXT("Move / MovementLock=%d / ActionLock=%d / CanMove=%d"),
		PlayerCharacter->IsMovementLocked(),
		PlayerCharacter->IsActionLocked(),
		bCanMove
	);

	if (!bCanMove)
	{
		return;
	}

	if (PlayerCharacter->IsMovementLocked())
	{
		return;
	}

	if (PlayerCharacter->IsActionLocked())
	{
		return;
	}

	const float AxisValue = InputValue.Get<float>();

	if (AxisValue <= 0.f)
	{
		return;
	}

	FHitResult Hit;

	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return;
	}

	if (!Hit.bBlockingHit)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Move To: %s"),
		*Hit.ImpactPoint.ToString()
	);

	UAIBlueprintHelperLibrary::SimpleMoveToLocation(
		this,
		Hit.ImpactPoint
	);
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
	UE_LOG(LogTemp, Error, TEXT("BOMB CALLED"));

	ALostArkCharacter* LostArkCharacter =
		Cast<ALostArkCharacter>(GetPawn());

	if (!LostArkCharacter)
	{
		return;
	}

	// 다른 행동 중이면 Bomb 입력 차단
	if (LostArkCharacter->IsActionLocked())
	{
		UE_LOG(LogTemp, Error, TEXT("MOVE BLOCKED - ACTION LOCK"));
		return;
	}

	// Bomb 쿨타임 중이면 1 입력 차단
	if (!LostArkCharacter->CanBomb())
	{
		UE_LOG(LogTemp, Warning, TEXT("BOMB INPUT BLOCKED - COOLDOWN"));
		return;
	}

	FHitResult Hit;

	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return;
	}

	if (!Hit.bBlockingHit)
	{
		return;
	}

	CancelCurrentMove();

	LostArkCharacter->BombAttack(Hit);
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
