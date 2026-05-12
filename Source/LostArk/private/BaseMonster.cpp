// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "LostArkCharacter.h"

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
void ABaseMonster::InitMonster(int HP)
{
	maxHP = HP;
	currentHP = maxHP;
}
void ABaseMonster::InitMonster(int HP, float speed)
{
	InitMonster(HP);
	moveSpeed = speed;
}
void ABaseMonster::InitMonster(int HP, float speed, float point)
{
	InitMonster(HP, speed);
	attackPoint = point;
}
void ABaseMonster::InitMonster(int HP, float speed, float point, float range)
{
	InitMonster(HP, speed, point);
	attackRange = range;
}

void ABaseMonster::Attack(ACharacter* player)
{
	ALostArkCharacter* target = Cast<ALostArkCharacter>(player);
	
	if (!target)
	{
		return;
	}

	FVector playerLocation = target->GetActorLocation();
	FVector monsterLocation = GetActorLocation();

	float distance = FVector::Dist(playerLocation, monsterLocation);

	if (distance > attackRange)
	{
		return;
	}

	// 미구현 어택모션

	float playerHP = target->GetCurrentHP();

	playerHP -= attackPoint;

	target->SetHP(playerHP);
}

void ABaseMonster::OnDamaged(ACharacter* player)
{
	ALostArkCharacter* Target = Cast<ALostArkCharacter>(player);

	if (!Target)
	{
		return;
	}

	currentHP -= Target->GetAttackPoint();

	if (currentHP <= 0.0f)
	{
		DieProcess();
	}
}

void ABaseMonster::DieProcess()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 미구현 죽음 모션
}
	
	// Called every frame
void ABaseMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 미구현 플레이어 서치 및 돌격
}

// Called to bind functionality to input
//void ABaseMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}