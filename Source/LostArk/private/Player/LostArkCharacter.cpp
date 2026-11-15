// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/LostArkCharacter.h"
#include "Combat/CombatComponent.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "GameFramework/DamageType.h"
#include "Player/LostArkPlayerController.h"

#include "UObject/ConstructorHelpers.h"

#include "Camera/CameraComponent.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/CapsuleComponent.h"
#include "Components/ProgressBar.h"

#include "Engine/TextureRenderTarget2D.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

#include "Player/LostArkHUD.h"
#include "Level/WaveManager.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include "Kismet/GameplayStatics.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"


ALostArkCharacter::ALostArkCharacter()
{
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	ConstructorHelpers::FObjectFinder<USkeletalMesh>
		TempMesh(
			TEXT("SkeletalMesh'/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin.SK_Mannequin'")
		);

	if (TempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempMesh.Object);

		GetMesh()->SetRelativeLocationAndRotation(
			FVector(0.f, 0.f, -90.f),
			FRotator(0.f, -90.f, 0.f)
		);
	}

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Camera Boom
	CameraBoom =
		CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 1000.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Camera
	TopDownCameraComponent =
		CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	TopDownCameraComponent->SetupAttachment(
		CameraBoom,
		USpringArmComponent::SocketName
	);

	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Minimap
	MinimapCapture =
		CreateDefaultSubobject<USceneCaptureComponent2D>(
			TEXT("MinimapCapture")
		);

	MinimapCapture->SetupAttachment(RootComponent);

	MinimapCapture->SetRelativeLocation(
		FVector(0.f, 0.f, 1500.f)
	);

	MinimapCapture->SetRelativeRotation(
		FRotator(-90.f, 0.f, 0.f)
	);

	MinimapCapture->ProjectionType =
		ECameraProjectionMode::Orthographic;

	MinimapCapture->OrthoWidth = 500.f;
	MinimapCapture->ShowFlags.SetFog(false);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}


void ALostArkCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController =
		Cast<ALostArkPlayerController>(GetController());

	if (!PlayerController)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("LostArkPlayerController is null")
		);

		return;
	}

	IMC = PlayerController->GetIMC();

	if (!IMC)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("IMC_LostArk is null")
		);

		return;
	}

	if (LostArkHUDClass)
	{
		LostArkHUD = CreateWidget<ULostArkHUD>(
			GetWorld(),
			LostArkHUDClass
		);

		if (LostArkHUD)
		{
			LostArkHUD->AddToViewport();

			AWaveManager* WaveManager =
				Cast<AWaveManager>(
					UGameplayStatics::GetActorOfClass(
						GetWorld(),
						AWaveManager::StaticClass()
					)
				);

			if (WaveManager)
			{
				WaveManager->SetLostArkHUD(LostArkHUD);
			}
		}
	}
}


void ALostArkCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 부드러운 슬라이딩과 매 프레임 캡슐 충돌 검사를 위해 기존 Tick을 사용한다.
	if (bSliding && DeltaSeconds > 0.f)
	{
		const float StepTime = FMath::Min(DeltaSeconds, SlideRemainingTime);
		const float StepDistance = FMath::Min(SlideSpeed * StepTime, SlideRemainingDistance);
		// NavMesh 경계는 물리 충돌이 없어도 멈추고, 벽·액터는 기존 캡슐 Sweep으로 막는다.
		FVector NavEnd = SlideNavLocation + SlideDirection * StepDistance;
		FVector NavHit = SlideNavLocation;
		const bool bNavBlocked = UNavigationSystemV1::NavigationRaycast(
			this, SlideNavLocation, NavEnd, NavHit, nullptr, GetController());
		const float SafeStepDistance = bNavBlocked
			? FMath::Clamp(FVector::Dist2D(SlideNavLocation, NavHit) - 2.f, 0.f, StepDistance)
			: StepDistance;
		FHitResult Hit;
		AddActorWorldOffset(SlideDirection * SafeStepDistance, true, &Hit);
		SlideNavLocation += SlideDirection * SafeStepDistance;
		SlideRemainingTime -= StepTime;
		SlideRemainingDistance -= StepDistance;
		if (bNavBlocked || Hit.bBlockingHit || SlideRemainingTime <= KINDA_SMALL_NUMBER || SlideRemainingDistance <= KINDA_SMALL_NUMBER)
		{
			EnableMovement();
		}
	}

	if (LostArkHUD && !bCanBomb)
	{
		const float Remaining =
			GetWorldTimerManager().GetTimerRemaining(BombCoolTimeTimerHandle);

		const float Percent =
			(FMath::IsFinite(BombCoolTime) && BombCoolTime > 0.f)
			? FMath::Clamp(Remaining / BombCoolTime, 0.f, 1.f) : 0.f;

		LostArkHUD->UpdateBombCooldown(Percent);
	}
}


