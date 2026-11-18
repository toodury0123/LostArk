// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "LostArkCharacter.generated.h"

class UCombatComponent;
class ALostArkPlayerController;
class UNiagaraSystem;
class UParticleSystem;
class UUserWidget;
class ULostArkHUD;
class UAnimMontage;
class UAnimSequence;
class UInputMappingContext;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class ABombExplosionCloud;

UCLASS(Blueprintable)
class LOSTARK_API ALostArkCharacter : public ACharacter

{	
	GENERATED_BODY()

public:
	ALostArkCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	FORCEINLINE UCameraComponent* GetTopDownCameraComponent() const
	{
		return TopDownCameraComponent;
	}

	FORCEINLINE USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}

	void SetCameraArmLength(float Length);

	// 오른손 본을 따라가는 장식용 총. 메시와 손잡이 보정값은 Blueprint에서 편집한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UStaticMeshComponent> GunProp;

	// 소서리스 애님 클래스가 사용할 반복 모션은 에디터에서 연결한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> IdleAnimation = nullptr;

	// 기존 이동 속도는 유지하고 실제 속도에 따라 이 모션의 재생 속도만 조절한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> WalkAnimation = nullptr;

	// 게임 시작 시 웅크린 착지 자세에서 일어나며, 미지정이면 기존 시작 동작을 유지한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Startup")
	TObjectPtr<UAnimSequence> StartupAnimation = nullptr;

	// 걷기 애니메이션이 1배속으로 재생되는 기준 속도(cm/s).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Locomotion", meta = (ClampMin = "1.0", Units = "cm/s"))
	float WalkAnimationSpeed = 180.f;

	// 진행 중인 공격/스킬/슬라이드를 방해하지 않는 짧은 피격 연출.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Reaction")
	TObjectPtr<UAnimMontage> HitMontage = nullptr;

	// 넘어짐과 일어나기를 한 번에 재생하며 기존 이동 잠금으로 입력을 제한한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Reaction")
	TObjectPtr<UAnimMontage> FallDownMontage = nullptr;

	// 넘어짐 판정이 필요한 기존 Blueprint/게임 규칙에서 명시적으로 호출한다.
	UFUNCTION(BlueprintCallable, Category = "Animation|Reaction")
	void PlayFallDownMotion();

public:
	// Left Click
	void Attack(const FHitResult& Point);

	// Press 1
	void BombAttack(const FHitResult& Point);

	// 마우스 방향으로 슬라이딩하며 장애물과 이동 가능 영역 경계에서 멈춘다.
	UFUNCTION(BlueprintCallable, Category = "Slide")
	void Slide();

	// 장애물이 없을 때 이동할 거리(cm).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0.0", Units = "cm"))
	float SlideDistance = 500.f;

	// 슬라이딩 이동에 걸리는 시간(초).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0.01", Units = "s"))
	float SlideDuration = 0.4f;

	// 이동은 코드가 담당하므로 루트 이동이 없는 슬라이딩 Montage를 지정한다.
	UPROPERTY(EditAnywhere, Category = "Slide")
	TObjectPtr<UAnimMontage> SlideMontage = nullptr;

	// Bomb 사용 가능 여부
	bool CanBomb() const
	{
		return bCanBomb;
	}

	// 공격/스킬 경직 중인지
	bool IsMovementLocked() const
	{
		return bMovementLocked;
	}


private:
	/*** Camera ***/

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = Camera,
		meta = (AllowPrivateAccess = "true")
	)
	UCameraComponent* TopDownCameraComponent;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = Camera,
		meta = (AllowPrivateAccess = "true")
	)
	USpringArmComponent* CameraBoom;


private:
	/*** Player Input ***/

	ALostArkPlayerController* PlayerController = nullptr;
	UInputMappingContext* IMC = nullptr;


public:
	/*** UI ***/

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULostArkHUD> LostArkHUDClass;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "UI",
		meta = (AllowPrivateAccess = "true")
	)
	ULostArkHUD* LostArkHUD = nullptr;


public:
	/*** Minimap ***/

	UPROPERTY(VisibleAnywhere)
	USceneCaptureComponent2D* MinimapCapture;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	UTextureRenderTarget2D* MinimapRenderTarget;

	UPROPERTY()
	UUserWidget* MinimapWidget;


public:
	/*** Status ***/

	FORCEINLINE float GetAttackPoint() const
	{
		return attackPoint;
	}

	FORCEINLINE float GetCurrentHP() const
	{
		return currentHP;
	}

	FORCEINLINE float GetCurrentMP() const
	{
		return currentMP;
	}

	FORCEINLINE FVector GetCharacterLocation() const
	{
		return GetActorLocation();
	}

	void SetHP(float HP);
	void SetMP(float MP);


private:
	float maxHP = 0.f;
	float maxMP = 0.f;

	float currentHP = 0.f;
	float currentMP = 0.f;

	float attackPoint = 0.f;

	void DieProccess();


public:
	// Keep attack tuning on the character for existing Blueprint serialization.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	UCombatComponent* CombatComponent;

	/*** Attack ***/

	UPROPERTY(EditAnywhere, Category = "Attack")
	UNiagaraSystem* AttackEffect;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 30.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage1;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage2;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackLockDuration = 0.2f;

	int32 AttackCount = 0;


