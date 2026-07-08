#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MonsterAnimNotify.generated.h"

UENUM(BlueprintType)
enum class EMonsterNotifyType : uint8
{
	AttackHit,
	ResetAttack
};

UCLASS()
class LOSTARK_API UMonsterAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Notify")
	EMonsterNotifyType NotifyType = EMonsterNotifyType::AttackHit;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
};