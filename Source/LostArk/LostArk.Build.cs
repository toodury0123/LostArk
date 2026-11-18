// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LostArk : ModuleRules
{
	public LostArk(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject",
			"Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput", "UMG" });

        // 대기/걷기 블렌딩과 기존 Montage 슬롯을 C++ 애님 그래프에서 사용한다.
        PrivateDependencyModuleNames.Add("AnimGraphRuntime");

        if (Target.bBuildEditor)
        {
            // 폭발 Niagara 애셋을 에디터에서 작성/검증할 때만 필요한 도구 의존성이다.
            PrivateDependencyModuleNames.AddRange(new string[] { "NiagaraEditor", "Json", "JsonUtilities" });
        }
    }
}
