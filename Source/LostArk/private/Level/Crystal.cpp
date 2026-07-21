// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Crystal.h"
#include "Level/WaveManager.h"

// Sets default values
ACrystal::ACrystal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = CrystalMesh;

	CrystalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CrystalMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

// Called when the game starts or when spawned
void ACrystal::BeginPlay()
{
	Super::BeginPlay();
}


float ACrystal::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser) 
{
	HP--;

	if (HP <= 0)
	{
		Destroy();
	}

	//UE_LOG(LogTemp, Warning, TEXT("Crystal took damage: %f"), DamageAmount);

	return DamageAmount;
}

void ACrystal::Destroyed()
{
	Super::Destroyed();

	if (WaveManager)
	{
		WaveManager->OnCrystalDestroyed();
	}
}