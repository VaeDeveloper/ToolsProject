// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ContentBrowserToolkit : ModuleRules
{
	public ContentBrowserToolkit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				
			}
			);


		PrivateIncludePaths.AddRange(
		new string[] 
		{
			System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "Source/Editor/Blutility/Private"
		}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				
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
				"ContentBrowser",
				"EditorScriptingUtilities",
				"UnrealEd",
				"AssetTools",
				"AnimGraphRuntime",
				"UMG"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
