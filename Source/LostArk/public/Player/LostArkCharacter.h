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

UCLASS(Blueprintable)
class LOSTARK_API ALostArkCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALostArkCharacter();

	virtual void BeginPlay() override;

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	void SetCameraArmLength(float length);

public:
// Left Click
   void Attack(const FHitResult& Point);
// Right Click
	//void Move(const struct FInputActionValue& inputValue);

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	

private:

	// PlayerInput
	class ALostArkPlayerController* PlayerController;
	class UInputMappingContext* IMC;

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULostArkHUD> LostArkHUDClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	ULostArkHUD* LostArkHUD = nullptr;

public:
	// Minimap
	UPROPERTY(VisibleAnywhere)
	class USceneCaptureComponent2D* MinimapCapture;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	UTextureRenderTarget2D* MinimapRenderTarget;	

	UPROPERTY()
	UUserWidget* MinimapWidget;

public:
	FORCEINLINE float GetAttackPoint() const { return attackPoint; }
	FORCEINLINE float GetCurrentHP() const { return currentHP; }
	FORCEINLINE float GetCurrentMP() const { return currentMP; }
	FORCEINLINE FVector GetCharacterLocation() const { return GetActorLocation(); }

	void SetHP(float HP);
	void SetMP(float MP);

private:
	float maxHP;
	float maxMP;
	float currentHP;
	float currentMP;
	float attackPoint;

	void DieProccess();


public:
	/***  Attack  ***/
	UPROPERTY(EditAnywhere, Category = "Attack")
	UNiagaraSystem* AttackEffect;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 30.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRadius = 200.f;

public:
	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage1;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage2;

	int32 AttackCount = 0;
};

	