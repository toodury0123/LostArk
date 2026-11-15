// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LostArkPlayerController.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"	

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include "GameFramework/SpringArmComponent.h"

#include "Player/LostArkCharacter.h"

ALostArkPlayerController::ALostArkPlayerController()
{
	// ViewPort ¿¡¼­ Mouse È°¼ºÈ­ ÄÚµå
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
	// ì—ë””í„°ì—ì„œ ì •í•œ ì‹œì‘ ëŒ€ê¸° ì‹œê°„ ë™ì•ˆ ì´ë™ê³¼ ìŠ¬ë¼ì´ë”©ì„ ì ê·¼ë‹¤.
	LockMove(StartDelay);
}

void ALostArkPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// ½ºÆäÀÌ½º¹Ù·Î Àü¹æ ½½¶óÀÌµùÀ» ½ÇÇàÇÑ´Ù.
	if (IsValid(InputComponent))
	{
		InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ALostArkPlayerController::OnSlide);
	}

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

// ìœ íš¨í•œ ì–‘ìˆ˜ ì‹œê°„ì—ë§Œ íƒ€ì´ë¨¸ë¥¼ ë“±ë¡í•´ 0ì´ˆ ì„¤ì •ì—ì„œë„ ì ê¸ˆì´ ë‚¨ì§€ ì•Šê²Œ í•œë‹¤.
void ALostArkPlayerController::LockMove(float Duration)
{
	GetWorldTimerManager().ClearTimer(MoveLockTimerHandle);
	bCanMove = !FMath::IsFinite(Duration) || Duration <= 0.f;
	if (bCanMove) { return; }
	CancelCurrentMove();
	GetWorldTimerManager().SetTimer(MoveLockTimerHandle, this,
		&ALostArkPlayerController::UnlockMove, Duration, false);
}

void ALostArkPlayerController::UnlockMove()
{
	bCanMove = true;
}

void ALostArkPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(MoveLockTimerHandle);
	Super::EndPlay(EndPlayReason);
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

	// ½ºÅ³ »ç¿ë ÁßÀÌ¸é ÁÂÅ¬¸¯ ÀÔ·Â Â÷´Ü
	if (PlayerCharacter->IsActionLocked() || PlayerCharacter->IsMovementLocked())
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
	if (!bCanMove) { return; }
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
		TEXT("Move / MovementLock=%d / ActionLock=%d"),
		PlayerCharacter->IsMovementLocked(),
		PlayerCharacter->IsActionLocked()
	);

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

	if (MoveClickEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			MoveClickEffect,
			Hit.ImpactPoint
		);
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

	// ´Ù¸¥ Çàµ¿ ÁßÀÌ¸é Bomb ÀÔ·Â Â÷´Ü
	if (LostArkCharacter->IsActionLocked())
	{
		UE_LOG(LogTemp, Error, TEXT("MOVE BLOCKED - ACTION LOCK"));
		return;
	}

	// Bomb ÄğÅ¸ÀÓ ÁßÀÌ¸é 1 ÀÔ·Â Â÷´Ü
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

// ½ÃÀÛ ½Ã ÀÌµ¿ Àá±İÀ» À¯ÁöÇÏ¸ç ½½¶óÀÌµùÀ» ¿äÃ»ÇÑ´Ù.
void ALostArkPlayerController::OnSlide()
{
	if (!bCanMove) { return; }
	if (ALostArkCharacter* SlidingCharacter = Cast<ALostArkCharacter>(GetPawn()); IsValid(SlidingCharacter))
	{
		SlidingCharacter->Slide();
	}
}
