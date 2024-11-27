// Copyright Epic Games, Inc. All Rights Reserved.

#include "FolderCleaner.h"
#include "EditorAssetLibrary.h"
#include "SFolderCleaningWidget.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "AssetManagerEditorModule.h"
#include "AssetViewUtils.h"

#define LOCTEXT_NAMESPACE "FFolderCleanerModule"

void FFolderCleanerModule::StartupModule()
{
	InitializeMenuExtention();
	RegisterAdvancedDeletedTabs();
}

void FFolderCleanerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

/**
 * @brief Initializes the menu extension for the automation plugin.
 *
 * This function adds a custom menu extender to the Content Browser, allowing the plugin to
 * integrate additional options into the context menu based on the selected paths.
 */
void FFolderCleanerModule::InitializeMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FFolderCleanerModule::CustomMenuExtender));
}

/**
 * @brief Extends the custom menu based on the selected paths in the Content Browser.
 *
 * @param SelectedPaths An array of strings representing the paths selected by the user in the
 * Content Browser. This function adds a custom "Delete" menu option for paths that have been selected.
 * @return A shared reference to an FExtender object that contains the extended menu options.
 */
TSharedRef<FExtender> FFolderCleanerModule::CustomMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),																	//
									   EExtensionHook::After,															//
									   TSharedPtr<FUICommandList>(),													//
									   FMenuExtensionDelegate::CreateRaw(this, &FFolderCleanerModule::AddMenuEntry));	//

		FolderPathsSelected = SelectedPaths;
	}

	return MenuExtender;
}

/**
 * @brief Registers a custom tab called "Advanced Deletion" in the global tab manager.
 *
 * This function registers a new tab spawner with the name "AdvancedDeletion" in the
 * global tab manager (`FGlobalTabmanager`). The tab is created using the
 * `OnSpawnAdvancedDeletionTab` function, which handles the tab's contents and behavior.
 * The display name of the tab is set to "Advanced Deletion".
 *
 * @note This function is primarily used to add a custom user interface tab within the editor.
 */
void FFolderCleanerModule::RegisterAdvancedDeletedTabs()
{
	/* clang-format off */
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("FolderCleaner"),			//
		FOnSpawnTab::CreateRaw(this, &FFolderCleanerModule::OnSpawnFolderCleanerTab))	//
		.SetDisplayName(FText::FromString(TEXT("FolderCleaner")));						//
}


/**
 * @brief Adds a custom menu entry to the automation plugin's menu.
 *
 * @param MenuBuilder A reference to the FMenuBuilder object used to build the menu. This function
 * adds a "Delete Unused Assets" entry to the menu, with the associated action to execute when selected.
 */
void FFolderCleanerModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("FolderCleaner")),								//
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),			//
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),								//
		FExecuteAction::CreateRaw(this, &FFolderCleanerModule::OnAdvancedDeletingButtonClicked));	//
}

/**
 * Filters and lists unused assets from the asset list.
 *
 * This method processes the provided array of asset data, filtering out assets that have no references
 * (i.e., unused assets). The resulting list of unused assets is stored in the output array `OutUnusedAssetData`.
 *
 * @param AssetDataToFilter An array of shared pointers to `FAssetData` objects representing the assets to be filtered.
 * @param OutUnusedAssetData An output array that will contain the shared pointers to unused assets.
 */
void FFolderCleanerModule::ListUnusedAssetForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetData)
{
	OutUnusedAssetData.Empty();

	for (const TSharedPtr<FAssetData>& Data : AssetDataToFilter)
	{
		const TArray<FString> AssetRef = UEditorAssetLibrary::FindPackageReferencersForAsset(Data->PackageName.ToString());

		if (AssetRef.Num() == 0)
		{
			OutUnusedAssetData.Add(Data);
		}
	}
}

/**
 * @brief Handler for the "Advanced Deleting" button click.
 *
 * This function is triggered when the "Advanced Deleting" button is clicked.
 * It attempts to open the tab named "AdvancedDeletion" in the editor UI.
 */
void FFolderCleanerModule::OnAdvancedDeletingButtonClicked()
{
	// FixupRedirectors();
	FGlobalTabmanager::Get()->TryInvokeTab(FName("FolderCleaner"));
}


/**
 * @brief Handler for the "Reference Viewer" button click.
 *
 * Opens the Reference Viewer UI for the specified list of assets.
 * This viewer allows users to explore asset references and dependencies.
 *
 * @param RefAssetData An array of asset data used to identify the assets whose references are to be viewed.
 */
void FFolderCleanerModule::OnReferenceViewerButtonClicked(TArray<FAssetData> RefAssetData)
{
	TArray<FAssetIdentifier> AssetIdentifiers;
	IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList(RefAssetData, AssetIdentifiers);
	IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetIdentifiers);
}

/**
 * @brief Handler for the "Size Map" button click.
 */
void FFolderCleanerModule::OnSizeMapButtonClicked(TArray<FAssetData> RefAssetData)
{
	TArray<FAssetIdentifier> AssetIdentifiers;
	IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList(RefAssetData, AssetIdentifiers);
	IAssetManagerEditorModule::Get().OpenSizeMapUI(AssetIdentifiers);
}

/**
 * Filters and lists assets with the same name from the asset list.
 *
 * This method processes the provided array of asset data, identifying assets that share the same name.
 * The resulting list of assets with duplicate names is stored in the output array `OutSameNameAssetData`.
 *
 * @param AssetDataToFilter An array of shared pointers to `FAssetData` objects representing the assets to be filtered.
 * @param OutSameNameAssetData An output array that will contain the shared pointers to assets with duplicate names.
 */
void FFolderCleanerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetData)
{
	OutSameNameAssetData.Empty();

	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsInfoMultiMap;

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetDataToFilter)
	{
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}

	for (const TSharedPtr<FAssetData>& DataShare : AssetDataToFilter)
	{
		TArray<TSharedPtr<FAssetData>> OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataShare->AssetName.ToString(), OutAssetsData);

		if (OutAssetsData.Num() <= 1) continue;

		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData)
		{
			if (SameNameData.IsValid())
			{
				OutSameNameAssetData.AddUnique(SameNameData);
			}
		}
	}
}

/**
 * @brief Spawns the "Advanced Deletion" tab in the editor UI.
 *
 * This function creates and returns a new tab of type `SDockTab` with the role
 * `ETabRole::NomadTab`. If no folder is selected (`FolderPathsSelected` is empty),
 * an empty tab is created and returned. Otherwise, the tab contains an
 * `SAdvancedDeletionTab` widget, which is initialized with asset data from the
 * selected folder and the folder path itself.
 *
 * @param TabArgs The arguments for spawning the tab, including metadata.
 * @return A reference to the newly created `SDockTab`.
 */
TSharedRef<SDockTab> FFolderCleanerModule::OnSpawnFolderCleanerTab(const FSpawnTabArgs& TabArgs)
{
	if (FolderPathsSelected.Num() == 0) return SNew(SDockTab).TabRole(ETabRole::NomadTab);

	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SFolderCleaning)
				.AssetDataToStore(GetAllAssetDataUnderSelectedFolder())
				.CurrentSelectedFolder(FolderPathsSelected[0])
		];
}

/* clang-format on */
/**
 * @brief Retrieves all asset data from the currently selected folder.
 *
 * This function gathers asset data from the first folder in `FolderPathsSelected`,
 * excluding assets in specific directories like "Developers", "Collections", and
 * other internal folders. It verifies the existence of each asset before adding its
 * data to the list.
 *
 * @return An array of shared pointers to `FAssetData` objects representing the assets
 *         found in the selected folder.
 */
TArray<TSharedPtr<FAssetData>> FFolderCleanerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetsData;
	const TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) ||			//
			AssetPathName.Contains(TEXT("Collections")) ||			//
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||	//
			AssetPathName.Contains(TEXT("__ExternalObjects__")))	//
			continue;												//

		if (! UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AvaiableAssetsData.Add(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetsData;
}

/**
 * Deletes a single asset from the asset list.
 *
 * This method adds the provided asset data to a temporary array and attempts to delete it using
 * the `ObjectTools::DeleteAssets` function. If the deletion is successful, it returns true; otherwise, it returns false.
 *
 * @param AssetDataToDelete The `FAssetData` object representing the asset to be deleted.
 * @return true if the asset was successfully deleted, false otherwise.
 */
bool FFolderCleanerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	return ObjectTools::DeleteAssets(AssetDataForDeletion) > 0;
}

/**
 * Opens an asset in the editor.
 *
 * This method checks the validity of the provided `FAssetData`. If valid, it loads the asset
 * and attempts to open it in the editor. The method returns true if the asset is successfully opened,
 * and false otherwise.
 *
 * @param AssetDataToOpen The `FAssetData` object representing the asset to be opened.
 * @return true if the asset was successfully opened, false otherwise.
 */
bool FFolderCleanerModule::OpenAsset(const FAssetData AssetDataToOpen)
{
	if (! AssetDataToOpen.IsValid()) return false;

	const UObject* LoadedAsset = UEditorAssetLibrary::LoadAsset(AssetDataToOpen.ToSoftObjectPath().ToString());
	if (! LoadedAsset) return false;

	return AssetViewUtils::OpenEditorForAsset(AssetDataToOpen.ToSoftObjectPath().ToString());
}

/**
 * Deletes multiple assets from the asset list.
 *
 * This method takes an array of `FAssetData` objects and attempts to delete all of them using
 * the `ObjectTools::DeleteAssets` function. If at least one asset is successfully deleted, it returns true; otherwise, false.
 *
 * @param AssetArrayToDelete An array of `FAssetData` objects representing the assets to be deleted.
 * @return true if at least one asset was successfully deleted, false otherwise.
 */
bool FFolderCleanerModule::DeleteMultipleAssetsForAsssetList(const TArray<FAssetData> AssetArrayToDelete)
{
	return ObjectTools::DeleteAssets(AssetArrayToDelete) > 0;
}

/**
 * @brief Called when the delete empty folder button is clicked.
 *
 * Deletes all empty folders in the selected path.
 */
void FFolderCleanerModule::OnDeleteEmptyFolderButtonClicked()
{
	const TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath : FolderPathsArray)
	{
		if (FolderPath.Contains(TEXT("Developers")) ||			//
			FolderPath.Contains(TEXT("Collections")) ||			//
			FolderPath.Contains(TEXT("__ExternalActors__")) ||	//
			FolderPath.Contains(TEXT("__ExternalObjects__")))	//
			continue;											//

		if (! UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if (! UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		Automation::ShowMessageDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}
	const EAppReturnType::Type ConfirmResult = Automation::ShowMessageDialog(	//
		EAppMsgType::OkCancel,													//
		TEXT("Empty folders founds in:\n") +									//
		EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"),		//
		false);																	//

	if (ConfirmResult == EAppReturnType::Cancel) return;

	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else
		{
			Automation::PrintGEngineScreen(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
		}
	}
	if (Counter > 0)
	{
		Automation::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT(" folders"));
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFolderCleanerModule, FolderCleaner)