// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OutlinerToolkit : ModuleRules
{
	public OutlinerToolkit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"UnrealEd",
				"EditorSubsystem",
				"LevelEditor",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"UMG",            
				"EditorStyle"    
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EditorStyle",
				"LevelEditor",
				"SceneOutliner",
				"Slate",
				"SlateCore",
				"Projects",
				"ToolMenus",
				"AssetTools",
				"Kismet",
				"BlueprintGraph",
				"EditorFramework",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
