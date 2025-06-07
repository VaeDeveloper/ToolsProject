// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentBrowserToolkit.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ObjectTools.h"

// Slate
#include "UI/SUnusedAssetPickerDialog.h"
#include "UI/SDuplicateAssetsPickerDialog.h"


#include "HAL/FileManager.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"


#define LOCTEXT_NAMESPACE "FContentBrowserToolkitModule"

namespace CBToolkit
{
	static bool IsExcludedFolder(const FString& FolderPath)
	{
		return FolderPath.Contains(TEXT("Developers"))
			|| FolderPath.Contains(TEXT("Collections"))
			|| FolderPath.Contains(TEXT("__ExternalActors__"))
			|| FolderPath.Contains(TEXT("__ExternalObjects__"));
	}

	static void PrintGEngineScreen(const FString& Message, const FColor& Color)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, Color, Message);
		}
	}

	EAppReturnType::Type ShowMessageDialog(EAppMsgType::Type MessageType, const FString& Message, bool bShowMessageAsWarning = true)
	{
		if (bShowMessageAsWarning)
		{
			FText MessageTitle = FText::FromString(TEXT("Warning"));
			return FMessageDialog::Open(MessageType, FText::FromString(Message), MessageTitle);
		}
		else
		{
			return FMessageDialog::Open(MessageType, FText::FromString(Message));
		}
	}

	void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.0f;
		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}

}



void FContentBrowserToolkitModule::StartupModule()
{
	InitContentBrowserMenuExtension();
}

void FContentBrowserToolkitModule::ShutdownModule()
{

}

void FContentBrowserToolkitModule::InitContentBrowserMenuExtension()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	FContentBrowserMenuExtender_SelectedPaths CustomMenuDelegate;

	ContentBrowserMenuExtenders.Add(CustomMenuDelegate);
	ContentBrowserMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FContentBrowserToolkitModule::CustomContentBrowserMenuExtender));

}

TSharedRef<FExtender> FContentBrowserToolkitModule::CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if(SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this,
			&FContentBrowserToolkitModule::AddContentBrowserMenuEntry));

		FolderPathsSelected = SelectedPaths;
	}

	return MenuExtender;
}

void FContentBrowserToolkitModule::AddContentBrowserMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddSubMenu(
		FText::FromString(TEXT("Asset Actions Toolkit")),
		FText::FromString(TEXT("Advanced tools for asset management")),
		FNewMenuDelegate::CreateRaw(this, &FContentBrowserToolkitModule::PopulateAssetActionSubmenu),
		false, // default false
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.GameSettings"));
}

