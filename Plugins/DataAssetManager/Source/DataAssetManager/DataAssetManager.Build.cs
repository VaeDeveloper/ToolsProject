// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DataAssetManager : ModuleRules
{
	public DataAssetManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
				"Engine",
				"AssetRegistry",
				"ApplicationCore",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"EditorStyle",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"PropertyEditor",
				"ContentBrowser",
				"DeveloperSettings",
				"EditorScriptingUtilities",
				"AssetManagerEditor",
				"SourceControl",
				"ClassViewer",
				"ToolWidgets",
				"StatusBar",
				"BlueprintGraph",
				"Kismet",
				"SettingsEditor",
				"PropertyEditor",
				"EditorWidgets",
				"MessageLog",
				"OutputLog"
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
