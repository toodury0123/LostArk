// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

class UInputMappingContext;
class UInputAction;
class UNiagaraSystem;

#include "LostArkPlayerController.generated.h"

/**
 * 
 */

UCLASS()
class LOSTARK_API ALostArkPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ALostArkPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

	FORCEINLINE UInputMappingContext* GetIMC() const { return IMC_LostArk; }

	// Input System
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* IMC_LostArk;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ia_LeftClick;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ia_RightClick;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ia_MouseWheel;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ia_Bomb;	

	UPROPERTY(EditAnywhere, Category = "Move")
	UNiagaraSystem* MoveClickEffect = nullptr;
	
	// 시작 후 이동과 슬라이딩을 기다리는 시간. 0초이면 즉시 허용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move", meta = (ClampMin = "0.0", Units = "s"))
	float StartDelay = 1.f;

	bool bLeftClickDown = false;
	bool bRightClickDown = false;

	FORCEINLINE bool IsLeftClickDown() const { return bLeftClickDown; }
	FORCEINLINE bool IsRightClickDown() const { return bRightClickDown; }

	void OnLeftClick(const FInputActionValue& InputValue);

public:

	bool GetMouseWorldLocation(FVector& OutLocation);

	void Zoom(const FInputActionValue& inputValue);

	bool IsMonster(AActor* Actor) const;
	void Attack(const FHitResult& Hit);
	
	void Move(const struct FInputActionValue& inputValue);
	void OnBomb();
	// Bomb 조준은 Visibility를 무시하는 몬스터도 찾고, 앞을 막는 벽은 유지한다.
	UFUNCTION(BlueprintCallable, Category = "Bomb|Targeting")
	bool GetBombHitFromRay(const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit) const;
	// 스페이스바 입력으로 마우스 방향 슬라이딩을 요청한다.
	void OnSlide();

private:
	// 시작 대기 중에만 사용하는 이동 잠금과 해제 타이머.
	bool bCanMove = true;
	FTimerHandle MoveLockTimerHandle;
	void LockMove(float Duration);
	void UnlockMove();

	void CancelCurrentMove();
	// 실제 커서와 자동 검증이 같은 Bomb 표적 판정 경로를 사용한다.
	bool GetBombHitUnderCursor(FHitResult& OutHit) const;
};
