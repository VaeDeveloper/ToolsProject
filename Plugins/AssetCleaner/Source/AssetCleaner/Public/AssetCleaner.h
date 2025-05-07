// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
/* clang-format off */

class IAssetCleaner : public IModuleInterface
{
public:
	virtual void OpenManagerTab() = 0;
};

class FAssetCleanerModule : public IAssetCleaner
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void OpenManagerTab() override;

	static const FName AssetCleanerTabName;

private:
	/**
	 * @brief Initializes the custom menu extension.
	 *
	 * This function adds custom menu entries to the UI for managing assets.
	 */
	void InitializeMenuExtention();

	/**
	 * @brief Extends the custom menu with additional options based on the selected paths.
	 *
	 * @param SelectedPaths The array of strings representing the selected folder paths.
	 * @return A shared reference to the extender used for the menu.
	 */
	TSharedRef<FExtender> CustomMenuExtender(const TArray<FString>& SelectedPaths);

	TSharedRef<SDockTab> CreateAssetCleanerTab(const FSpawnTabArgs& Args);
	
	TSharedRef<SDockTab> CreateAssetCleanerTab(const TArray<TSharedPtr<FAssetData>>& AssetData, const FString& Path);


	/**
	 * @brief Adds a menu entry for asset operations.
	 *
	 * @param MenuBuilder A reference to the menu builder used to construct the UI menu.
	 */
	void AddMenuEntry(FMenuBuilder& MenuBuilder);

	void OnAssetCleanerButtonClicked();
	

	/**
	 * Retrieves all valid assets under the specified path, excluding those in ignored folders
	 * and those that do not exist. Uses PackagePath instead of full ObjectPath to avoid deprecated behavior.
	 *
	 * @param Path The virtual folder path (e.g., "/Game/MyFolder") to scan for assets.
	 * @return An array of shared pointers to valid FAssetData instances.
	 */
	TArray<TSharedPtr<FAssetData>> GetAllAssets(const FString& Path) const;
	
	void CreateAssetCleanerMenu(FMenuBuilder& SubMenuBuilder);

	
	/**
	 * Stores the paths of the selected folders.
	 *
	 * This array contains the paths of the folders currently selected by the user in the UI.
	 * The paths are represented as strings.
	 */
	TArray<FString> SelectedFolderPaths;

	/**
	 * A shared pointer to an extensibility manager for the Level Editor menu.
	 *
	 * This manager allows the modification and extension of the Level Editor's menu, enabling
	 * custom menu entries, actions, and functionality to be added.
	 */
	TSharedPtr<FExtensibilityManager> LevelEditorMenuExtensibilityManager;
};