public:
	/*** Bomb ***/

	// Bomb 발사 반대 방향으로 물러나는 거리(cm). 0이면 반동 이동과 작은 점프를 생략한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb|Recoil", meta = (ClampMin = "0.0", Units = "cm"))
	float backwardDistance = 50.f;

	// 현재 플레이어 스켈레톤의 발사·무릎 굽힘·착지 모션을 반동 진행에 맞춰 재생한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Bomb")
	TObjectPtr<UAnimSequence> BombRecoilAnimation = nullptr;

	// 애님 그래프가 이동 진행률과 같은 시점의 자세를 평가하도록 제공한다.
	bool IsBombRecoiling() const { return bBombRecoiling; }
	float GetBombRecoilProgress() const { return FMath::Clamp(BombRecoilElapsed / BombRecoilDuration, 0.f, 1.f); }

	// 기존 산탄총 발사 효과를 장식용 총의 총구에 부착한다. 미지정이면 생략한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Visual")
	TObjectPtr<UParticleSystem> BombMuzzleEffect = nullptr;

	// GunProp 로컬 공간의 총구 위치와 발사 방향은 메시 구성에 맞춰 편집한다.
	UPROPERTY(EditDefaultsOnly, Category = "Bomb|Visual")
	FTransform BombMuzzleTransform;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float BombDamage = 100.f;

	// 기존 폭탄의 피해 판정 반경이며 폭발 구름 표시 범위와 독립적이다.
	UPROPERTY(EditAnywhere, Category = "Bomb", meta = (ClampMin = "0.0", Units = "cm"))
	float BombRadius = 500.f;

	// BombExplosionCloud의 수평 표시 반경(cm). 피해 판정 반경은 변경하지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Visual", meta = (ClampMin = "1.0", Units = "cm", DisplayName = "Bomb Explosion Cloud Range"))
	float BombExplosionCloudRange = 500.f;

	// 이전 비활성화 요청을 재컴파일 후에도 유지하며, 필요할 때만 구름 표시를 켠다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bomb|Visual")
	bool bEnableBombExplosionCloud = false;

	// 지정하면 사용 시점에 클릭 위치에 구름을 표시하고 기존 Niagara 시각 효과를 대체한다.
	UPROPERTY(EditDefaultsOnly, Category = "Bomb|Visual")
	TSubclassOf<ABombExplosionCloud> BombCloudEffectClass;

	// 폭발 구름이 생성되어 사라지기까지의 전체 시간.
	UPROPERTY(EditAnywhere, Category = "Bomb|Visual", meta = (ClampMin = "0.05", Units = "s"))
	float BombEffectDuration = 1.f;

	// 구름의 바깥쪽 색상과 중심 색상은 에디터에서 독립적으로 조절한다.
	UPROPERTY(EditAnywhere, Category = "Bomb|Visual")
	FLinearColor BombOuterColor = FLinearColor(1.f, 0.18f, 0.015f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Bomb|Visual")
	FLinearColor BombInnerColor = FLinearColor(1.f, 0.85f, 0.035f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float BombLockDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float BombCoolTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	UAnimMontage* BombMontage;

	// AnimNotify
	UFUNCTION(BlueprintCallable)
	void BombHit();


private:
	FVector BombLocation = FVector::ZeroVector;

	bool bCanBomb = true;
	bool bBombAttackActive = false;

	FTimerHandle BombCoolTimeTimerHandle;
	// 애니메이션 알림이 없는 애님 클래스에서도 실제 폭탄 피해를 한 번 전달한다.
	FTimerHandle BombDamageTimerHandle;

	void ResetBombCoolTime();

	// 기존 Bomb 입력 잠금 동안만 진행하는 짧은 후퇴와 메시의 작은 점프 연출.
	void StartBombRecoil();
	void UpdateBombRecoil(float DeltaSeconds);
	void StopBombRecoil();
	bool bBombRecoiling = false;
	FVector BombRecoilDirection = FVector::ZeroVector;
	FVector BombRecoilNavLocation = FVector::ZeroVector;
	FVector BombRecoilMeshLocation = FVector::ZeroVector;
	float BombRecoilDistance = 0.f;
	static constexpr float BombRecoilDuration = 0.28f;
	float BombRecoilElapsed = 0.f;


private:
	bool bMovementLocked = false;
	// 진행 중 설정값이 바뀌어도 시작 시 정한 거리와 시간을 유지한다.
	bool bSliding = false;
	FVector SlideDirection = FVector::ZeroVector;
	// 슬라이딩 중 직선 경로가 NavMesh 밖으로 나가는지 확인할 기준점.
	FVector SlideNavLocation = FVector::ZeroVector;
	float SlideRemainingDistance = 0.f;
	float SlideRemainingTime = 0.f;
	float SlideSpeed = 0.f;
	bool bActionLocked = false;
	FTimerHandle ActionLockTimerHandle;
	void UnlockAction();

public:
	bool IsActionLocked() const
	{
		return bActionLocked;
	}
	FTimerHandle MovementLockTimerHandle;

	void LockMovement(float Duration);
	void EnableMovement();
};
	