void ALostArkCharacter::SetCameraArmLength(float Length)
{
	if (!CameraBoom)
	{
		return;
	}

	CameraBoom->TargetArmLength =
		FMath::Clamp(Length, 300.f, 1200.f);
}

void ALostArkCharacter::Slide()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (bSliding || bActionLocked || bMovementLocked || !IsValid(Movement) ||
		!Movement->IsMovingOnGround() || !FMath::IsFinite(SlideDistance) ||
		!FMath::IsFinite(SlideDuration) || SlideDistance <= 0.f || SlideDuration < 0.01f)
	{
		return;
	}

	// 입력 순간의 마우스 위치를 고정하며 포인터가 가까우면 그 위치까지만 이동한다.
	APlayerController* SlideController = Cast<APlayerController>(GetController());
	FHitResult CursorHit;
	if (!IsValid(SlideController) ||
		!SlideController->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit) ||
		!CursorHit.bBlockingHit)
	{
		return;
	}
	const FVector ToCursor = CursorHit.ImpactPoint - GetActorLocation();
	SlideDirection = ToCursor.GetSafeNormal2D();
	if (SlideDirection.IsNearlyZero()) { return; }

	// 캐릭터 발밑이 이동 가능한 영역인지 확인하고 직선 경계를 따라 검사한다.
	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation StartNav;
	const FVector FeetLocation = GetActorLocation() -
		FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	if (!IsValid(Navigation) || !Navigation->ProjectPointToNavigation(FeetLocation, StartNav,
		FVector(10.f, 10.f, Movement->MaxStepHeight + 10.f), &Movement->GetNavAgentPropertiesRef()))
	{
		return;
	}
	SlideNavLocation = StartNav.Location;
	SlideRemainingDistance = FMath::Min(SlideDistance, ToCursor.Size2D());
	SetActorRotation(SlideDirection.Rotation());
	SlideRemainingTime = SlideDuration;
	SlideSpeed = SlideRemainingDistance / SlideDuration;
	if (AController* MyController = GetController(); IsValid(MyController))
	{
		MyController->StopMovement();
	}
	// 기존 입력의 이동 잠금 검사를 재사용하고 종료 시 보행을 복구한다.
	Movement->StopMovementImmediately();
	Movement->DisableMovement();
	bMovementLocked = true;
	bSliding = true;
	if (IsValid(SlideMontage))
	{
		PlayAnimMontage(SlideMontage);
	}
}


// ============================================================
// Attack
// ============================================================

void ALostArkCharacter::Attack(const FHitResult& Point)
{
	if (!Point.bBlockingHit || bActionLocked || bMovementLocked || !IsValid(CombatComponent))
	{
		return;
	}

	// 이동 경직
	LockMovement(AttackLockDuration);

	CombatComponent->ExecuteBasicAttack(Point.ImpactPoint, AttackDamage, AttackRadius, AttackEffect);

	UAnimMontage* MontageToPlay = nullptr;

	if (AttackCount == 0)
	{
		MontageToPlay = AttackMontage1;
		AttackCount = 1;
	}
	else
	{
		MontageToPlay = AttackMontage2;
		AttackCount = 0;
	}

	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}
}


// ============================================================
// Bomb
// ============================================================

void ALostArkCharacter::BombAttack(const FHitResult& Point)
{
	if (!Point.bBlockingHit || bActionLocked || bMovementLocked)
	{
		return;
	}

	if (!bCanBomb)
	{
		return;
	}

	bCanBomb = false;
	bBombAttackActive = true;


	UE_LOG(
		LogTemp,
		Error,
		TEXT("BOMB START / CoolTime=%f / LockDuration=%f / CanBomb=%d"),
		BombCoolTime,
		BombLockDuration,
		bCanBomb
	);

	// Bomb 사용 중 모든 행동 입력 잠금
	bActionLocked = true;
	// Movement may be released separately; Bomb owns its own action deadline.
	const float ActionDuration = FMath::IsFinite(BombLockDuration)
		? FMath::Max(BombLockDuration, KINDA_SMALL_NUMBER) : KINDA_SMALL_NUMBER;
	GetWorldTimerManager().SetTimer(ActionLockTimerHandle, this,
		&ALostArkCharacter::UnlockAction, ActionDuration, false);

	UE_LOG(LogTemp, Error, TEXT("ACTION LOCK TRUE"));


	GetWorldTimerManager().SetTimer(
		BombCoolTimeTimerHandle,
		this,
		&ALostArkCharacter::ResetBombCoolTime,
		FMath::IsFinite(BombCoolTime) ? FMath::Max(BombCoolTime, KINDA_SMALL_NUMBER) : KINDA_SMALL_NUMBER,
		false
	);

	BombLocation = Point.ImpactPoint;

	LockMovement(BombLockDuration);

	FVector Direction =
		BombLocation - GetActorLocation();

	Direction.Z = 0.f;

	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}

	// Bomb montage playback remains owned by the existing Blueprint/AnimBP.

}


