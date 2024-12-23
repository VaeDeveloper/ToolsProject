// Copyright Epic Games, Inc. All Rights Reserved.

#include "FolderCleaner.h"
#include "EditorAssetLibrary.h"
#include "SFolderCleaningWidget.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "AssetManagerEditorModule.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"


#define LOCTEXT_NAMESPACE "FFolderCleanerModule"

namespace FolderCleaner
{
	/**
	 * Static variable holding the root directory of the project.
	 *
	 * This variable combines a forward slash with the root directory of the project,
	 * as determined by the `UEditorAssetLibrary::GetProjectRootAssetDirectory()` function.
	 * It provides a convenient reference to the root directory for asset management
	 * and file operations within the project.
	 */
	static const FString ProjectDirectory = TEXT("/") + UEditorAssetLibrary::GetProjectRootAssetDirectory();


	static bool IsExcludedFolder(const FString& FolderPath) 
	{
		return FolderPath.Contains(TEXT("Developers")) 
			|| FolderPath.Contains(TEXT("Collections")) 
			|| FolderPath.Contains(TEXT("__ExternalActors__")) 
			|| FolderPath.Contains(TEXT("__ExternalObjects__"));
	}
}

void FFolderCleanerModule::StartupModule()
{
	InitializeMenuExtention();

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuBarExtension("Help", EExtensionHook::After, nullptr, FMenuBarExtensionDelegate::CreateRaw(this, &FFolderCleanerModule::MakePulldownMenu));
	LevelEditorMenuExtensibilityManager->AddExtender(MenuExtender);
}

void FFolderCleanerModule::ShutdownModule() {}



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
	ContentMenuExtender->AddMenuExtension(FName("Delete"),															//
								   EExtensionHook::After,															//
								   TSharedPtr<FUICommandList>(),													//
								   FMenuExtensionDelegate::CreateRaw(this, &FFolderCleanerModule::AddMenuEntry));	//
	
	return ContentMenuExtender;
}

void FFolderCleanerModule::MakePulldownMenu(FMenuBarBuilder& menuBuilder)
{
	menuBuilder.AddPullDownMenu(FText::FromString("Custom"), 
								FText::FromString("Open the Custom menu"), 
								FNewMenuDelegate::CreateRaw(this, &FFolderCleanerModule::FillPulldownMenu), "Custom", FName(TEXT("CustomMenu")));
}

void FFolderCleanerModule::FillPulldownMenu(FMenuBuilder& menuBuilder)
{
	menuBuilder.BeginSection("Folder Section", FText::FromString("Folder Settings Section "));

	menuBuilder.AddSubMenu(
		FText::FromString("Folders Structure"), 
		FText::FromString(" Section for Folder Structure "), 
		FNewMenuDelegate::CreateLambda(
			[this](FMenuBuilder& SubMenuBuilder) 
			{ 
				CreateFolderCleanerMenu(SubMenuBuilder); 
			}
		));

	menuBuilder.EndSection();
}

TSharedRef<SDockTab> FFolderCleanerModule::CreateFolderCleanerTab(const TArray<TSharedPtr<FAssetData>>& AssetData, const FString& Path)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(FText::FromString("Folder Cleaner Plugin"))
		[
			SNew(SFolderCleaning)
				.AssetDataToStore(AssetData)
				.CurrentSelectedFolder(Path)
		];
}

void FFolderCleanerModule::CreateFolderCleanerMenu(FMenuBuilder& SubMenuBuilder)
{
	SubMenuBuilder.AddMenuEntry(FText::FromString("Open Folder Cleaner"), 
								FText::FromString("Designed to optimize and manage folders and assets in a project"), 
								FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
								FUIAction(FExecuteAction::CreateLambda(
									[this]()
									{
										TSharedPtr<SDockTab> ExistingTab = 
										FGlobalTabmanager::Get()->FindExistingLiveTab(FolderCleaner::FolderCModuleName);

										if (ExistingTab.IsValid())
										{
											ExistingTab->ActivateInParent(ETabActivationCause::SetDirectly);
											return;
										}
										
										TSharedRef<SDockTab> NewTab = CreateFolderCleanerTab(GetAllAssets(FolderCleaner::ProjectDirectory), FolderCleaner::ProjectDirectory);
										
										FGlobalTabmanager::Get()->InsertNewDocumentTab(FolderCleaner::FolderCModuleName, FTabManager::ESearchPreference::PreferLiveTab, NewTab);
									}
								)));
};

void FFolderCleanerModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddSeparator();

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("FolderCleaner")),								//
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),			//
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),								//
		FExecuteAction::CreateRaw(this, &FFolderCleanerModule::OnFolderCleanerButtonClicked));		//
	
	MenuBuilder.AddSeparator();
}

void FFolderCleanerModule::OnFolderCleanerButtonClicked()
{
	TSharedRef<SDockTab> NewTab = CreateFolderCleanerTab(GetAllAssets(FolderPathsSelected[0]), FolderPathsSelected[0]);

	FGlobalTabmanager::Get()->InsertNewDocumentTab(FName("FolderCleaner"), FTabManager::ESearchPreference::PreferLiveTab, NewTab);
}

