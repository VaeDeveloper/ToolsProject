// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FCustomToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** This function will be bound to Command. */
	void PluginButtonClicked();

private:
	//
	void RegisterMenus();


	void InitializeMenuExtention();
	void RegisterFolderCleanerTabs();

protected:
	/**
	 * @brief Spawns the Folder Cleaner tab in the UI.
	 *
	 * @param TabArgs The arguments passed for spawning the tab.
	 * @return A shared reference to the dock tab widget.
	 */
	TSharedRef<SDockTab> OnSpawnFolderCleanerTab(const FSpawnTabArgs& TabArgs);




private:
	TSharedPtr<class FUICommandList> PluginCommands;

	/**
	 * Stores the paths of the selected folders.
	 *
	 * This array contains the paths of the folders currently selected by the user in the UI.
	 * The paths are represented as strings.
	 */
	TArray<FString> FolderPathsSelected;


	/**
	 * @brief Extends the custom menu with additional options based on the selected paths.
	 *
	 * @param SelectedPaths The array of strings representing the selected folder paths.
	 * @return A shared reference to the extender used for the menu.
	 */
	TSharedRef<FExtender> CustomMenuExtender(const TArray<FString>& SelectedPaths);

	/**
	 * @brief Adds a menu entry for asset operations.
	 *
	 * @param MenuBuilder A reference to the menu builder used to construct the UI menu.
	 */
	void AddMenuEntry(FMenuBuilder& MenuBuilder);

	/**
	 * @brief Called when the advanced delete button is clicked.
	 *
	 * Initiates the process of advanced asset deletion.
	 */
	void OnFolderCleanerButtonClicked();


};
