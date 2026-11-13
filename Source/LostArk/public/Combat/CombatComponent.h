#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UNiagaraSystem;

// Execution only: character retains input, montage selection, locks and tuning.
UCLASS()
class LOSTARK_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	void ExecuteBasicAttack(const FVector& Location, float Damage, float Radius, UNiagaraSystem* Effect);
};
