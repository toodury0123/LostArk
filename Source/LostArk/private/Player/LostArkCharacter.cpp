// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/LostArkCharacter.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Particles/ParticleSystem.h"
#include "Combat/CombatComponent.h"
#include "Combat/BombExplosionCloud.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "GameFramework/DamageType.h"
#include "Player/LostArkPlayerController.h"

#include "UObject/ConstructorHelpers.h"

#include "Camera/CameraComponent.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ProgressBar.h"

#include "Engine/TextureRenderTarget2D.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

#include "Player/LostArkHUD.h"
#include "Level/WaveManager.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
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

	// 장식용 총은 오른손 애니메이션만 따라가며 이동/공격/내비게이션 판정에 참여하지 않는다.
	GunProp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunProp"));
	GunProp->SetupAttachment(GetMesh(), TEXT("hand_r"));
	GunProp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunProp->SetGenerateOverlapEvents(false);
	GunProp->SetCanEverAffectNavigation(false);
	GunProp->SetRelativeScale3D(FVector(0.55f));

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

	// 시작 위치가 바닥보다 높아도 중력을 끄기 전에 실제 캡슐 충돌로 접지를 확인한다.
	if (IsValid(StartupAnimation) && IsValid(GetMesh()->GetAnimInstance()))
	{
		UCharacterMovementComponent* Movement = GetCharacterMovement();
		UCapsuleComponent* Capsule = GetCapsuleComponent();
		if (IsValid(Movement) && IsValid(Capsule))
		{
			FFindFloorResult Floor;
			const float SearchDistance = Capsule->GetScaledCapsuleHalfHeight() * 2.f + Movement->MaxStepHeight;
			Movement->ComputeFloorDist(GetActorLocation(), SearchDistance, SearchDistance,
				Floor, Capsule->GetScaledCapsuleRadius());
			if (Floor.IsWalkableFloor() && Floor.GetDistanceToFloor() >= 0.f)
			{
				// X/Y와 PlayerStart는 유지하고 기존 CharacterMovement의 바닥 여유 간격으로 맞춘다.
				const float FloorGap = (UCharacterMovementComponent::MIN_FLOOR_DIST +
					UCharacterMovementComponent::MAX_FLOOR_DIST) * 0.5f;
				FHitResult GroundHit;
				SetActorLocation(GetActorLocation() - FVector(0.f, 0.f,
					FMath::Max(0.f, Floor.GetDistanceToFloor() - FloorGap)), true, &GroundHit);
				// 이동 경로에 다른 충돌이 있으면 공중에서 고정하지 않고 중력으로 정착시킨다.
				Movement->ComputeFloorDist(GetActorLocation(), SearchDistance, SearchDistance,
					Floor, Capsule->GetScaledCapsuleRadius());
			}
			if (Floor.IsWalkableFloor() && Floor.GetDistanceToFloor() <= UCharacterMovementComponent::MAX_FLOOR_DIST)
			{
				LockMovement(StartupAnimation->GetPlayLength());
			}
			else
			{
				// 바닥이 없는 시작점에서는 입력만 잠그고 중력·낙하 처리를 유지한다.
				bMovementLocked = true;
				if (IsValid(GetController())) { GetController()->StopMovement(); }
				GetWorldTimerManager().SetTimer(MovementLockTimerHandle, this,
					&ALostArkCharacter::EnableMovement, StartupAnimation->GetPlayLength(), false);
			}
		}
	}

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

	// 반동 중에만 기존 Tick에서 캡슐 경로·바닥을 검사하고 작은 점프 자세를 갱신한다.
	if (bBombRecoiling && DeltaSeconds > 0.f) { UpdateBombRecoil(DeltaSeconds); }

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
	if (!Point.bBlockingHit || bActionLocked || bMovementLocked || bBombAttackActive)
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

	// 기존 AttackBomb 알림 시점을 유지하되, 다른 스켈레톤/애님 클래스에서도 피해는 보장한다.
	float DamageDelay = 0.f;
	if (IsValid(BombMontage))
	{
		for (const FAnimNotifyEvent& Notify : BombMontage->Notifies)
		{
			if (Notify.NotifyName == TEXT("AttackBomb"))
			{
				DamageDelay = FMath::IsFinite(Notify.GetTriggerTime())
					? FMath::Max(0.f, Notify.GetTriggerTime()) : 0.f;
				break;
			}
		}
	}
	if (DamageDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(BombDamageTimerHandle, this,
			&ALostArkCharacter::BombHit, DamageDelay, false);
	}
	else
	{
		BombDamageTimerHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ALostArkCharacter::BombHit);
	}

	// 연출은 클릭 지점에 즉시 표시한다. 기존 BombHit의 데미지/Notify 시점은 유지한다.
	const ABombExplosionCloud* CloudDefaults = BombCloudEffectClass
		? BombCloudEffectClass->GetDefaultObject<ABombExplosionCloud>() : nullptr;
	if (bEnableBombExplosionCloud && IsValid(CloudDefaults) && IsValid(CloudDefaults->CloudSystem) && CloudDefaults->CloudMaterial != nullptr)
	{
		const FTransform EffectTransform(FRotator::ZeroRotator, BombLocation + FVector(0.f, 0.f, 2.f));
		ABombExplosionCloud* Explosion = GetWorld()->SpawnActorDeferred<ABombExplosionCloud>(
			BombCloudEffectClass, EffectTransform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (IsValid(Explosion))
		{
			Explosion->InitializeExplosion(BombExplosionCloudRange, BombEffectDuration, BombOuterColor, BombInnerColor);
			UGameplayStatics::FinishSpawningActor(Explosion, EffectTransform);
		}
	}

	LockMovement(BombLockDuration);

	FVector Direction =
		BombLocation - GetActorLocation();

	Direction.Z = 0.f;

	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
	StartBombRecoil();

	// 기존 Blueprint의 모션·Notify·데미지 시점과 독립적으로 실제 사용에만 총구 효과를 표시한다.
	if (IsValid(BombMuzzleEffect) && IsValid(GunProp) && IsValid(GunProp->GetStaticMesh()))
	{
		UGameplayStatics::SpawnEmitterAttached(BombMuzzleEffect, GunProp, NAME_None,
			BombMuzzleTransform.GetLocation(), BombMuzzleTransform.Rotator(), BombMuzzleTransform.GetScale3D(),
			EAttachLocation::KeepRelativeOffset, true);
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


void ALostArkCharacter::StartBombRecoil()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!IsValid(Movement) || !IsValid(Capsule) || !IsValid(GetMesh()) ||
		!FMath::IsFinite(backwardDistance) || backwardDistance <= 0.f) { return; }

	// 이동 잠금으로 MovementMode가 None이어도 실제 바닥에 서 있는지 직접 확인한다.
	FFindFloorResult Floor;
	Movement->ComputeFloorDist(GetActorLocation(), Movement->MaxStepHeight + 5.f,
		Movement->MaxStepHeight + 5.f, Floor, Capsule->GetScaledCapsuleRadius());
	if (!Floor.IsWalkableFloor() || Floor.GetDistanceToFloor() < 0.f ||
		Floor.GetDistanceToFloor() > UCharacterMovementComponent::MAX_FLOOR_DIST) { return; }

	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation StartNav;
	const FVector Feet = GetActorLocation() - FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight());
	if (!IsValid(Navigation) || !Navigation->ProjectPointToNavigation(Feet, StartNav,
		FVector(10.f, 10.f, Movement->MaxStepHeight + 10.f), &Movement->GetNavAgentPropertiesRef())) { return; }

	BombRecoilDirection = -GetActorForwardVector().GetSafeNormal2D();
	BombRecoilNavLocation = StartNav.Location;
	BombRecoilMeshLocation = GetMesh()->GetRelativeLocation();
	BombRecoilDistance = backwardDistance;
	BombRecoilElapsed = 0.f;
	bBombRecoiling = true;
}


