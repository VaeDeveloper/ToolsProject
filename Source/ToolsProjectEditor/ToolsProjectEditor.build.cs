using UnrealBuildTool;
 
public class ToolsProjectEditor : ModuleRules
{
	public ToolsProjectEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd"});
 
		PublicIncludePaths.AddRange(new string[] {"ToolsProjectEditor/Public"});
		PrivateIncludePaths.AddRange(new string[] {"ToolsProjectEditor/Private"});
	}
}