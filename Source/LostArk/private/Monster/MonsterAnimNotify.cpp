// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterAnimNotify.h"
#include "Monster/BaseMonster.h"
#include "Components/SkeletalMeshComponent.h"

void UMonsterAnimNotify::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	ABaseMonster* Monster = Cast<ABaseMonster>(MeshComp->GetOwner());

	if (!Monster)
		return;

	switch (NotifyType)
	{
	case EMonsterNotifyType::AttackHit:
		Monster->AttackHit();
		break;

	case EMonsterNotifyType::ResetAttack:
		Monster->ResetAttack();
		break;
	}
}