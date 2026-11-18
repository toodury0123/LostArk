#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BombExplosionCloud.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UStaticMesh;

// Niagara 구름을 클릭 위치에 재생하고 지정된 수명 뒤 정리하는 일회성 연출.
UCLASS(Blueprintable)
class LOSTARK_API ABombExplosionCloud : public AActor
{
	GENERATED_BODY()

public:
	ABombExplosionCloud();

	// 지연 스폰 완료 전에 호출하여 이번 폭발의 범위와 외형을 고정한다.
	void InitializeExplosion(float InRadius, float InDuration, FLinearColor InOuterColor, FLinearColor InInnerColor);

	// 메시와 재질은 Blueprint에서 지정하여 코드에 애셋 경로를 넣지 않는다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Assets")
	TObjectPtr<UStaticMesh> CloudMesh = nullptr;

	// 실제 구름 파티클과 팽창/소멸은 Niagara 시스템이 담당한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Assets")
	TObjectPtr<UNiagaraSystem> CloudSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Assets")
	TObjectPtr<UMaterialInterface> CloudMaterial = nullptr;

	// 수평 최대 반경(cm). 각 구름 덩어리가 이 원 안에 들어가도록 배치한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (ClampMin = "1.0", Units = "cm", ExposeOnSpawn = "true"))
	float Radius = 500.f;

	// 팽창과 페이드아웃을 모두 포함한 전체 표시 시간.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (ClampMin = "0.05", Units = "s", ExposeOnSpawn = "true"))
	float Duration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (ExposeOnSpawn = "true"))
	FLinearColor OuterColor = FLinearColor(1.f, 0.18f, 0.015f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (ExposeOnSpawn = "true"))
	FLinearColor InnerColor = FLinearColor(1.f, 0.85f, 0.035f, 1.f);

protected:
	virtual void BeginPlay() override;

private:
	// 이펙트 전체를 Niagara 컴포넌트 하나로 재생한다.
	UPROPERTY(VisibleAnywhere, Category = "Explosion")
	TObjectPtr<UNiagaraComponent> CloudNiagara;

	// 액터와 함께 정리되는 이번 폭발 전용 재질 인스턴스.
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial = nullptr;

};
