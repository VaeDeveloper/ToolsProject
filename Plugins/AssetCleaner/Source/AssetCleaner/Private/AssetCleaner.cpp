// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetCleaner.h"
#include "UI/SAssetCleanerWidget.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "AssetManagerEditorModule.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "StatusBarSubsystem.h"

#define LOCTEXT_NAMESPACE "FAssetCleanerModule"
/* clang-format off */

namespace AssetCleaner
{	
	/**
	 * Static variable holding the root directory of the project.
	 */
	static const FString ProjectDirectory = TEXT("/") + UEditorAssetLibrary::GetProjectRootAssetDirectory();

	static constexpr const TCHAR* CBModuleName = TEXT("ContentBrowser");

	static bool IsExcludedFolder(const FString& FolderPath)
	{
		return FolderPath.Contains(TEXT("Developers")) 
			|| FolderPath.Contains(TEXT("Collections")) 
			|| FolderPath.Contains(TEXT("__ExternalActors__")) 
			|| FolderPath.Contains(TEXT("__ExternalObjects__"));
	}

}

const FName FAssetCleanerModule::AssetCleanerTabName = FName("AssetCleaner");

void FAssetCleanerModule::StartupModule()
{
	InitializeMenuExtention();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AssetCleanerTabName,
	FOnSpawnTab::CreateRaw(this, &FAssetCleanerModule::CreateAssetCleanerTab))
	.SetDisplayName(FText::FromString("Asset Cleaner"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);
}
TSharedRef<SDockTab> FAssetCleanerModule::CreateAssetCleanerTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(FText::FromString("Asset Cleaner"))
		[
			SNew(SAssetCleanerWidget)
			.DiscoveredAssets(GetAllAssets(SelectedFolderPaths[0]))
			.CurrentSelectedFolder(SelectedFolderPaths[0])
		];
}

TSharedRef<SDockTab> FAssetCleanerModule::CreateAssetCleanerTab(const TArray<TSharedPtr<FAssetData>>& AssetData, const FString & Path)
{
	TSharedRef<SDockTab> AssetCleanerTab = SNew(SDockTab).TabRole(ETabRole::NomadTab).Label(FText::FromString(Path));
	UStatusBarSubsystem* StatusBarSubsystem = GEditor->GetEditorSubsystem<UStatusBarSubsystem>();
	if (StatusBarSubsystem)
	{
		TSharedRef<SWidget> StatusBarWidget = StatusBarSubsystem->MakeStatusBarWidget(FName("AssetCleanerStatusBar"), AssetCleanerTab);

		AssetCleanerTab->SetContent(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f) 
			[
				SNew(SAssetCleanerWidget)
					.CurrentSelectedFolder(Path)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				StatusBarWidget
			]
		);
	}
	else
	{
		AssetCleanerTab->SetContent(SNew(SAssetCleanerWidget).CurrentSelectedFolder(Path));
	}

	return AssetCleanerTab;
}

void FAssetCleanerModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("AssetCleaner");
}

void FAssetCleanerModule::OpenManagerTab()
{
	const TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(AssetCleaner::AssetCleanerModuleName);

	if(ExistingTab.IsValid())
	{
		ExistingTab->ActivateInParent(ETabActivationCause::SetDirectly);
		return;
	}

	const TSharedRef<SDockTab> NewTab = CreateAssetCleanerTab(GetAllAssets(AssetCleaner::ProjectDirectory), AssetCleaner::ProjectDirectory);
	FGlobalTabmanager::Get()->InsertNewDocumentTab(AssetCleaner::AssetCleanerModuleName, FTabManager::ESearchPreference::PreferLiveTab, NewTab);
}

void FAssetCleanerModule::InitializeMenuExtention() 
{
	if (FModuleManager::Get().IsModuleLoaded(AssetCleaner::CBModuleName))
	{
		FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(AssetCleaner::CBModuleName);
		TArray<FContentBrowserMenuExtender_SelectedPaths>& CBModuleMenuExtenders = CBModule.GetAllPathViewContextMenuExtenders();
		CBModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FAssetCleanerModule::CustomMenuExtender));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ContentBrowserModule is not loaded"));
	}
}