#pragma region IFolderInterface
TArray<TSharedPtr<FAssetData>> FFolderCleanerModule::GetAllAssets(const FString& Path) const
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;
	const TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(Path);

	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (FolderCleaner::IsExcludedFolder(AssetPathName) || ! UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AvailableAssetsData.Add(MakeShared<FAssetData>(Data));
	}

	return AvailableAssetsData;
}

bool FFolderCleanerModule::DeleteSingleAsset(const FAssetData& Asset) const
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(Asset);

	return ObjectTools::DeleteAssets(AssetDataForDeletion) > 0;
}

bool FFolderCleanerModule::DeleteMultiplyAsset(const TArray<FAssetData>& Assets) const
{
	return ObjectTools::DeleteAssets(Assets) > 0;
}

bool FFolderCleanerModule::OpenAsset(const FAssetData Asset)
{
	if (! Asset.IsValid()) return false;

	const UObject* LoadedAsset = UEditorAssetLibrary::LoadAsset(Asset.ToSoftObjectPath().ToString());
	if (! LoadedAsset) return false;

	return AssetViewUtils::OpenEditorForAsset(Asset.ToSoftObjectPath().ToString());
}

void FFolderCleanerModule::FilterUnusedAssets(const TArray<TSharedPtr<FAssetData>>& FilderAssetData, TArray<TSharedPtr<FAssetData>>& UnusedAssetData)
{
	UnusedAssetData.Empty();

	for (const TSharedPtr<FAssetData>& Data : FilderAssetData)
	{
		const TArray<FString> AssetRef = UEditorAssetLibrary::FindPackageReferencersForAsset(Data->PackageName.ToString());

		if (AssetRef.IsEmpty())
		{
			UnusedAssetData.Add(Data);
		}
	}
}

void FFolderCleanerModule::FilterDuplicateAssets(const TArray<TSharedPtr<FAssetData>>& FilterAssetData, TArray<TSharedPtr<FAssetData>>& DuplicateAssetData)
{
	DuplicateAssetData.Empty();

	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsInfoMultiMap;

	for (const TSharedPtr<FAssetData>& DataSharedPtr : FilterAssetData)
	{
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}

	for (const TSharedPtr<FAssetData>& DataShare : FilterAssetData)
	{
		TArray<TSharedPtr<FAssetData>> OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataShare->AssetName.ToString(), OutAssetsData);

		if (OutAssetsData.Num() <= 1) continue;

		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData)
		{
			if (SameNameData.IsValid())
			{
				DuplicateAssetData.AddUnique(SameNameData);
			}
		}
	}
}
#pragma endregion

void FFolderCleanerModule::OnReferenceViewerButtonClicked(TArray<FAssetData> RefAssetData)
{
    ProcessAssetData(RefAssetData, [](const TArray<FAssetIdentifier>& AssetIdentifiers)
    {
        IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetIdentifiers);
    });
}

void FFolderCleanerModule::OnSizeMapButtonClicked(TArray<FAssetData> RefAssetData)
{
    ProcessAssetData(RefAssetData, [](const TArray<FAssetIdentifier>& AssetIdentifiers)
    {
        IAssetManagerEditorModule::Get().OpenSizeMapUI(AssetIdentifiers);
    });
}

void FFolderCleanerModule::ProcessAssetData(const TArray<FAssetData>& RefAssetData, TFunction<void(const TArray<FAssetIdentifier>&)> ProcessFunction)
{
	TArray<FAssetIdentifier> AssetIdentifiers;
	IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList(RefAssetData, AssetIdentifiers);
	ProcessFunction(AssetIdentifiers);
}

void FFolderCleanerModule::OnDeleteEmptyFolderButtonClicked()
{
	const TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderCleaner::ProjectDirectory, true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath : FolderPathsArray)
	{
		if (FolderCleaner::IsExcludedFolder(FolderPath) || ! UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if (! UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));
			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if (EmptyFoldersPathsArray.IsEmpty())
	{
		FolderCleaner::ShowMessageDialog(EAppMsgType::Ok, TEXT("No empty Folder found under selected folder"), false);
		return;
	}

	const FString Msg = TEXT("Empty Folders founds in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?");
	const EAppReturnType::Type ConfirmResult = FolderCleaner::ShowMessageDialog(EAppMsgType::YesNoCancel,Msg,false);

	if (ConfirmResult == EAppReturnType::Cancel || ConfirmResult == EAppReturnType::No) return;

	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else
		{
			FolderCleaner::PrintGEngineScreen(TEXT("Failed to Delete " + EmptyFolderPath), FColor::Red);
		}
	}
	if (Counter > 0)
	{
		FolderCleaner::ShowNotifyInfo(TEXT("Successfully Deleted ") + FString::FromInt(Counter) + TEXT(" Folders"));
	}
}

void FFolderCleanerModule::FixupRedirectors() 
{
	/** Handler for when "Fix up Redirectors in Folder" is selected */

	TArray<UObjectRedirector*> RedirectorsToFixArray;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(FolderCleaner::ProjectDirectory);
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFolderCleanerModule, FolderCleaner)