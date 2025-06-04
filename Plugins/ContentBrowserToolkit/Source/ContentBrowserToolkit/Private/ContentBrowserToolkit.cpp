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
		FText::FromString("Find Duplicate Assets"),
		FText::FromString("Scan for duplicate assets in the selected folders"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.GenericFind"),
		FUIAction(FExecuteAction::CreateRaw(this, &FContentBrowserToolkitModule::FindDuplicateAssets))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Rename All Assets "),
		FText::FromString(""),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.GenericFind"),
		FUIAction(FExecuteAction::CreateRaw(this, &FContentBrowserToolkitModule::ShowRenameAssetsDialog))
	);
}

void FContentBrowserToolkitModule::OnDeleteEmptyFoldersClicked()
{
	const TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for(const FString& FolderPath : FolderPathsArray)
	{
		if(CBToolkit::IsExcludedFolder(FolderPath) || !UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if(!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath, true))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));
			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if(EmptyFoldersPathsArray.IsEmpty())
	{
		CBToolkit::ShowMessageDialog(EAppMsgType::Ok, TEXT("No empty Folder found under selected folder"), false);
		return;
	}

	const FString Msg = TEXT("Empty Folders founds in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?");
	const EAppReturnType::Type ConfirmResult = CBToolkit::ShowMessageDialog(EAppMsgType::YesNoCancel, Msg, false);

	if(ConfirmResult == EAppReturnType::Cancel || ConfirmResult == EAppReturnType::No) return;

	for(const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if(UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else
		{
			CBToolkit::PrintGEngineScreen(TEXT("Failed to Delete " + EmptyFolderPath), FColor::Red);
		}
	}
	if(Counter > 0)
	{
		CBToolkit::ShowNotifyInfo(TEXT("Successfully Deleted ") + FString::FromInt(Counter) + TEXT(" Folders"));
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
		AssetList.Append(FolderAssets);
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

void FContentBrowserToolkitModule::ShowRenameAssetsDialog()
{
	struct FAssetRenamePreview
	{
		FString OldName;
		FString NewName;

		FAssetRenamePreview(const FString& InOldName, const FString& InNewName)
			: OldName(InOldName), NewName(InNewName)
		{
		}
	};

	TArray<FString> AssetsInFolder = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	if(AssetsInFolder.Num() == 0)
	{
		CBToolkit::ShowMessageDialog(EAppMsgType::Ok, TEXT("No assets found in the selected folder."));
		return;
	}

	TArray<TSharedPtr<FAssetRenamePreview>> PreviewAssets;
	for(const FString& AssetPath : AssetsInFolder)
	{
		FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPath);
		if(AssetData.IsValid())
		{
			PreviewAssets.Add(MakeShareable(new FAssetRenamePreview(AssetData.AssetName.ToString(), AssetData.AssetName.ToString())));
		}
	}

	TSharedRef<SWindow> DialogWindow =
		SNew(SWindow)
		.Title(FText::FromString(TEXT("Rename Assets")))
		.ClientSize(FVector2D(500, 300))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	TSharedPtr<SEditableTextBox> PrefixTextBox;
	TSharedPtr<SEditableTextBox> SuffixTextBox;

	DialogWindow->SetContent(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Enter Prefix to Add")))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(PrefixTextBox, SEditableTextBox)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Enter Suffix to Add")))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(SuffixTextBox, SEditableTextBox)
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SListView<TSharedPtr<FAssetRenamePreview>>)
				.ItemHeight(24)
				.ListItemsSource(&PreviewAssets)
				.OnGenerateRow_Lambda([] (TSharedPtr<FAssetRenamePreview> AssetPreview, const TSharedRef<STableViewBase>& OwnerTable)
					{
						return SNew(STableRow<TSharedPtr<FAssetRenamePreview>>, OwnerTable)
							[
								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(STextBlock).Text(FText::FromString(AssetPreview->OldName)) // старое имя
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(STextBlock).Text(FText::FromString(AssetPreview->NewName)) // новое имя
									]
							];
					})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("Cancel")))
						.OnClicked_Lambda([DialogWindow] ()
							{
								DialogWindow->RequestDestroyWindow();
								return FReply::Handled();
							})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("Apply Changes")))
						.OnClicked_Lambda([PrefixTextBox, SuffixTextBox, &PreviewAssets, DialogWindow] ()
							{
								FString Prefix = PrefixTextBox->GetText().ToString();
								FString Suffix = SuffixTextBox->GetText().ToString();

								for(TSharedPtr<FAssetRenamePreview>& AssetPreview : PreviewAssets)
								{
									FString NewAssetName = Prefix + AssetPreview->OldName + Suffix;
									AssetPreview->NewName = NewAssetName;

									FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPreview->OldName);
									if(AssetData.IsValid())
									{
										TWeakObjectPtr<UObject> AssetObject = AssetData.GetAsset();

										FString NewPackagePath = AssetData.PackageName.ToString(); // Путь к пакету

										FAssetRenameData RenameData(AssetObject, NewPackagePath, NewAssetName);
										FAssetToolsModule::GetModule().Get().RenameAssets({ RenameData });
									}
								}

								DialogWindow->RequestDestroyWindow();
								CBToolkit::ShowNotifyInfo(TEXT("Assets renamed successfully."));
								return FReply::Handled();
							})
				]
		]
	);

	FSlateApplication::Get().AddModalWindow(DialogWindow, nullptr);
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FContentBrowserToolkitModule, ContentBrowserToolkit)