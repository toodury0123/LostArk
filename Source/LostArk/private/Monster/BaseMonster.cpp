// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ABaseMonster::ABaseMonster()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseMonster::BeginPlay()
{
	Super::BeginPlay();
	
}

// Init
void ABaseMonster::InitMonster(int HP, float speed, float point, float range)
{
	maxHP = HP;
	currentHP = maxHP;

	moveSpeed = speed;
	attackPoint = point;
	attackRange = range;

	GetCharacterMovement()->MaxWalkSpeed = moveSpeed;
}


void ABaseMonster::Attack()
{
	if (!targetPlayer)
	{
		return;
	}

	FVector playerLocation = targetPlayer->GetActorLocation();
	FVector monsterLocation = GetActorLocation();

	float distance = FVector::Dist(playerLocation, monsterLocation);

	if (distance > attackRange)
	{
		return;
	}

	// 미구현 어택모션

	float playerHP = targetPlayer->GetCurrentHP();

	playerHP -= attackPoint;

	targetPlayer->SetHP(playerHP);
}

void ABaseMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	

}

void ABaseMonster::OnDamaged(float Damage)
{
	currentHP -= Damage;

	if (currentHP <= 0.0f)
	{
		currentHP = 0.0f;
		DieProcess();
	}
}

void ABaseMonster::DieProcess()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Destroy();
	// 미구현 죽음 모션
}

// Called to bind functionality to input
//void ABaseMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}