void ALostArkCharacter::UpdateBombRecoil(float DeltaSeconds)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!IsValid(Movement) || !IsValid(Capsule) || !IsValid(GetMesh())) { StopBombRecoil(); return; }

	constexpr float Duration = BombRecoilDuration;
	constexpr float HopHeight = 8.f;
	const float OldAlpha = FMath::Clamp(BombRecoilElapsed / Duration, 0.f, 1.f);
	BombRecoilElapsed = FMath::Min(BombRecoilElapsed + DeltaSeconds, Duration);
	const float Alpha = BombRecoilElapsed / Duration;
	// 프레임률과 관계없이 지정한 거리에서 부드럽게 출발·정지한다.
	const auto Ease = [](float T) { return T * T * (3.f - 2.f * T); };
	const float StepDistance = BombRecoilDistance * (Ease(Alpha) - Ease(OldAlpha));
	const FVector NavEnd = BombRecoilNavLocation + BombRecoilDirection * StepDistance;
	FVector NavHit = BombRecoilNavLocation;
	const bool bNavBlocked = UNavigationSystemV1::NavigationRaycast(
		this, BombRecoilNavLocation, NavEnd, NavHit, nullptr, GetController());
	const float SafeDistance = bNavBlocked
		? FMath::Clamp(FVector::Dist2D(BombRecoilNavLocation, NavHit) - 2.f, 0.f, StepDistance)
		: StepDistance;

	const FVector Before = GetActorLocation();
	FVector Proposed = Before + BombRecoilDirection * SafeDistance;
	FFindFloorResult Floor;
	Movement->ComputeFloorDist(Proposed, Movement->MaxStepHeight + 5.f,
		Movement->MaxStepHeight + 5.f, Floor, Capsule->GetScaledCapsuleRadius());
	if (!Floor.IsWalkableFloor() || Floor.GetDistanceToFloor() < 0.f) { StopBombRecoil(); return; }
	const float FloorGap = (UCharacterMovementComponent::MIN_FLOOR_DIST +
		UCharacterMovementComponent::MAX_FLOOR_DIST) * 0.5f;
	Proposed.Z += FloorGap - Floor.GetDistanceToFloor();
	FHitResult Hit;
	AddActorWorldOffset(Proposed - Before, true, &Hit);
	BombRecoilNavLocation += GetActorLocation() - Before;
	if (bNavBlocked || Hit.bBlockingHit || Alpha >= 1.f) { StopBombRecoil(); return; }

	// 캡슐은 접지 상태를 유지하고 메시만 올려 충돌·카메라·미니맵의 높이는 바꾸지 않는다.
	GetMesh()->SetRelativeLocation(BombRecoilMeshLocation + FVector(0.f, 0.f, HopHeight * FMath::Sin(PI * Alpha)));
}


