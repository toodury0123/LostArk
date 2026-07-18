// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/SpawnPoint.h"
#include "Level/Crystal.h"

// Sets default values
ASpawnPoint::ASpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ASpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASpawnPoint::SetSpawnCrystalPoint(FVector point)
{
    FVector SpawnLocation = point;

    FHitResult Hit;

    const FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, 1000.f);
    const FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, 3000.f);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (!GetWorld()->LineTraceSingleByChannel(
        Hit,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        Params))
    {
        return;
    }

    SpawnLocation = Hit.ImpactPoint;

    GetWorld()->SpawnActor<ACrystal>(
        ACrystal::StaticClass(),
        SpawnLocation,
        GetActorRotation()
    );

}

// Called every frame
void ASpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

