#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BombNiagaraAuthoringLibrary.generated.h"

class UNiagaraSystem;
class UNiagaraEmitter;
class UNiagaraScript;
class UMaterialInterface;
class ALostArkCharacter;

// Niagara 에디터 API로 이 프로젝트의 폭발 애셋을 작성하고 실제 스킬 경로를 검사한다.
UCLASS()
class LOSTARK_API UBombNiagaraAuthoringLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Bomb|Editor")
	static UNiagaraSystem* BuildBombCloudSystem(UNiagaraEmitter* TemplateEmitter,
		UNiagaraScript* ShapeModule, UMaterialInterface* Material, const FString& AssetPath, FString& Report);

	// 입력의 지면 히트 결과를 재현해 실제 BombAttack의 생성 경로를 검증한다.
	UFUNCTION(BlueprintCallable, Category = "Bomb|Editor")
	static bool TestBombAtLocation(ALostArkCharacter* Character, FVector Location);
};
