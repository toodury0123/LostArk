#include "Combat/CombatComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::ExecuteBasicAttack(const FVector& Location, float Damage, float Radius, UNiagaraSystem* Effect)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn) || !GetWorld())
	{
		return;
	}

	if (Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), Effect, Location, FRotator::ZeroRotator,
			FVector::OneVector, true, true, ENCPoolMethod::None, true);
	}

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(OwnerPawn);
	UGameplayStatics::ApplyRadialDamage(
		GetWorld(), Damage, Location, Radius, UDamageType::StaticClass(),
		IgnoreActors, OwnerPawn, OwnerPawn->GetController(), true);
}