TSharedRef<FExtender> FAssetCleanerModule::CustomMenuExtender(const TArray<FString>& SelectedPaths)
{
	if (SelectedPaths.IsEmpty()) return TSharedRef<FExtender>();
	SelectedFolderPaths = SelectedPaths;
	TSharedRef<FExtender> ContentMenuExtender(new FExtender());
	ContentMenuExtender->AddMenuExtension(FName("Delete"),															//
								   EExtensionHook::After,															//
								   TSharedPtr<FUICommandList>(),													//
								   FMenuExtensionDelegate::CreateRaw(this, &FAssetCleanerModule::AddMenuEntry));	//
	
	return ContentMenuExtender;
}

void FAssetCleanerModule::AddMenuEntry(FMenuBuilder & MenuBuilder)
{
	MenuBuilder.AddSeparator();
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("AssetCleaner")),								//
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),			//
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),								//
		FExecuteAction::CreateRaw(this, &FAssetCleanerModule::OnAssetCleanerButtonClicked));		//
	MenuBuilder.AddSeparator();
}

void FAssetCleanerModule::OnAssetCleanerButtonClicked()
{
	TSharedRef<SDockTab> AssetCleanerTab = CreateAssetCleanerTab(GetAllAssets(SelectedFolderPaths[0]), SelectedFolderPaths[0]);
	FGlobalTabmanager::Get()->InsertNewDocumentTab(FName("AssetCleaner"), FTabManager::ESearchPreference::PreferLiveTab, AssetCleanerTab );
}

TArray<TSharedPtr<FAssetData>> FAssetCleanerModule::GetAllAssets(const FString & Path) const
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;
	const TArray<FString> AssetsObjectPaths = UEditorAssetLibrary::ListAssets(Path);
	for (const FString& ObjectPath : AssetsObjectPaths)
	{
		/** Get all asset paths under the specified folder 
		 *  (returned as ObjectPaths like "/Game/Folder/Asset.Asset").
		 */
		FString PackagePath;
		ObjectPath.Split(TEXT("."), &PackagePath, nullptr);
		
		/** Skip excluded folders or assets that do not exist (using PackagePath to avoid deprecation warning).*/
		if (AssetCleaner::IsExcludedFolder(PackagePath) || 
			!UEditorAssetLibrary::DoesAssetExist(PackagePath))
		{
			continue;
		}

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(PackagePath);
		if (Data.IsValid())
		{
			AvailableAssetsData.Add(MakeShared<FAssetData>(Data));
		}
	}

	return AvailableAssetsData;
}

void FAssetCleanerModule::CreateAssetCleanerMenu(FMenuBuilder & SubMenuBuilder)
{
	SubMenuBuilder.AddMenuEntry(FText::FromString("Open Folder Cleaner"), 
							FText::FromString("Designed to optimize and manage folders and assets in a project"), 
							FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
							FUIAction(FExecuteAction::CreateLambda(
								[this]()
								{
									const TSharedPtr<SDockTab> ExistingTab = 
									FGlobalTabmanager::Get()->FindExistingLiveTab(AssetCleaner::AssetCleanerModuleName);

									if (ExistingTab.IsValid())
									{
										ExistingTab->ActivateInParent(ETabActivationCause::SetDirectly);
										return;
									}
									
									const TSharedRef<SDockTab> NewTab = 
									CreateAssetCleanerTab(GetAllAssets(AssetCleaner::ProjectDirectory), AssetCleaner::ProjectDirectory);
									FGlobalTabmanager::Get()->InsertNewDocumentTab(AssetCleaner::AssetCleanerModuleName, FTabManager::ESearchPreference::PreferLiveTab, NewTab);
								}
							)));
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetCleanerModule, AssetCleaner)