// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserDelegates.h"
#include "LevelEditor.h"

DECLARE_LOG_CATEGORY_EXTERN(BlueprintScannerLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(BlueprintScannerMessageLog, Log, All);

class FToolBarBuilder;
class FMenuBuilder;
class FUICommandList;
class UBlueprintScannerSettings;

class FBlueprintScannerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<FExtender>			 LevelEditorExtender;						/**< Extender for the Level Editor menu. */
	TSharedPtr<const FExtensionBase> LevelEditorExtension;						/**< Extension for the Level Editor. */
	TSharedPtr<const FExtensionBase> ContentBrowserExtension;					/**< Extension for the Content Browser. */
	TArray<FString>					 SelectedFolders;							/**< List of selected folder paths. */

	UPROPERTY()
	TArray<UBlueprint*> ErrorBlueprints;	/**< List of blueprints that failed to compile. */

	UPROPERTY()
	TArray<UBlueprint*> WarningBlueprints;	/**< List of blueprints that compiled with warnings. */
	
	void PluginButtonClicked();


	/**
	 * @brief Registers a button in the Level Editor toolbar.
	 */
	void RegisterLevelEditorButton();

	/**
	 * @brief Registers a button in the Content Browser context menu.
	 */
	void RegisterPathViewContextMenuButton();

		/**
	 * @brief Creates a custom Content Browser extender for selected paths.
	 * @param SelectedPaths List of selected folder paths in the Content Browser.
	 * @return A shared reference to the created Content Browser extender.
	 */
	TSharedRef<FExtender> CreateContentBrowserExtender(const TArray<FString>& SelectedPaths);

	/**
	 * @brief Callback for the "Refresh All" button click in the Level Editor.
	 */
	void RefreshAllButton_Clicked();

	/**
	 * @brief Callback for the "Refresh Path" button click in the Content Browser.
	 */
	void RefreshPathButton_Clicked();

	/**
	 * @brief Finds and refreshes blueprints based on the provided filter.
	 * @param Filter Asset filter criteria.
	 * @param bShouldExclude Whether to exclude specific paths during the search.
	 */
	void FindAndRefreshBlueprints(const FARFilter& Filter, bool bShouldExclude = true);

	/**
	 * @brief Adds an entry to the Level Editor menu.
	 * @param Builder Menu builder for creating the menu entry.
	 */
	void AddLevelEditorMenuEntry(FMenuBuilder& Builder);

	/**
	 * @brief Adds an entry to the Content Browser's path view context menu.
	 * @param Builder Menu builder for creating the context menu entry.
	 */
	void AddPathViewContextMenuEntry(FMenuBuilder& Builder);
	
private:

	const UBlueprintScannerSettings* GetCustomPluginSettings() const;


	void RegisterMenus();

		/**
	 * @brief Finds assets matching the given filter criteria.
	 * @param Filter Asset filter criteria.
	 * @param NumAsset Output parameter that stores the number of assets found.
	 * @return List of found asset data.
	 */
	TArray<FAssetData> FindAssets(const FARFilter& Filter, int32& NumAsset);

	/**
	 * @brief Retrieves a blueprint from the given asset data.
	 * @param Data Asset data for the blueprint.
	 * @return A weak pointer to the blueprint object.
	 */
	TWeakObjectPtr<UBlueprint> GetBlueprintFromAsset(const FAssetData& Data);

	/**
	 * @brief Logs debug information for a list of asset data.
	 * @param AssetData List of asset data to log.
	 */
	void PrintDebugMessageForAssetData(const TArray<FAssetData>& AssetData);

	/**
	 * @brief Displays a compilation notification for blueprints with problems or warnings.
	 * @param Blueprints List of blueprints to notify about.
	 * @param Message Notification message template.
	 * @param PropertyName Name of the icon property for the notification.
	 */
	void NotificationInfoCompilationMessage(TArray<UBlueprint*>& Blueprints, const FString& Message, const FName& PropertyName);

	/**
	 * @brief Compiles a given blueprint.
	 * @param Blueprint Weak pointer to the blueprint to compile.
	 * @param AssetPathString Asset path of the blueprint.
	 */
	void CompileBlueprint(TWeakObjectPtr<UBlueprint>& Blueprint, const FString& AssetPathString);

	/**
	 * @brief Displays debug messages on screen for problematic blueprints.
	 * @param Settings Settings object containing debug display configurations.
	 */
	void DisplayDebugMessage(const UBlueprintScannerSettings* Settings);

	/**
	 * @brief Resets the state of the internal blueprint lists.
	 */
	void ResetBlueprintsState();

	/**
	 * @brief Shows a progress notification for blueprint processing.
	 * @param NumAssets Number of assets being processed.
	 * @return Shared pointer to the created notification item.
	 */
	TSharedPtr<SNotificationItem> ShowProgressNotification(int32 NumAssets);

	/**
	 * @brief Displays a notification with a custom message and duration.
	 * @param Message Notification message text.
	 * @param Duration Duration of the notification in seconds.
	 */
	void ShowNotification(const FText& Message, float Duration);

	/**
	 * @brief Processes a list of asset data and returns packages that need saving.
	 * @param AssetDatas List of asset data to process.
	 * @param Settings Settings object for blueprint processing.
	 * @param bShouldExclude Whether to exclude certain paths during processing.
	 * @return List of packages to save.
	 */
	TArray<UPackage*> ProcessAssets(const TArray<FAssetData>& AssetDatas, const UBlueprintScannerSettings* Settings, bool bShouldExclude);

	/**
	 * @brief Determines if an asset should be skipped based on the given settings.
	 * @param Data Asset data to check.
	 * @param Settings Settings object for blueprint processing.
	 * @param bShouldExclude Whether to exclude certain paths during checking.
	 * @return True if the asset should be skipped, false otherwise.
	 */
	bool ShouldSkipAsset(const FAssetData& Data, const UBlueprintScannerSettings* Settings, bool bShouldExclude);

	/**
	 * @brief Displays a compilation notification for the current state of blueprints.
	 */
	void DisplayCompilationNotification();

	/**
	 * @brief Saves a list of packages to disk.
	 * @param PackagesToSave List of packages to save.
	 */
	void HandleSavePackages(const TArray<UPackage*>& PackagesToSave);

	/**
	 * @brief Updates the progress notification after blueprint processing.
	 * @param RefreshingNotification Shared pointer to the progress notification item.
	 * @param NumAssets Number of assets processed.
	 */
	void UpdateProgressNotification(TSharedPtr<SNotificationItem> RefreshingNotification, int32 NumAssets);



private:
	TSharedPtr<FUICommandList> PluginCommands;
};
