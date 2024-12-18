// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


/**
 * FFolderCleanerModule handles the operations related to asset management within the selected folders.
 * This class extends the IModuleInterface and provides various functions to filter, manage, and delete assets.
 */
class FFolderCleanerModule : public IModuleInterface
{
public:
	/**
	 * @brief Initializes the module when it is started.
	 *
	 * This function is called when the module is first loaded into memory.
	 */
	virtual void StartupModule() override;
	/**
	 * @brief Cleans up the module when it is shut down.
	 *
	 * This function is called when the module is unloaded or the application is closing.
	 */
	virtual void ShutdownModule() override;

	/**
	 * @brief Filters unused assets from the provided list of asset data.
	 *
	 * @param AssetDataToFilter The list of assets to be filtered.
	 * @param OutUnusedAssetData The output array containing assets that are unused.
	 */
	void ListUnusedAssetForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetData);

	/**
	 * @brief Filters assets that share the same name from the provided list of asset data.
	 *
	 * @param AssetDataToFilter The list of assets to be filtered.
	 * @param OutSameNameAssetData The output array containing assets with duplicate names.
	 */
	void ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetData);

	/**
	* @brief Called when the reference viewer button is clicked.
	*
	* @param RefAssetData The list of asset data to be viewed in the reference viewer.
	*/
	void OnReferenceViewerButtonClicked(TArray<FAssetData> RefAssetData);

	/**
	 * @brief Called when the size map button is clicked.
	 *
	 * @param RefAssetData The list of asset data to be displayed in the size map.
	 */
	void OnSizeMapButtonClicked(TArray<FAssetData> RefAssetData);

	/**
	 * @brief Opens the specified asset in the editor.
	 *
	 * @param AssetDataToOpen The asset data of the asset to open.
	 * @return True if the asset was successfully opened, false otherwise.
	 */
	bool OpenAsset(const FAssetData AssetDataToOpen);

	/**
	 * @brief Deletes multiple assets from the provided asset list.
	 *
	 * @param AssetArrayToDelete The array of asset data representing the assets to be deleted.
	 * @return True if the assets were successfully deleted, false otherwise.
	 */
	bool DeleteMultipleAssetsForAsssetList(const TArray<FAssetData> AssetArrayToDelete);

	/**
	 * @brief Deletes a single asset from the asset list.
	 *
	 * @param AssetDataToDelete The asset data representing the asset to delete.
	 * @return True if the asset was successfully deleted, false otherwise.
	 */
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);

	/**
	 * @brief Called when the delete empty folder button is clicked.
	 *
	 * Deletes all empty folders in the selected path.
	 */
	void OnDeleteEmptyFolderButtonClicked();

	void RefreshFolderCleanerTab();

protected:
	/**
	 * @brief Spawns the Folder Cleaner tab in the UI.
	 *
	 * @param TabArgs The arguments passed for spawning the tab.
	 * @return A shared reference to the dock tab widget.
	 */
	TSharedRef<SDockTab> OnSpawnFolderCleanerTab(const FSpawnTabArgs& TabArgs);

private:
	/**
	 * @brief Initializes the custom menu extension.
	 *
	 * This function adds custom menu entries to the UI for managing assets.
	 */
	void InitializeMenuExtention();

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
	void OnAdvancedDeletingButtonClicked();

	/**
	 * @brief Registers tabs for advanced deletion operations.
	 */
	void RegisterAdvancedDeletedTabs();

	/**
	 * @brief Retrieves all asset data from the selected folder.
	 *
	 * @return An array containing all the asset data under the selected folder.
	 */
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

	/**
	 * @brief Extends the custom menu with additional options based on the selected paths.
	 *
	 * @param SelectedPaths The array of strings representing the selected folder paths.
	 * @return A shared reference to the extender used for the menu.
	 */
	TSharedRef<FExtender> CustomMenuExtender(const TArray<FString>& SelectedPaths);
	
	/**
	 * @brief A shared pointer to the command list for the plugin.
	 *
	 * This member stores the commands associated with the plugin, allowing the plugin to respond
	 * to user interactions such as menu clicks or keyboard shortcuts.
	 */
	TSharedPtr<class FUICommandList> PluginCommands;

	/**
	 * Stores the paths of the selected folders.
	 *
	 * This array contains the paths of the folders currently selected by the user in the UI.
	 * The paths are represented as strings.
	 */
	TArray<FString> FolderPathsSelected;

};
