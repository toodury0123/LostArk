#include "Combat/BombNiagaraAuthoringLibrary.h"
#include "Player/LostArkCharacter.h"
#include "NiagaraSystem.h"

#if WITH_EDITOR
#include "NiagaraEmitter.h"
#include "NiagaraScript.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraExternalSystemEditorUtilities.h"
#include "NiagaraVariant.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"

namespace
{
	// 형식이 있는 값은 엔진 API로 전달하며 Niagara 내부 메모리를 직접 수정하지 않는다.
	template<typename T>
	FNiagaraExt_SetParameterEntry MakeEntry(FName Name, const FNiagaraTypeDefinition& Type, const T& Value)
	{
		FNiagaraExt_SetParameterEntry Entry;
		Entry.Variable.Name = Name;
		Entry.Variable.Type = Type;
		Entry.DefaultValue.Set(Type, FNiagaraVariant(&Value, sizeof(T)));
		return Entry;
	}

	void SetFloat(const FNiagaraExt_StackItemReference& Ref, float Value, FNiagaraExternalEditContext& Context)
	{
		FNiagaraExt_StackInputValue Data;
		Data.InitializeAs<FNiagaraFloat>().Value = Value;
		UNiagaraExternalEditUtilities::SetStackInputData(Ref, Data, Context);
	}
}
#endif

