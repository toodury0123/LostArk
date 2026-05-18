// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

//#include "BaseMonster.h"
#include "Monster/NormalMonster.h"

#include "Crystal.generated.h"

UCLASS()
class LOSTARK_API ACrystal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrystal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	float HP = 3;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* CrystalMesh;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ABaseMonster> NormalMonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ABaseMonster> NamedMonsterClass;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnMonster();

	void Damgaed();

	void Die();
};
