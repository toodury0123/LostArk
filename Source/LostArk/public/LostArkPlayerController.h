// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

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

	void BeginPlay();
	void SetupInputComponent();

	// Input System
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMC_LostArk;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_LeftClick;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_RightClick;

	void OnLeftClick(const FInputActionValue& InputValue);


	bool IsMonster(AActor* Actor) const;
	void Move(const struct FInputActionValue& inputValue);
	void Attack(const struct FInputActionValue& inputValue);
};
