#include "Player/SorceressAnimInstance.h"
#include "Player/LostArkCharacter.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "AnimNodes/AnimNode_TwoWayBlend.h"
#include "AnimNodes/AnimNode_Slot.h"
#include "AnimNodes/AnimNode_SequenceEvaluator.h"

// UObject의 값은 게임 스레드에서만 읽고 평가 스레드에는 노드 설정만 전달한다.
struct FSorceressAnimInstanceProxy final : FAnimInstanceProxy
{
	explicit FSorceressAnimInstanceProxy(UAnimInstance* Instance)
		: FAnimInstanceProxy(Instance)
	{
		Locomotion.A.SetLinkNode(&Idle);
		Locomotion.B.SetLinkNode(&Walk);
		Arrival.A.SetLinkNode(&Locomotion);
		Arrival.B.SetLinkNode(&Startup);
		Recoil.A.SetLinkNode(&Arrival);
		Recoil.B.SetLinkNode(&RecoilPose);
		Slot.Source.SetLinkNode(&Recoil);
		Slot.SlotName = TEXT("DefaultSlot");
		Idle.SetLoopAnimation(true);
		Walk.SetLoopAnimation(true);
		Startup.SetLoopAnimation(false);
		RecoilPose.SetShouldLoop(false);
		RecoilPose.SetTeleportToExplicitTime(true);
	}

	virtual FAnimNode_Base* GetCustomRootNode() override { return &Slot; }

	virtual void GetCustomNodes(TArray<FAnimNode_Base*>& OutNodes) override
	{
		OutNodes.Append({ &Idle, &Walk, &Locomotion, &Startup, &Arrival, &RecoilPose, &Recoil, &Slot });
	}

	virtual void PreUpdate(UAnimInstance* Instance, float DeltaSeconds) override
	{
		FAnimInstanceProxy::PreUpdate(Instance, DeltaSeconds);
		const ALostArkCharacter* Character = Cast<ALostArkCharacter>(Instance->TryGetPawnOwner());
		if (!IsValid(Character)) { return; }

		Idle.SetSequence(Character->IdleAnimation);
		Walk.SetSequence(Character->WalkAnimation);
		Startup.SetSequence(Character->StartupAnimation);
		// 별도 스켈레톤 Montage 대신 실제 후퇴 진행률과 같은 시점의 발사·착지 자세를 사용한다.
		RecoilPose.SetSequence(Character->BombRecoilAnimation);
		const bool bHasRecoil = IsValid(Character->BombRecoilAnimation);
		RecoilPose.SetExplicitTime(bHasRecoil
			? Character->GetBombRecoilProgress() * Character->BombRecoilAnimation->GetPlayLength() : 0.f);
		Recoil.Alpha = FMath::FInterpTo(Recoil.Alpha,
			bHasRecoil && Character->IsBombRecoiling() ? 1.f : 0.f, DeltaSeconds, Character->IsBombRecoiling() ? 40.f : 20.f);
		// 첫 프레임부터 낮은 자세를 표시하고 마지막 0.35초에 대기 자세로 부드럽게 복귀한다.
		const float StartupLength = IsValid(Character->StartupAnimation) ? Character->StartupAnimation->GetPlayLength() : 0.f;
		const float Fade = FMath::Clamp((StartupLength - StartupElapsed) / 0.35f, 0.f, 1.f);
		Arrival.Alpha = Fade * Fade * Fade * (Fade * (Fade * 6.f - 15.f) + 10.f);
		StartupElapsed += FMath::Max(DeltaSeconds, 0.f);
		const float Speed = Character->GetVelocity().Size2D();
		// 매 프레임 속도를 반영해야 보행 시작과 정지가 부드럽게 이어진다.
		const float TargetAlpha = IsValid(Character->WalkAnimation) ? FMath::Clamp(Speed / 80.f, 0.f, 1.f) : 0.f;
		Locomotion.Alpha = FMath::FInterpTo(Locomotion.Alpha, TargetAlpha, DeltaSeconds, 8.f);
		// 실제 이동 속도는 유지하고 보행 재생 배속만 보간하여 출발·정지 시 급변을 줄인다.
		SmoothedPlayRate = FMath::FInterpTo(SmoothedPlayRate,
			FMath::Clamp(Speed / FMath::Max(Character->WalkAnimationSpeed, 1.f), 0.5f, 2.f), DeltaSeconds, 6.f);
		Walk.SetPlayRate(SmoothedPlayRate);
	}

private:
	// 시퀀스 노드는 애셋 경로 대신 캐릭터의 편집 가능한 참조를 사용한다.
	FAnimNode_SequencePlayer_Standalone Idle;
	FAnimNode_SequencePlayer_Standalone Walk;
	FAnimNode_TwoWayBlend Locomotion;
	// 시작 자세는 인스턴스당 한 번만 진행하며 새 타이머나 매 프레임 검색이 필요 없다.
	FAnimNode_SequencePlayer_Standalone Startup;
	FAnimNode_TwoWayBlend Arrival;
	// 반동 시퀀스는 피해 Notify나 루트 이동을 발생시키지 않고 자세만 평가한다.
	FAnimNode_SequenceEvaluator_Standalone RecoilPose;
	FAnimNode_TwoWayBlend Recoil;
	float StartupElapsed = 0.f;
	float SmoothedPlayRate = 0.5f;
	FAnimNode_Slot Slot;
};

FAnimInstanceProxy* USorceressAnimInstance::CreateAnimInstanceProxy()
{
	return new FSorceressAnimInstanceProxy(this);
}

void USorceressAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	delete static_cast<FSorceressAnimInstanceProxy*>(InProxy);
}
