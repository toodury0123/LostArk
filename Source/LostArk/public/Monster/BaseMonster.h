// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player/LostArkCharacter.h"

#include "BaseMonster.generated.h"

UCLASS(Abstract)
class LOSTARK_API ABaseMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseMonster();

// Property

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float maxHP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
	float currentHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float moveSpeed = 300.0f;

// Attack

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float attackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float attackPoint = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	float attackCooldown = 3.0f;

	UPROPERTY()
	ALostArkCharacter* targetPlayer = nullptr;

// Bool
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
	bool bCanAttack = true;

public:

	// Called when the game starts or when spawned
	virtual void InitMonster(int HP, float speed, float point, float range);
	virtual void BeginPlay() override;
	virtual void Attack();
	virtual void OnDamaged(float Damage);
	virtual void DieProcess();

	virtual ALostArkCharacter* FindTarget();
	virtual void MoveToTarget(float DeltaTime);
	virtual bool IsTargetInAttackRange() const;
	virtual void ResetAttack();

public:
	FORCEINLINE float GetMaxHP() const { return maxHP; }
	FORCEINLINE float GetCurrentHP() const { return currentHP; }
	FORCEINLINE float CurrentMoveSpeed() const { return moveSpeed; }
	FORCEINLINE float GetAttackRange() const { return attackRange; }
	FORCEINLINE float GetAttackPoint() const { return attackPoint; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
};
