// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

class UInputMappingContext;
class UInputAction;

#include "LostArkPlayerController.generated.h"

/**
 * 
 */

UCLASS()
class LOSTARK_API ALostArkPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ALostArkPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	FORCEINLINE UInputMappingContext* GetIMC() const { return IMC_LostArk; }

	// Input System
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* IMC_LostArk;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ia_LeftClick;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ia_RightClick;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ia_MouseWheel;

	bool bLeftClickDown = false;
	bool bRightClickDown = false;

	FORCEINLINE bool IsLeftClickDown() const { return bLeftClickDown; }
	FORCEINLINE bool IsRightClickDown() const { return bRightClickDown; }

	void OnLeftClick(const FInputActionValue& InputValue);


public:

	bool GetMouseWorldLocation(FVector& OutLocation);

	void Zoom(const FInputActionValue& inputValue);

	bool IsMonster(AActor* Actor) const;
	void Attack(const struct FInputActionValue& inputValue);
	
	void Move(const struct FInputActionValue& inputValue);
};
