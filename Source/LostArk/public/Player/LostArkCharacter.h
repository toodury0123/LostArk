// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "LostArkCharacter.generated.h"

class UCombatComponent;
class ALostArkPlayerController;
class UNiagaraSystem;
class UUserWidget;
class ULostArkHUD;
class UAnimMontage;
class UInputMappingContext;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;
class UCameraComponent;
class USpringArmComponent;

UCLASS(Blueprintable)
class LOSTARK_API ALostArkCharacter : public ACharacter

{	
	GENERATED_BODY()

public:
	ALostArkCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	FORCEINLINE UCameraComponent* GetTopDownCameraComponent() const
	{
		return TopDownCameraComponent;
	}

	FORCEINLINE USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}

	void SetCameraArmLength(float Length);

public:
	// Left Click
	void Attack(const FHitResult& Point);

	// Press 1
	void BombAttack(const FHitResult& Point);

	// 마우스 방향으로 슬라이딩하며 장애물과 이동 가능 영역 경계에서 멈춘다.
	UFUNCTION(BlueprintCallable, Category = "Slide")
	void Slide();

	// 장애물이 없을 때 이동할 거리(cm).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0.0", Units = "cm"))
	float SlideDistance = 500.f;

	// 슬라이딩 이동에 걸리는 시간(초).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0.01", Units = "s"))
	float SlideDuration = 0.4f;

	// 이동은 코드가 담당하므로 루트 이동이 없는 슬라이딩 Montage를 지정한다.
	UPROPERTY(EditAnywhere, Category = "Slide")
	TObjectPtr<UAnimMontage> SlideMontage = nullptr;

	// Bomb 사용 가능 여부
	bool CanBomb() const
	{
		return bCanBomb;
	}

	// 공격/스킬 경직 중인지
	bool IsMovementLocked() const
	{
		return bMovementLocked;
	}


private:
	/*** Camera ***/

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = Camera,
		meta = (AllowPrivateAccess = "true")
	)
	UCameraComponent* TopDownCameraComponent;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = Camera,
		meta = (AllowPrivateAccess = "true")
	)
	USpringArmComponent* CameraBoom;


private:
	/*** Player Input ***/

	ALostArkPlayerController* PlayerController = nullptr;
	UInputMappingContext* IMC = nullptr;


public:
	/*** UI ***/

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULostArkHUD> LostArkHUDClass;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "UI",
		meta = (AllowPrivateAccess = "true")
	)
	ULostArkHUD* LostArkHUD = nullptr;


public:
	/*** Minimap ***/

	UPROPERTY(VisibleAnywhere)
	USceneCaptureComponent2D* MinimapCapture;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	UTextureRenderTarget2D* MinimapRenderTarget;

	UPROPERTY()
	UUserWidget* MinimapWidget;


public:
	/*** Status ***/

	FORCEINLINE float GetAttackPoint() const
	{
		return attackPoint;
	}

	FORCEINLINE float GetCurrentHP() const
	{
		return currentHP;
	}

	FORCEINLINE float GetCurrentMP() const
	{
		return currentMP;
	}

	FORCEINLINE FVector GetCharacterLocation() const
	{
		return GetActorLocation();
	}

	void SetHP(float HP);
	void SetMP(float MP);


private:
	float maxHP = 0.f;
	float maxMP = 0.f;

	float currentHP = 0.f;
	float currentMP = 0.f;

	float attackPoint = 0.f;

	void DieProccess();


public:
	// Keep attack tuning on the character for existing Blueprint serialization.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	UCombatComponent* CombatComponent;

	/*** Attack ***/

	UPROPERTY(EditAnywhere, Category = "Attack")
	UNiagaraSystem* AttackEffect;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 30.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage1;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage2;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackLockDuration = 0.4f;

	int32 AttackCount = 0;


public:
	/*** Bomb ***/

	UPROPERTY(EditAnywhere, Category = "Bomb")
	UNiagaraSystem* BombEffect;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float BombDamage = 100.f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float BombRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float BombLockDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float BombCoolTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	UAnimMontage* BombMontage;

	// AnimNotify
	UFUNCTION(BlueprintCallable)
	void BombHit();


private:
	FVector BombLocation = FVector::ZeroVector;

	bool bCanBomb = true;
	bool bBombAttackActive = false;

	FTimerHandle BombCoolTimeTimerHandle;

	void ResetBombCoolTime();


private:
	bool bMovementLocked = false;
	// 진행 중 설정값이 바뀌어도 시작 시 정한 거리와 시간을 유지한다.
	bool bSliding = false;
	FVector SlideDirection = FVector::ZeroVector;
	// 슬라이딩 중 직선 경로가 NavMesh 밖으로 나가는지 확인할 기준점.
	FVector SlideNavLocation = FVector::ZeroVector;
	float SlideRemainingDistance = 0.f;
	float SlideRemainingTime = 0.f;
	float SlideSpeed = 0.f;
	bool bActionLocked = false;
	FTimerHandle ActionLockTimerHandle;
	void UnlockAction();

public:
	bool IsActionLocked() const
	{
		return bActionLocked;
	}
	FTimerHandle MovementLockTimerHandle;

	void LockMovement(float Duration);
	void EnableMovement();
};
