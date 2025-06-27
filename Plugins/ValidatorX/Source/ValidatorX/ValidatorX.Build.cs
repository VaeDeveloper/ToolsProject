// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ValidatorX : ModuleRules
{
	public ValidatorX(ReadOnlyTargetRules Target) : base(Target)
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
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"EditorScriptingUtilities",
				"BlueprintGraph",
				"Kismet",
				"DataValidation",
				"EditorStyle", 
				"LevelEditor",
				"InputCore",
				"ToolMenus"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
			);
	}
}
