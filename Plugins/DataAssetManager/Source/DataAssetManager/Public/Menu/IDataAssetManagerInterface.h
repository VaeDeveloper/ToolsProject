// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


/**
 * @class IDataAssetManagerInterface
 * @brief Interface for managing data assets in the system.
 *
 * This interface defines a set of methods for managing Data Assets within the application. It includes
 * functionalities for creating, opening, saving, validating, and interacting with Data Assets, as well as
 * integrating with source control and documentation tools.
 *
 * The implementing class should define the logic for each of the methods outlined in this interface, ensuring
 * that the user can interact with and manipulate Data Assets in a seamless and efficient manner.
 */
class IDataAssetManagerInterface
{
public:
	virtual ~IDataAssetManagerInterface() = default;

	/**
	 * @brief Creates a new Data Asset.
	 *
	 * This method will initiate the creation process of a new Data Asset in the system. It is expected to
	 * handle all the necessary steps required to create and register the new asset in the asset manager.
	 */
	virtual void CreateNewDataAsset() = 0;

	/**
	 * @brief Opens the selected Data Asset in the editor.
	 *
	 * This method allows the user to open an existing Data Asset in the editor. The editor will load the asset
	 * based on the current selection, and the asset will be presented for editing or viewing.
	 */
	virtual void OpenSelectedDataAssetInEditor() = 0;

	/**
	 * @brief Toggles the visibility of the Data Asset list.
	 *
	 * This method controls whether the Data Asset list is visible or hidden in the user interface.
	 * It allows the user to quickly show or hide the list of available assets.
	 */
	virtual void ToggleDataAssetListVisibility() = 0;

	/**
	 * @brief Displays the documentation for Data Assets.
	 *
	 * This method opens the documentation related to Data Assets. It could either show an internal documentation
	 * page or redirect the user to an external resource for more detailed information.
	 */
	virtual void ShowDocumentation() = 0;
	
	/**
	 * @brief Saves the current Data Asset.
	 *
	 * This method saves the current Data Asset to persistent storage. Any changes made to the asset will be committed
	 * to the underlying storage system.
	 */
	virtual void SaveDataAsset() = 0;
	
	/**
	 * @brief Saves all Data Assets.
	 *
	 * This method saves all the currently open or modified Data Assets in the system to persistent storage.
	 * It ensures that all changes are committed in a single batch.
	 */
	virtual void SaveAllData() = 0;
	
	/**
	 * @brief Validates the current Data Asset.
	 *
	 * This method checks the integrity and validity of the selected Data Asset. It may perform various validation
	 * checks to ensure the asset is correctly structured and free of errors.
	 */
	virtual void OnValidateDataAsset() = 0;
	
	/**
	 * @brief Validates all Data Assets.
	 *
	 * This method performs validation checks on all Data Assets in the system. It ensures that every asset is valid
	 * and free of errors, ensuring consistency across the entire dataset.
	 */
	virtual void OnValidateAllDataAsset() = 0;

	/**
	 * @brief Syncs the Content Browser to the selected Data Asset.
	 *
	 * This method ensures that the Content Browser reflects the current selection of the Data Asset. It may update
	 * the user interface to highlight or focus on the selected asset.
	 */
	virtual void SyncContentBrowserToSelectedAsset() = 0;
	
	/**
	 * @brief Copies the Data Asset paths to the clipboard.
	 *
	 * This method allows the user to copy the paths of one or more Data Assets to the clipboard. The `bCopyPaths`
	 * argument determines whether the full paths or just the names of the assets are copied.
	 *
	 * @param bCopyPaths Determines whether to copy full asset paths or just asset names.
	 */
	virtual void CopyToClipboard(bool bCopyPaths) = 0;

	/**
	 * @brief Opens the Reference Viewer for the selected Data Asset.
	 *
	 * This method opens the Reference Viewer, which allows the user to inspect the relationships and dependencies
	 * associated with the selected Data Asset. It provides insight into how the asset is used within the project.
	 */
	virtual void OpenReferenceViewer() = 0;
	
	/**
	 * @brief Opens the Size Map for the selected Data Asset.
	 *
	 * This method opens the Size Map, providing a visual representation of the size distribution of the selected
	 * Data Asset. It is useful for analyzing the resource usage of assets in the project.
	 */
	virtual void OpenSizeMap() = 0;
	
	/**
	 * @brief Opens the Audit Tool for the selected Data Asset.
	 *
	 * This method opens an auditing tool for the selected Data Asset. The audit may provide various metrics or
	 * diagnostic information about the asset's health, usage, and performance within the project.
	 */
	virtual void OpenAuditAsset() = 0;
	
	/**
	 * @brief Opens the plugin settings interface.
	 *
	 * This method opens the settings interface for the plugin managing the Data Assets. It allows the user to
	 * adjust configuration settings and preferences related to the asset management system.
	 */
	virtual void OpenPluginSettings() = 0;
	
	/**
	 * @brief Displays the Source Control Dialog.
	 *
	 * This method opens the Source Control dialog, allowing the user to interact with the version control system
	 * for managing changes to Data Assets. The dialog can be used to commit, update, or resolve conflicts in the
	 * asset management workflow.
	 */
	virtual void ShowSourceControlDialog() = 0;

	/**
	 * @brief Focuses the view on the currently selected asset.
	 *
	 * This method ensures the selected asset is brought into focus in the UI.
	 */
	virtual void FocusOnSelectedAsset() = 0;

	/**
	 * @brief Deletes the selected data asset.
	 *
	 * This method removes the currently selected asset from the system and updates the UI accordingly.
	 */
	virtual void DeleteDataAsset() = 0;

	/**
	 * @brief Restarts the plugin, reloading all its modules and resources.
	 *
	 * @note This will terminate all active plugin operations.
	 * @warning Unsaved changes in managed assets may be lost.
	 */
	virtual void RestartPlugin() = 0;


	/**
	 * @brief Opens the Message Log window with focus on plugin-specific messages.
	 *
	 * @see FMessageLog for message filtering capabilities.
	 * @remarks Automatically creates the log if it doesn't exist.
	 */
	virtual void OpenMessageLogWindow() = 0;

	/**
	 * @brief Opens the Output Log window and scrolls to the latest plugin entries.
	 *
	 * @remarks Useful for debugging low-level engine interactions.
	 * @see FOutputLogModule::Get().OpenOutputLog()
	 */
	virtual void OpenOutputLogWindow() = 0;

	/**
	 * @brief Checks if the currently selected asset can be renamed.
	 */
	virtual bool CanRename() const = 0;


	virtual void ShowAssetMetaData() = 0;

};