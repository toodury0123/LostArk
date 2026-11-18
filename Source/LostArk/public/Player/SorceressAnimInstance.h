#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SorceressAnimInstance.generated.h"

// 캐릭터에 지정한 대기/걷기 애셋과 기존 Montage를 재생하는 최소 애님 클래스.
UCLASS(Transient, Blueprintable)
class LOSTARK_API USorceressAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	// 그래프의 애니메이션 노드와 갱신용 데이터를 인스턴스별로 소유한다.
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;
};