void FContentBrowserToolkitModule::PopulateAssetActionSubmenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Delete Unused Assets"),
		FText::FromString("Safely delete all unused assets"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Delete"),
		FUIAction(FExecuteAction::CreateRaw(this, &FContentBrowserToolkitModule::OnDeleteUnusedAssetClicked))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Delete Emtpy Folders"),
		FText::FromString(""),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "CurveEditor.DeleteTab.Small"),
		FUIAction(FExecuteAction::CreateRaw(this, &FContentBrowserToolkitModule::OnDeleteEmptyFoldersClicked))
	);

	// FSlateIcon(FName("EditorStyle"), "CurveEditor.DeleteTab.Small")

	MenuBuilder.AddMenuEntry(
		FText::FromString("Find Duplicate Assets"),
		FText::FromString("Scan for duplicate assets in the selected folders"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.GenericFind"),
		FUIAction(FExecuteAction::CreateRaw(this, &FContentBrowserToolkitModule::FindDuplicateAssets))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Add Prefixes"),
		FText::FromString("Scan for duplicate assets in the selected folders"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.GenericFind"),
		FUIAction(FExecuteAction::CreateRaw(this, &FContentBrowserToolkitModule::AddPrefix))
	);
}

void FContentBrowserToolkitModule::OnDeleteEmptyFoldersClicked()
{
	TArray<FString> EmptyFolders;
	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	TArray<FString> EmptyFoldersPathsArray;

	for(const FString& FolderPath : FolderPathsArray)
	{
		if(CBToolkit::IsExcludedFolder(FolderPath) || !UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if(!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath, true))
		{
			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if(EmptyFoldersPathsArray.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No empty folders found under the selected folder."));
		CBToolkit::ShowMessageDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	const FString Msg = TEXT("Empty folders found:\n") + FString::Join(EmptyFoldersPathsArray, TEXT("\n")) + TEXT("\nWould you like to delete all?");
	const EAppReturnType::Type ConfirmResult = CBToolkit::ShowMessageDialog(EAppMsgType::YesNoCancel, Msg, false);

	if(ConfirmResult == EAppReturnType::Cancel || ConfirmResult == EAppReturnType::No)
	{
		return;
	}

	FScopedSlowTask SlowTaskMain(
		1.0f,
		FText::FromString(TEXT("Deleting empty folders...")),
		GIsEditor && !IsRunningCommandlet()
	);
	SlowTaskMain.MakeDialog(false, false);
	SlowTaskMain.EnterProgressFrame(1.0f);

	FScopedSlowTask SlowTask(
		EmptyFoldersPathsArray.Num(),
		FText::FromString(TEXT(" ")),
		GIsEditor && !IsRunningCommandlet()
	);
	SlowTask.MakeDialog(false, false);

	bool bErrors = false;
	int32 NumFoldersDeleted = 0;

	for(const FString& FolderPath : EmptyFoldersPathsArray)
	{
		SlowTask.EnterProgressFrame(1.0f, FText::FromString(FolderPath));

		if(UEditorAssetLibrary::DeleteDirectory(FolderPath))
		{
			++NumFoldersDeleted;
		}
		else
		{
			bErrors = true;
			UE_LOG(LogTemp, Error, TEXT("Failed to delete folder: %s"), *FolderPath);
		}
	}

	const FString ResultMsg = FString::Printf(TEXT("Deleted %d of %d empty folders"), NumFoldersDeleted, EmptyFoldersPathsArray.Num());
	UE_LOG(LogTemp, Display, TEXT("%s"), *ResultMsg);

	if(bErrors)
	{
		UE_LOG(LogTemp, Error, TEXT("Some folders could not be deleted. Check the log for more details."));
		CBToolkit::ShowNotifyInfo(ResultMsg);
	}
	else
	{
		CBToolkit::ShowNotifyInfo(ResultMsg);
	}
}

void FContentBrowserToolkitModule::OnDeleteUnusedAssetClicked()
{
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	if(AssetsPathNames.Num() == 0)
	{
		CBToolkit::ShowMessageDialog(EAppMsgType::Ok, TEXT("No assets found under the selected folder."));
		return;
	}

	TArray<FAssetData> UnusedAssetDataArray;

	for(const FString& AssetPathName : AssetsPathNames)
	{
		// Skip special folders
		if(AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Collections")))
		{
			continue;
		}

		if(!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		// Check for referencers
		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);
		if(AssetReferencers.Num() == 0)
		{
			FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			if(UnusedAssetData.IsValid())
			{
				UnusedAssetDataArray.Add(UnusedAssetData);
			}
		}
	}

	if(UnusedAssetDataArray.Num() == 0)
	{
		CBToolkit::ShowMessageDialog(EAppMsgType::Ok, TEXT("No unused assets found under the selected folder."));
		return;
	}

	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(FText::FromString(TEXT("Select Assets to Delete")))
		.ClientSize(FVector2D(600, 400))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	TSharedPtr<SUnusedAssetPickerDialog> PickerWidget;

	PickerWindow->SetContent(
		SAssignNew(PickerWidget, SUnusedAssetPickerDialog)
		.Assets(UnusedAssetDataArray)
		.OnConfirmed(FOnUnusedAssetsConfirmed::CreateLambda([PickerWindow] (const TArray<FAssetData>& SelectedAssets)
			{
				if(SelectedAssets.Num() > 0)
				{
					ObjectTools::DeleteAssets(SelectedAssets);
				}
				else
				{
					CBToolkit::ShowMessageDialog(EAppMsgType::Ok, TEXT("No assets selected for deletion."));
				}
				PickerWindow->RequestDestroyWindow();
			}))
		.OnCanceled(FOnUnusedAssetsCanceled::CreateLambda([PickerWindow] ()
			{
				CBToolkit::ShowMessageDialog(EAppMsgType::Ok, TEXT("Deletion canceled."));
				PickerWindow->RequestDestroyWindow();
			}))
	);

	FSlateApplication::Get().AddModalWindow(PickerWindow, nullptr);
}

void FContentBrowserToolkitModule::FindDuplicateAssets()
{
	TMap<FString, TArray<FAssetData>> NameMap;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetList;

	for(const FString& FolderPath : FolderPathsSelected)
	{
		const FName FolderPathName = *FolderPath;
		TArray<FAssetData> FolderAssets;
		AssetRegistryModule.Get().GetAssetsByPath(FolderPathName, FolderAssets, true);

		for(const FAssetData& Asset : FolderAssets)
		{
			const FString AssetPath = Asset.PackagePath.ToString();
			if(!CBToolkit::IsExcludedFolder(AssetPath))
			{
				AssetList.Add(Asset);
			}
		}
	}

	for(const FAssetData& Asset : AssetList)
	{
		const FString Key = FString::Printf(TEXT("%s__%s"),
			*Asset.AssetName.ToString(),
			*Asset.AssetClassPath.ToString());

		NameMap.FindOrAdd(Key).Add(Asset);
	}

	TArray<TSharedPtr<FDuplicateAssetInfo>> DuplicateAssets;

	for(const auto& Pair : NameMap)
	{
		if(Pair.Value.Num() > 1)
		{
			TSharedPtr<FDuplicateAssetInfo> NewInfo = MakeShareable(new FDuplicateAssetInfo);
			NewInfo->AssetName = Pair.Key;
			NewInfo->Assets = Pair.Value;
			DuplicateAssets.Add(NewInfo);
		}
	}

	if(DuplicateAssets.Num() > 0)
	{
		ShowDuplicateAssetsWindow(DuplicateAssets);
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("No duplicate assets found.")));
	}
}

void FContentBrowserToolkitModule::ShowDuplicateAssetsWindow(const TArray<TSharedPtr<FDuplicateAssetInfo>>& DuplicateAssets)
{
	TSharedRef<SWindow> PickerWindow = 
		SNew(SWindow)
		.Title(FText::FromString(TEXT("Duplicate Assets Found")))
		.ClientSize(FVector2D(800, 400))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	TSharedPtr<SDuplicateAssetsPickerDialog> PickerWidget;

	PickerWindow->SetContent
	(
		SAssignNew(PickerWidget, SDuplicateAssetsPickerDialog)
		.DuplicateAssets(DuplicateAssets)
	);

	FSlateApplication::Get().AddWindow(PickerWindow);
}

void FContentBrowserToolkitModule::AddPrefix()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	TArray<FAssetRenameData> RenameDataArray;
	uint32 Counter = 0;

	for(const FString& FolderPath : FolderPathsSelected)
	{
		TArray<FAssetData> FolderAssets;
		const FName FolderPathName = *FolderPath;

		// Получаем ассеты из указанной папки
		AssetRegistryModule.Get().GetAssetsByPath(FolderPathName, FolderAssets, true);

		for(const FAssetData& Asset : FolderAssets)
		{
			if(!Asset.IsValid())
				continue;

			const FString AssetPath = Asset.PackagePath.ToString();
			if(CBToolkit::IsExcludedFolder(AssetPath))
				continue;

			const UClass* AssetClass = Asset.GetClass();
			if(!AssetClass)
				continue;

			const FString* Prefix = AssetRenameConfig.PrefixMap.Find(AssetClass);
			if(!Prefix || Prefix->IsEmpty())
			{
				CBToolkit::PrintGEngineScreen("No prefix for " + AssetClass->GetName(), FColor::Red);
				continue;
			}

			FString OldName = Asset.AssetName.ToString();

			if(OldName.StartsWith(*Prefix))
			{
				CBToolkit::PrintGEngineScreen(OldName + TEXT(" already has prefix"), FColor::Yellow);
				continue;
			}

			// Специальная логика для Material Instance
			if(AssetClass->IsChildOf(UMaterialInstanceConstant::StaticClass()))
			{
				OldName.RemoveFromStart(TEXT("M_"));
				OldName.RemoveFromEnd(TEXT("_Inst"));
			}

			const FString NewName = *Prefix + OldName;

			// Загружаем UObject для использования с FAssetRenameData
			UObject* AssetObj = Asset.GetAsset();
			if(!AssetObj)
			{
				CBToolkit::PrintGEngineScreen("Failed to load asset: " + Asset.ObjectPath.ToString(), FColor::Red);
				continue;
			}

			FAssetRenameData RenameData(AssetObj, Asset.PackagePath.ToString(), NewName);
			RenameDataArray.Add(MoveTemp(RenameData));
			++Counter;
		}
	}

	if(RenameDataArray.Num() > 0)
	{
		AssetTools.RenameAssets(RenameDataArray);
		CBToolkit::ShowNotifyInfo("Renamed " + FString::FromInt(Counter) + " assets with prefixes.");
	}
	else
	{
		CBToolkit::ShowNotifyInfo("No assets needed renaming.");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FContentBrowserToolkitModule, ContentBrowserToolkit)