using UnrealBuildTool;
 
public class ToolsProjectEditor : ModuleRules
{
	public ToolsProjectEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"UnrealEd",
			"LevelEditor",
			"Slate",
			"SlateCore",
			"Projects",
			"DataAssetManager",
			"ValidatorX"
		});
 
		PublicIncludePaths.AddRange(new string[] {"ToolsProjectEditor/Public"});
		PrivateIncludePaths.AddRange(new string[] {"ToolsProjectEditor/Private"});
	}
}