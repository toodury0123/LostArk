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

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Input System
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMC_LostArk;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_LeftClick;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_RightClick;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_MouseWheel;

	void OnLeftClick(const FInputActionValue& InputValue);
	void Zoom(const FInputActionValue& Value);


	FORCEINLINE UInputMappingContext* GetIMC() const { return IMC_LostArk; }

	bool IsMonster(AActor* Actor) const;
	void Move(const struct FInputActionValue& inputValue);
	void Attack(const struct FInputActionValue& inputValue);
};
