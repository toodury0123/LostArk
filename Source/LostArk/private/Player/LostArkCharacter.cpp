// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/LostArkCharacter.h"
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

	if (LostArkHUD && !bCanBomb)
	{
		const float Remaining =
			GetWorldTimerManager().GetTimerRemaining(BombCoolTimeTimerHandle);

		const float Percent =
			FMath::Clamp(Remaining / BombCoolTime, 0.f, 1.f);

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


// ============================================================
// Attack
// ============================================================

void ALostArkCharacter::Attack(const FHitResult& Point)
{
	if (!Point.bBlockingHit)
	{
		return;
	}

	// 이동 경직
	LockMovement(AttackLockDuration);

	const FVector AttackLocation =
		Point.ImpactPoint;

	if (AttackEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AttackEffect,
			AttackLocation,
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
		AttackDamage,
		AttackLocation,
		AttackRadius,
		UDamageType::StaticClass(),
		IgnoreActors,
		this,
		GetController(),
		true
	);

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
	if (!Point.bBlockingHit)
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

	UE_LOG(LogTemp, Error, TEXT("ACTION LOCK TRUE"));


	GetWorldTimerManager().SetTimer(
		BombCoolTimeTimerHandle,
		this,
		&ALostArkCharacter::ResetBombCoolTime,
		BombCoolTime,
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

	FTimerHandle  TestTimer;

	GetWorldTimerManager().SetTimer(
		TestTimer,
		[this]()
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			if (!AnimInstance || !BombMontage)
			{
				return;
			}

			UE_LOG(
				LogTemp,
				Error,
				TEXT(
					"BOMB CHECK / Playing=%d / Position=%f / Length=%f"
				),
				AnimInstance->Montage_IsPlaying(BombMontage),
				AnimInstance->Montage_GetPosition(BombMontage),
				BombMontage->GetPlayLength()
			);
		},
		1.3f,
		false
	);
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
		Duration,
		false
	);
}


void ALostArkCharacter::EnableMovement()
{
	bMovementLocked = false;
	bActionLocked = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()
		->SetMovementMode(
			MOVE_Walking
		);
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