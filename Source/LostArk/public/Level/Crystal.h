// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Crystal.generated.h"

class AWaveManager;

UCLASS()
class LOSTARK_API ACrystal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrystal();

	void SetWaveManager(AWaveManager* InWaveManager)
	{
		WaveManager = InWaveManager;
	}
private:
	UPROPERTY()
	AWaveManager* WaveManager = nullptr;

	// Shake
	FVector OriginalMeshLocation;

	FTimerHandle ShakeTimerHandle;
	FTimerHandle ShakeEndTimerHandle;

	void StartShake();
	void UpdateShake();
	void EndShake();

	UPROPERTY(EditAnywhere, Category = "Shake")
	float ShakeDuration = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Shake")
	float ShakeStrength = 10.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	int32 HP = 3;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* CrystalMesh;

public:
	virtual float TakeDamage(
		float DamageAmount,
		const FDamageEvent& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	virtual void Destroyed() override;
};