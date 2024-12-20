// Copyright Epic Games, Inc. All Rights Reserved.

#include "FolderCleaner.h"
#include "EditorAssetLibrary.h"
#include "SFolderCleaningWidget.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "AssetManagerEditorModule.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"

#include "AssetViewUtils.h"

#define LOCTEXT_NAMESPACE "FFolderCleanerModule"

void FFolderCleanerModule::StartupModule()
{
	InitializeMenuExtention();
	//RegisterAdvancedDeletedTabs();

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuBarExtension("Help", EExtensionHook::After, nullptr, FMenuBarExtensionDelegate::CreateRaw(this, &FFolderCleanerModule::MakePulldownMenu));
	LevelEditorMenuExtensibilityManager->AddExtender(MenuExtender);
}

void FFolderCleanerModule::ShutdownModule() { }

void FFolderCleanerModule::InitializeMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FFolderCleanerModule::CustomMenuExtender));
}

TSharedRef<FExtender> FFolderCleanerModule::CustomMenuExtender(const TArray<FString>& SelectedPaths)
{
	// If no paths are selected, return an empty menu extender
	if (SelectedPaths.IsEmpty()) return TSharedRef<FExtender>();

	FolderPathsSelected = SelectedPaths;

	TSharedRef<FExtender> ContentMenuExtender(new FExtender());
	ContentMenuExtender->AddMenuExtension(FName("Delete"),																  //
								   EExtensionHook::After,															//
								   TSharedPtr<FUICommandList>(),													//
								   FMenuExtensionDelegate::CreateRaw(this, &FFolderCleanerModule::AddMenuEntry));	//
	
	return ContentMenuExtender;
}

void FFolderCleanerModule::MakePulldownMenu(FMenuBarBuilder& menuBuilder) 
{
	menuBuilder.AddPullDownMenu(
		FText::FromString("Custom"), 
		FText::FromString("Open the Custom menu"), 
		FNewMenuDelegate::CreateRaw(this, &FFolderCleanerModule::FillPulldownMenu), "Custom", FName(TEXT("CustomMenu")));
}

void FFolderCleanerModule::FillPulldownMenu(FMenuBuilder& menuBuilder) 
{
	menuBuilder.BeginSection("Tag Section", FText::FromString("Tag Settings Section "));
	menuBuilder.AddSubMenu(
					FText::FromString("Folder Structure"),
					FText::FromString("Description: "),
					FNewMenuDelegate::CreateLambda(
						[this](FMenuBuilder& SubMenuBuilder)
						{
							SubMenuBuilder.AddMenuEntry(
								FText::FromString("Open Folder Cleaner"),
								FText::FromString(""),
								FSlateIcon(),
								FUIAction(FExecuteAction::CreateLambda([this]() 
								{ 
									FName TabName = FName("FolderCleaner");

									TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(FolderCleaner::FolderCModuleName);
									if (ExistingTab.IsValid())
									{
										ExistingTab->ActivateInParent(ETabActivationCause::SetDirectly);
										return;
									}
									
									// Create a new tab manually
									TSharedRef<SDockTab> NewTab =
										SNew(SDockTab)
										.TabRole(ETabRole::NomadTab)
										.Label(FText::FromString("Folder Cleaner Plugin"))
										[
											SNew(SFolderCleaning)
											.AssetDataToStore(GetAllAssetDataUnderProjectDirFolder())
											.CurrentSelectedFolder(TEXT("/") + UEditorAssetLibrary::GetProjectRootAssetDirectory()) 
										];

									FGlobalTabmanager::Get()->InsertNewDocumentTab(TabName, FTabManager::ESearchPreference::PreferLiveTab, NewTab);
							    })));
						}));
		
	menuBuilder.EndSection();		
}

void FFolderCleanerModule::RegisterAdvancedDeletedTabs()
{
	/* clang-format off */
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("FolderCleaner"),			//
		FOnSpawnTab::CreateRaw(this, &FFolderCleanerModule::OnSpawnFolderCleanerTab));	//
}

void FFolderCleanerModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddSeparator();

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("FolderCleaner")),								//
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),			//
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),								//
		FExecuteAction::CreateRaw(this, &FFolderCleanerModule::OnFolderCleanerButtonClicked));		//
	
	MenuBuilder.AddSeparator();
}

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

