// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BaseMonster.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

#include "AIController.h"

#include "Player/LostArkCharacter.h"
#include "Level/WaveManager.h"

// Sets default values
ABaseMonster::ABaseMonster()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();
}

// Called when the game starts or when spawned
void ABaseMonster::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Controller: %s"), *GetNameSafe(GetController()));
	
	FindTarget();


	UE_LOG(LogTemp, Warning, TEXT("Monster BeginPlay / Target: %s / Controller: %s"),
		targetPlayer ? TEXT("Valid") : TEXT("Null"),
		GetController() ? TEXT("Valid") : TEXT("Null"));
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

void ABaseMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!targetPlayer)
	{
		FindTarget();
		return;
	}

	if (bIsAttacking)
		return;

	if (!IsTargetInAttackRange())
	{
		MoveToTarget(DeltaTime);
	}
	else
	{
		Attack();
	}
}

// ====================== Attack Proccess ==================== //

void ABaseMonster::Attack()
{
	if (!AttackMontage)
		return;

	if (bIsAttacking)
		return;

	bIsAttacking = true;

	GetCharacterMovement()->StopMovementImmediately();

	const float MontageDuration = PlayAnimMontage(AttackMontage);

	if (MontageDuration > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			AttackTimerHandle,
			this,
			&ABaseMonster::ResetAttack,
			MontageDuration,
			false
		);
	}
	else
	{
		ResetAttack();
	}
}

void ABaseMonster::AttackHit()
{
	if (!targetPlayer)
		return;

	if (!IsTargetInAttackRange())
		return;

	float playerHP = targetPlayer->GetCurrentHP();
	playerHP -= attackPoint;
	targetPlayer->SetHP(playerHP);
}

void ABaseMonster::ResetAttack()
{
	//UE_LOG(LogTemp, Warning, TEXT("ResetAttack Called"));
	bIsAttacking = false;
}

// ==================== Dying  Proccess ==================== //

float ABaseMonster::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	OnDamaged(DamageAmount);

	return DamageAmount;
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
	if (WaveManager)
	{
		WaveManager->OnMonsterKilled();
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Destroy();
	// 미구현 죽음 모션
}

// ================= Move Proccess ========================= // 

ALostArkCharacter* ABaseMonster::FindTarget()
{
	targetPlayer =
		Cast<ALostArkCharacter>(
			UGameplayStatics::GetPlayerCharacter(this, 0)
		);

	if (targetPlayer == nullptr)
		return nullptr;

	return targetPlayer;
}

void ABaseMonster::MoveToTarget(float DeltaTime)
{
	FVector direction =	targetPlayer->GetActorLocation() - GetActorLocation();

	direction.Z = 0.0f;

	if (!direction.IsNearlyZero())
	{
		direction.Normalize();

		AddMovementInput(direction, 1.0f);

		FRotator targetRotation = direction.Rotation();
		targetRotation.Yaw += 180.f;

		SetActorRotation(targetRotation);
	}
}

bool ABaseMonster::IsTargetInAttackRange() const
{
	if (!targetPlayer)
		return false;

	float Distance = FVector::Dist(
		GetActorLocation(),
		targetPlayer->GetActorLocation()
	);

	return Distance <= attackRange;
}

// Called to bind functionality to input
//void ABaseMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}