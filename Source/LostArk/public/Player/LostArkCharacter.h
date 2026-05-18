// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player/LostArkPlayerController.h"
#include "InputActionValue.h"

#include "LostArkCharacter.generated.h"


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
	void Attack(FHitResult point);
// Right Click
	//void Move(const struct FInputActionValue& inputValue);

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	

	// PlayerInput
	class ALostArkPlayerController* PlayerController;
	class UInputMappingContext* IMC;

public:
	FORCEINLINE float GetAttackPoint() const { return attackPoint; }
	FORCEINLINE float GetCurrentHP() const { return currentHP; }
	FORCEINLINE FVector GetCharacterLocation() const { return GetActorLocation(); }

	void SetHP(float HP);

private:
	float maxHP;
	float currentHP;
	float attackPoint;

	void DieProccess();
};