void FFolderCleanerModule::OnFolderCleanerButtonClicked()
{
	// FixupRedirectors();
	// FGlobalTabmanager::Get()->TryInvokeTab(FName("FolderCleaner"));

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(FText::FromString(" Folder Cleaner Plugin "))
		[
			SNew(SFolderCleaning)
				.AssetDataToStore(GetAllAssetDataUnderSelectedFolder())
				.CurrentSelectedFolder(FolderPathsSelected[0])
		];

	FGlobalTabmanager::Get()->InsertNewDocumentTab(FName("FolderCleaner"), FTabManager::ESearchPreference::PreferLiveTab, NewTab);
}

void FFolderCleanerModule::OnReferenceViewerButtonClicked(TArray<FAssetData> RefAssetData)
{
	TArray<FAssetIdentifier> AssetIdentifiers;
	IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList(RefAssetData, AssetIdentifiers);
	IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetIdentifiers);
}

void FFolderCleanerModule::OnSizeMapButtonClicked(TArray<FAssetData> RefAssetData)
{
	TArray<FAssetIdentifier> AssetIdentifiers;
	IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList(RefAssetData, AssetIdentifiers);
	IAssetManagerEditorModule::Get().OpenSizeMapUI(AssetIdentifiers);
}

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

TSharedRef<SDockTab> FFolderCleanerModule::OnSpawnFolderCleanerTab(const FSpawnTabArgs& TabArgs)
{
	const FString Context = TabArgs.GetTabId().ToString(); 

	if (FolderPathsSelected.IsEmpty()) 
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			.Label(FText::FromString(" Folder Cleaner Plugin "))
			[
				SNew(SFolderCleaning)
					.AssetDataToStore(GetAllAssetDataUnderProjectDirFolder())
					.CurrentSelectedFolder(TEXT("/") + UEditorAssetLibrary::GetProjectRootAssetDirectory())
			];
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(FText::FromString(" Folder Cleaner Plugin "))
		[
			SNew(SFolderCleaning)
				.AssetDataToStore(GetAllAssetDataUnderSelectedFolder())
				.CurrentSelectedFolder(FolderPathsSelected[0])
		];
}

/* clang-format on */
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

TArray<TSharedPtr<FAssetData>> FFolderCleanerModule::GetAllAssetDataUnderProjectDirFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetsData;
	const TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(TEXT("/") + UEditorAssetLibrary::GetProjectRootAssetDirectory());

	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) ||		   //
			AssetPathName.Contains(TEXT("Collections")) ||		   //
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||  //
			AssetPathName.Contains(TEXT("__ExternalObjects__")))   //
			continue;											   //

		if (! UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AvaiableAssetsData.Add(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetsData;
}

bool FFolderCleanerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	return ObjectTools::DeleteAssets(AssetDataForDeletion) > 0;
}

bool FFolderCleanerModule::OpenAsset(const FAssetData AssetDataToOpen)
{
	if (! AssetDataToOpen.IsValid()) return false;

	const UObject* LoadedAsset = UEditorAssetLibrary::LoadAsset(AssetDataToOpen.ToSoftObjectPath().ToString());
	if (! LoadedAsset) return false;

	return AssetViewUtils::OpenEditorForAsset(AssetDataToOpen.ToSoftObjectPath().ToString());
}

bool FFolderCleanerModule::DeleteMultipleAssetsForAsssetList(const TArray<FAssetData> AssetArrayToDelete)
{
	return ObjectTools::DeleteAssets(AssetArrayToDelete) > 0;
}

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
		FolderCleaner::ShowMessageDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}
	const EAppReturnType::Type ConfirmResult = FolderCleaner::ShowMessageDialog(	//
		EAppMsgType::OkCancel,														//
		TEXT("Empty folders founds in:\n") +										//
		EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"),			//
		false);																		//

	if (ConfirmResult == EAppReturnType::Cancel) return;

	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else
		{
			FolderCleaner::PrintGEngineScreen(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
		}
	}
	if (Counter > 0)
	{
		FolderCleaner::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT(" folders"));
	}
}

void FFolderCleanerModule::RefreshFolderCleanerTab() 
{

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFolderCleanerModule, FolderCleaner)