// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomTools : ModuleRules
{
	public CustomTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
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
                "Niagara",
                "AIModule",
                "NavigationSystem",
                "UMGEditor",
            }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Blutility",
                "UMG",
                "EditorScriptingUtilities",
                "Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
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
