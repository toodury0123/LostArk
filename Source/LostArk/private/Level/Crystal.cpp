#include "Level/Crystal.h"
#include "Level/WaveManager.h"

ACrystal::ACrystal()
{
	PrimaryActorTick.bCanEverTick = false;

	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = CrystalMesh;

	CrystalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CrystalMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void ACrystal::BeginPlay()
{
	Super::BeginPlay();

	OriginalMeshLocation = CrystalMesh->GetRelativeLocation();
}

float ACrystal::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	StartShake();

	HP--;

	if (HP <= 0)
	{
		Destroy();
	}

	return DamageAmount;
}

void ACrystal::StartShake()
{
	// 연속으로 공격받았을 경우 기존 Shake 초기화
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);
	GetWorldTimerManager().ClearTimer(ShakeEndTimerHandle);

	GetWorldTimerManager().SetTimer(
		ShakeTimerHandle,
		this,
		&ACrystal::UpdateShake,	
		0.02f,
		true
	);

	GetWorldTimerManager().SetTimer(
		ShakeEndTimerHandle,
		this,
		&ACrystal::EndShake,
		ShakeDuration,
		false
	);
}

void ACrystal::UpdateShake()
{
	const FVector ShakeOffset(
		FMath::RandRange(-ShakeStrength, ShakeStrength),
		FMath::RandRange(-ShakeStrength, ShakeStrength),
		0.f
	);

	CrystalMesh->SetRelativeLocation(
		OriginalMeshLocation + ShakeOffset
	);
}

void ACrystal::EndShake()
{
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);

	CrystalMesh->SetRelativeLocation(OriginalMeshLocation);
}

void ACrystal::Destroyed()
{
	Super::Destroyed();

	if (WaveManager)
	{
		WaveManager->OnCrystalDestroyed();
	}
}