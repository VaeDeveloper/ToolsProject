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

	/**
	 * Fixes up any redirectors in the project.
	 *
	 * Redirectors are references in the project that point to assets that have been moved or renamed.
	 * This function scans for such redirectors and attempts to resolve them, ensuring that all references
	 * point to the correct assets and that there are no broken links in the project.
	 */
	void FixupRedirectors();

private:
	/**
	 * @brief Retrieves all asset data located under the project's directory folder.
	 *
	 * @return An array of shared pointers to `FAssetData` objects, representing all assets
	 * found under the project's directory structure.
	 *
	 * This method scans the project's directory folder and collects metadata about all assets
	 * present within it. The returned `TArray` provides access to the asset data for further processing.
	 */
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderProjectDirFolder();

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
	void OnFolderCleanerButtonClicked();

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
	 * Retrieves all asset data under the specified root path.
	 *
	 * This function scans the directory structure starting from the given `RootPath`
	 * and returns an array of `TSharedPtr<FAssetData>`, each representing an asset
	 * found in the directories. The assets are filtered based on the root directory.
	 *
	 * @param RootPath The path to the directory where asset data will be collected from.
	 *
	 * @return An array of shared pointers to asset data found under the specified root path.
	 */
	TArray<TSharedPtr<FAssetData>> GetAllAssetData(const FString& RootPath);

	/**
	 * Creates a new folder cleaner tab with the specified asset data and folder path.
	 *
	 * This function creates and returns a new dockable tab containing a folder cleaner interface.
	 * It is initialized with the provided `AssetData` and the folder path (`Path`), allowing
	 * users to manage and clean folders within the project.
	 *
	 * @param AssetData An array of shared pointers to asset data to be displayed in the tab.
	 * @param Path The folder path that will be displayed in the folder cleaner interface.
	 *
	 * @return A reference to a new SDockTab representing the folder cleaner interface.
	 */
	TSharedRef<SDockTab> CreateFolderCleanerTab(const TArray<TSharedPtr<FAssetData>>& AssetData, const FString& Path);

	/**
	 * Creates a menu entry for opening the Folder Cleaner plugin.
	 *
	 * This function adds an entry to the menu that, when selected, will open the Folder Cleaner plugin.
	 * The menu entry is configured with a description and an icon. If the plugin is already open,
	 * the function will activate the existing tab; otherwise, it will create a new tab for the plugin.
	 *
	 * @param SubMenuBuilder The menu builder object that will have the menu entry added to it.
	 */
	void CreateFolderCleanerMenu(FMenuBuilder& SubMenuBuilder);

	/**
	 * Creates a pull-down menu and adds it to the specified menu builder.
	 *
	 * This function is responsible for constructing a pull-down menu and populating it with
	 * custom menu entries. The menu will be built using the provided `menuBuilder` object.
	 *
	 * @param menuBuilder The menu bar builder that the pull-down menu will be added to.
	 */
	void MakePulldownMenu(FMenuBarBuilder& menuBuilder);

	/**
	 * Fills the pull-down menu with the required menu entries.
	 *
	 * This function populates the pull-down menu with entries that provide the actions and
	 * options available to the user. It uses the provided `menuBuilder` object to add these
	 * entries to the menu.
	 *
	 * @param menuBuilder The menu builder object that the pull-down menu will be populated in.
	 */
	void FillPulldownMenu(FMenuBuilder& menuBuilder);

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

	/**
	 * A shared pointer to an extensibility manager for the Level Editor menu.
	 *
	 * This manager allows the modification and extension of the Level Editor's menu, enabling
	 * custom menu entries, actions, and functionality to be added.
	 */
	TSharedPtr<FExtensibilityManager> LevelEditorMenuExtensibilityManager;

	/**
	 * A shared pointer to a menu extender for the Level Editor menu.
	 *
	 * The menu extender is responsible for extending the existing menu system by adding custom
	 * menu entries or altering the layout of the menu.
	 */
	TSharedPtr<FExtender> MenuExtender;
};