void ALostArkCharacter::ResetBombCoolTime()
{
	bCanBomb = true;

	if (LostArkHUD)
	{
		LostArkHUD->UpdateBombCooldown(0.0f);
	}

	UE_LOG(LogTemp, Warning, TEXT("Bomb Ready"));
}


// ============================================================
// Bomb Notify
// ============================================================

void ALostArkCharacter::BombHit()
{
	// 정상적으로 시작된 Bomb만 공격 처리
	if (!bBombAttackActive)
	{
		return;
	}

	// 한 번만 공격 가능
	bBombAttackActive = false;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Bomb Hit")
	);

	//DrawDebugSphere(
	//	GetWorld(),
	//	BombLocation,
	//	BombRadius,
	//	32,
	//	FColor::Red,
	//	false,
	//	2.f
	//);

	if (BombEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			BombEffect,
			BombLocation,
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			true,
			ENCPoolMethod::None,
			true
		);
	}

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),
		BombDamage,
		BombLocation,
		BombRadius,
		UDamageType::StaticClass(),
		IgnoreActors,
		this,
		GetController(),
		true
	);
}


// ============================================================
// Movement Lock
// ============================================================

void ALostArkCharacter::LockMovement(
	float Duration
)
{
	// 외부 경직이 발생하면 슬라이딩을 중단하고 해당 경직을 따른다.
	if (bSliding)
	{
		bSliding = false;
		if (IsValid(SlideMontage)) { StopAnimMontage(SlideMontage); }
	}
	const float SafeDuration = FMath::IsFinite(Duration)
		? FMath::Max(Duration, KINDA_SMALL_NUMBER) : KINDA_SMALL_NUMBER;
	// A repeated lock must not shorten an existing longer lock.
	const float LockDuration = FMath::Max(SafeDuration,
		GetWorldTimerManager().GetTimerRemaining(MovementLockTimerHandle));
	bMovementLocked = true;

	// 기존 SimpleMove 취소
	if (AController* MyController =
		GetController())
	{
		MyController->StopMovement();
	}

	// 현재 속도 제거
	GetCharacterMovement()
		->StopMovementImmediately();

	// CharacterMovement 자체 잠금
	GetCharacterMovement()
		->DisableMovement();

	UE_LOG(LogTemp, Error, TEXT("MovementMode = %d"),
		(int32)GetCharacterMovement()->MovementMode);

	// 이전 경직 타이머 제거
	GetWorldTimerManager()
		.ClearTimer(
			MovementLockTimerHandle
		);

	// 새 경직 시간 적용
	GetWorldTimerManager().SetTimer(
		MovementLockTimerHandle,
		this,
		&ALostArkCharacter::EnableMovement,
		LockDuration,
		false
	);
}


void ALostArkCharacter::EnableMovement()
{
	// 벽에 닿거나 이동 시간이 끝나면 슬라이딩 자세에서도 복귀한다.
	if (bSliding && IsValid(SlideMontage)) { StopAnimMontage(SlideMontage); }
	bSliding = false;
	GetWorldTimerManager().ClearTimer(MovementLockTimerHandle);
	bMovementLocked = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void ALostArkCharacter::UnlockAction()
{
	bActionLocked = false;
}

void ALostArkCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(MovementLockTimerHandle);
	GetWorldTimerManager().ClearTimer(ActionLockTimerHandle);
	GetWorldTimerManager().ClearTimer(BombCoolTimeTimerHandle);
	Super::EndPlay(EndPlayReason);
}


// ============================================================
// Status
// ============================================================

void ALostArkCharacter::SetHP(float HP)
{
	currentHP = HP;

	if (currentHP <= 0.f)
	{
		DieProccess();
	}
}


void ALostArkCharacter::SetMP(float MP)
{
	currentMP = MP;
}


void ALostArkCharacter::DieProccess()
{}
