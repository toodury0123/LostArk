// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMonster.generated.h"

UCLASS(Abstract)
class LOSTARK_API ABaseMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseMonster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float maxHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float currentHP = maxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float moveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float attackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float attackPoint = 150.0f;

public:
	FORCEINLINE float GetMaxHP() const { return maxHP; }
	FORCEINLINE float GetCurrentHP() const { return currentHP; }
	FORCEINLINE float CurrentMoveSpeed() const { return moveSpeed; }
	FORCEINLINE float GetAttackRange() const { return attackRange; }
	FORCEINLINE float GetAttackPoint() const { return attackPoint; }

protected:
	// Called when the game starts or when spawned
	virtual void InitMonster(int HP);
	virtual void InitMonster(int HP, float speed);
	virtual void InitMonster(int HP, float speed, float point);
	virtual void InitMonster(int HP, float speed, float point, float range);

protected:
	virtual void BeginPlay() override;
	virtual void Attack(ACharacter* player);
	virtual void OnDamaged(ACharacter* player);
	virtual void DieProcess();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	
	// virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