void ALostArkCharacter::StopBombRecoil()
{
	if (bBombRecoiling && IsValid(GetMesh())) { GetMesh()->SetRelativeLocation(BombRecoilMeshLocation); }
	bBombRecoiling = false;
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
	// 기존 Blueprint 알림이 먼저 호출해도 예약된 피해와 중복 적용하지 않는다.
	GetWorldTimerManager().ClearTimer(BombDamageTimerHandle);

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
	// 다른 이동 잠금이 시작되면 진행 중인 반동과 메시 높이를 정리한다.
	StopBombRecoil();
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
	StopBombRecoil();
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
	StopBombRecoil();
	GetWorldTimerManager().ClearTimer(MovementLockTimerHandle);
	GetWorldTimerManager().ClearTimer(ActionLockTimerHandle);
	GetWorldTimerManager().ClearTimer(BombCoolTimeTimerHandle);
	GetWorldTimerManager().ClearTimer(BombDamageTimerHandle);
	Super::EndPlay(EndPlayReason);
}


// ============================================================
// Status
// ============================================================

void ALostArkCharacter::SetHP(float HP)
{
	// 체력 계산은 유지하고, 감소 시 가능한 경우에만 피격 연출을 추가한다.
	const bool bShouldReact = HP < currentHP && HP > 0.f;
	currentHP = HP;

	if (currentHP <= 0.f)
	{
		DieProccess();
	}
	else if (bShouldReact && !bMovementLocked && !bActionLocked && IsValid(HitMontage))
	{
		PlayAnimMontage(HitMontage);
	}
}

void ALostArkCharacter::PlayFallDownMotion()
{
	// 사망/넉다운 규칙을 새로 만들지 않고 요청된 모션만 외부에서 호출할 수 있다.
	if (bMovementLocked || bActionLocked || !IsValid(FallDownMontage)) { return; }
	const float Duration = PlayAnimMontage(FallDownMontage);
	if (Duration > 0.f) { LockMovement(Duration); }
}


void ALostArkCharacter::SetMP(float MP)
{
	currentMP = MP;
}


void ALostArkCharacter::DieProccess()
{}
