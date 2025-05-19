// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AssetCleaner : ModuleRules
{
	public AssetCleaner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		

        PrivateIncludePaths.AddRange(
		new string[] {
		            System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "Source/Editor/Blutility/Private"
		}
		);

        PublicDependencyModuleNames.AddRange(
			new string[]
				{
					"Core",
					"Blutility",
					"UMG",
					"EditorScriptingUtilities",
					"UnrealEd",
                    "MaterialEditor",
                    "TargetPlatform",
                }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
				{
					"CoreUObject",
					"Engine",
					"Slate",
					"SlateCore",
					"InputCore",
					"LevelEditor",
					"ToolMenus",
					"MainFrame",
					"ContentBrowser",
					"ContentBrowserData",
					"EditorWidgets",
					"AssetManagerEditor",
					"SourceControl",
					"EditorWidgets",
					"ToolWidgets",
					"AssetRegistry",
					"StatusBar",
					"ApplicationCore",
					"RHI",
					"DeveloperSettings",

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