UNiagaraSystem* UBombNiagaraAuthoringLibrary::BuildBombCloudSystem(UNiagaraEmitter* TemplateEmitter,
	UNiagaraScript* ShapeModule, UMaterialInterface* Material, const FString& AssetPath, FString& Report)
{
#if WITH_EDITOR
	if (!IsValid(TemplateEmitter) || !IsValid(ShapeModule) || !IsValid(Material))
	{
		Report = TEXT("TemplateEmitter, ShapeModule or Material is missing.");
		return nullptr;
	}
	FNiagaraExternalEditContext CreateContext;
	UNiagaraSystem* System = UNiagaraExternalEditUtilities::CreateNiagaraSystem(
		FPackageName::GetLongPackageAssetName(AssetPath), FPackageName::GetLongPackagePath(AssetPath), nullptr, CreateContext);
	if (!IsValid(System)) { Report = TEXT("Niagara system creation failed."); return nullptr; }
	FNiagaraExternalEditContext Context(System);
	FNiagaraExt_EmitterTopology Topology;
	UNiagaraExternalEditUtilities::AddEmitter(TemplateEmitter, TEXT("BombCloud"), Topology, Context);
	const FName EmitterName = Topology.EmitterName;

	// 한 번에 32개의 연기 스프라이트를 생성한다. 반복 루프는 Once로 고정한다.
	bool bBurstConfigured = false;
	for (const FNiagaraExt_ModuleTopology& Module : Topology.EmitterUpdateScript.Modules)
	{
		for (const FNiagaraExt_StackInputTopology& Input : Module.Inputs)
		{
			FNiagaraExt_StackItemReference Ref(System, EmitterName, TEXT("EmitterUpdateScript"), Module.ModuleName);
			Ref.InputNameStack.Add(Input.Name);
			FString Name = Input.Name.ToString().Replace(TEXT(" "), TEXT(""));
			if (Name.Contains(TEXT("SpawnCount")) && Input.Type == FNiagaraTypeDefinition::GetIntDef())
			{
				FNiagaraExt_StackInputValue Value;
				Value.InitializeAs<FNiagaraInt32>().Value = 32;
				UNiagaraExternalEditUtilities::SetStackInputData(Ref, Value, Context);
				bBurstConfigured = true;
			}
			if (Name.Contains(TEXT("LoopBehavior")) && Input.Type.GetEnum())
			{
				UEnum* Enum = Input.Type.GetEnum();
				for (int32 Index = 0; Index < Enum->NumEnums(); ++Index)
				{
					if (Enum->GetNameStringByIndex(Index).Contains(TEXT("Once")))
					{
						FNiagaraExt_StackInputValue Value;
						auto& Data = Value.InitializeAs<FNiagaraExt_StackInputData_Enum>();
						Data.Enum = Enum;
						Data.EnumName = Enum->GetNameByIndex(Index);
						UNiagaraExternalEditUtilities::SetStackInputData(Ref, Value, Context);
						break;
					}
				}
			}
		}
	}

	FNiagaraExt_ModuleTopology Shape;
	UNiagaraExternalEditUtilities::AddModule(
		FNiagaraExt_StackItemReference(System, EmitterName, TEXT("ParticleSpawnScript")), ShapeModule, Shape, Context);
	bool bRadiusConfigured = false;
	for (const auto& Input : Shape.Inputs)
	{
		if (Input.Name.ToString().Replace(TEXT(" "), TEXT("")) == TEXT("SphereRadius") && Input.Type == FNiagaraTypeDefinition::GetFloatDef())
		{
			FNiagaraExt_StackItemReference Ref(System, EmitterName, TEXT("ParticleSpawnScript"), Shape.ModuleName);
			Ref.InputNameStack.Add(Input.Name);
			SetFloat(Ref, 350.f, Context);
			bRadiusConfigured = true;
		}
	}
	if (!bRadiusConfigured)
	{
		// 엔진 버전별 Niagara 입력 이름이 다를 때 생성 실패 원인을 확인한다.
		for (const auto& Input : Shape.Inputs) { Report += Input.Name.ToString() + TEXT(";"); }
	}

	// 초기화 모듈의 기본값 대신 1초 수명과 300cm 연기 크기를 명시한다.
	TArray<FNiagaraExt_SetParameterEntry> Entries;
	FNiagaraFloat Lifetime; Lifetime.Value = 1.f;
	Entries.Add(MakeEntry(TEXT("Particles.Lifetime"), FNiagaraTypeDefinition::GetFloatDef(), Lifetime));
	Entries.Add(MakeEntry(TEXT("Particles.SpriteSize"), FNiagaraTypeDefinition::GetVec2Def(), FVector2f(300.f, 300.f)));
	Entries.Add(MakeEntry(TEXT("Particles.Velocity"), FNiagaraTypeDefinition::GetVec3Def(), FVector3f::ZeroVector));
	Entries.Add(MakeEntry(TEXT("Particles.Color"), FNiagaraTypeDefinition::GetColorDef(), FLinearColor::White));
	FNiagaraExt_ModuleTopology Overrides;
	UNiagaraExternalEditUtilities::AddSetParametersModule(
		FNiagaraExt_StackItemReference(System, EmitterName, TEXT("ParticleSpawnScript")), Entries, Overrides, Context);

	FNiagaraVariable MaterialParameter(FNiagaraTypeDefinition(UMaterialInterface::StaticClass()), TEXT("User.CloudMaterial"));
	System->GetExposedParameters().AddParameter(MaterialParameter);
	System->GetExposedParameters().SetUObject(Material, MaterialParameter);
	for (auto& Handle : System->GetEmitterHandles())
	{
		const FVersionedNiagaraEmitter Instance = Handle.GetInstance();
		if (FVersionedNiagaraEmitterData* Data = Instance.GetEmitterData())
		{
			Data->bLocalSpace = true;
			for (UNiagaraRendererProperties* Properties : Data->GetRenderers())
			{
				if (UNiagaraSpriteRendererProperties* Sprite = Cast<UNiagaraSpriteRendererProperties>(Properties))
				{
					Sprite->Material = Material;
					Sprite->MaterialUserParamBinding.Parameter = MaterialParameter;
					Sprite->FacingMode = ENiagaraSpriteFacingMode::FaceCamera;
				}
			}
		}
	}
	for (const FText& Error : Context.Errors) { Report += Error.ToString() + TEXT("\n"); }
	Report += FString::Printf(TEXT("BurstConfigured=%d RadiusConfigured=%d"), bBurstConfigured, bRadiusConfigured);
	if (Context.HasErrors() || !bBurstConfigured || !bRadiusConfigured) { return nullptr; }
	System->RequestCompile(true);
	System->WaitForCompilationComplete(true, false);
	System->MarkPackageDirty();
	return System;
#else
	Report = TEXT("Editor only.");
	return nullptr;
#endif
}

bool UBombNiagaraAuthoringLibrary::TestBombAtLocation(ALostArkCharacter* Character, FVector Location)
{
#if WITH_EDITOR
	if (!IsValid(Character) || !Character->CanBomb() || Character->IsActionLocked() || Character->IsMovementLocked()) { return false; }
	FHitResult Hit;
	Hit.bBlockingHit = true;
	Hit.ImpactPoint = Location;
	Character->BombAttack(Hit);
	return !Character->CanBomb();
#else
	return false;
#endif
}
