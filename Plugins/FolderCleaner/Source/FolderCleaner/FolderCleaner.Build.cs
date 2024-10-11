// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FolderCleaner : ModuleRules
{
	public FolderCleaner(ReadOnlyTargetRules Target) : base(Target)
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
                "Niagara",
                "AIModule",
                "NavigationSystem",
                "UMGEditor",
                "ContentBrowser",
                "AssetTools",
                "ContextualAnimation",
                "StructUtils",
                "AnimationSharing",
                "ContentBrowser",
                "ContentBrowserData",
                "EditorWidgets",
                "AssetManagerEditor",
                "CollectionManager",
                "ContentBrowser",
                "ContentBrowserData",
                "WorkspaceMenuStructure",
                "AssetDefinition",
                "AssetTools",
                "PropertyEditor",
                "GraphEditor",
                "BlueprintGraph",
                "KismetCompiler",
                "LevelEditor",
                "SandboxFile",
                "EditorWidgets",
                "TreeMap",
                "ToolMenus",
                "ToolWidgets",
                "SourceControl",
                "SourceControlWindows",
                "UncontrolledChangelists",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects",
                "InputCore",
                "LevelEditor",
                "CoreUObject",
                "ToolMenus",
                "Engine",
                "MainFrame",
                "ContentBrowser",
                "ContentBrowserData",
                "EditorWidgets",
                "AssetManagerEditor"

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
