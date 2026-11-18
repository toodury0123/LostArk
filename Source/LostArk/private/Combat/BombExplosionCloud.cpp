#include "Combat/BombExplosionCloud.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Materials/MaterialInstanceDynamic.h"

ABombExplosionCloud::ABombExplosionCloud()
{
	PrimaryActorTick.bCanEverTick = false;
	CloudNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("CloudNiagara"));
	SetRootComponent(CloudNiagara);
	CloudNiagara->SetAutoActivate(false);
	CloudNiagara->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CloudNiagara->SetGenerateOverlapEvents(false);
	CloudNiagara->SetCanEverAffectNavigation(false);
	CloudNiagara->SetCastShadow(false);
}

void ABombExplosionCloud::InitializeExplosion(float InRadius, float InDuration,
	FLinearColor InOuterColor, FLinearColor InInnerColor)
{
	Radius = FMath::IsFinite(InRadius) ? FMath::Max(InRadius, 1.f) : 500.f;
	Duration = FMath::IsFinite(InDuration) ? FMath::Max(InDuration, 0.05f) : 1.f;
	OuterColor = InOuterColor;
	InnerColor = InInnerColor;
}

void ABombExplosionCloud::BeginPlay()
{
	Super::BeginPlay();
	InitializeExplosion(Radius, Duration, OuterColor, InnerColor);
	if (!IsValid(CloudSystem) || !IsValid(CloudMaterial))
	{
		UE_LOG(LogTemp, Error, TEXT("BombExplosionCloud: CloudSystem or CloudMaterial is missing."));
		Destroy();
		return;
	}

	DynamicMaterial = UMaterialInstanceDynamic::Create(CloudMaterial, this);
	if (!IsValid(DynamicMaterial)) { Destroy(); return; }
	DynamicMaterial->SetVectorParameterValue(TEXT("OuterColor"), OuterColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("InnerColor"), InnerColor);
	const FVector Center = GetActorLocation();
	DynamicMaterial->SetVectorParameterValue(TEXT("ExplosionCenter"), FLinearColor(Center.X, Center.Y, Center.Z, 1.f));
	DynamicMaterial->SetScalarParameterValue(TEXT("ExplosionRadius"), Radius);
	CloudNiagara->SetAsset(CloudSystem);
	CloudNiagara->SetVariableMaterial(TEXT("User.CloudMaterial"), DynamicMaterial);
	CloudNiagara->SetRelativeScale3D(FVector(Radius / 500.f));
	// Niagara의 기준 수명 1초를 사용자가 지정한 전체 표시 시간으로 늘이거나 줄인다.
	CloudNiagara->SetCustomTimeDilation(1.f / Duration);
	CloudNiagara->Activate(true);
	// 일회성 액터의 수명으로 컴포넌트와 동적 재질도 함께 정리한다.
	SetLifeSpan(Duration);
}

