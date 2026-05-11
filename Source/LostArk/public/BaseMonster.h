// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMonster.generated.h"

UCLASS()
class LOSTARK_API ABaseMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseMonster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float maxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float currentHP = maxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float AttackRange = 150.0f;

protected:
	// Called when the game starts or when spawned
	virtual void InitMonster();	

	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
