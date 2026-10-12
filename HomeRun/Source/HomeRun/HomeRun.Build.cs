using UnrealBuildTool; // 언리얼의 빌드 설정 도구입니다.

public class HomeRun : ModuleRules
{
    public HomeRun(ReadOnlyTargetRules Target) : base(Target)
    {
        // 자주 쓰는 헤더를 미리 컴파일해서 빌드 시간을 줄입니다.
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 게임 코드, 입력, UMG 화면에 필요한 모듈입니다.
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "SlateCore"
        });

        // UMG가 내부적으로 사용하는 화면 구성 모듈입니다.
        PrivateDependencyModuleNames.Add("Slate");
    }
}