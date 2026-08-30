// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "LostArkCharacter.generated.h"

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
	bool bActionLocked = false;

public:
	bool IsActionLocked() const
	{
		return bActionLocked;
	}
	FTimerHandle MovementLockTimerHandle;

	void LockMovement(float Duration);
	void EnableMovement();
};