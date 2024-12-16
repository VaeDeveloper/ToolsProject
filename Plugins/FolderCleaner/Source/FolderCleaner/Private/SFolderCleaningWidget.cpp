// Fill out your copyright notice in the Description page of Project Settings.

#include "SFolderCleaningWidget.h"
#include "SlateBasics.h"
#include "FolderCleaner.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SBorder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

namespace
{
	static constexpr float TitleInfoFontTextSize = 30.0f;
	static FName ListAllAssetsName = TEXT("All Available Assets");
	static FName ListUnusedAssetsName = TEXT("List of Unused Assets");
	static FName ListAssetWithSameName = TEXT("List Assets With The Same Name");
}

void SFolderCleaning::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetsData = InArgs._AssetDataToStore;
	DisplayedAssetData = StoredAssetsData;
	CheckBoxArray.Empty();
	AssetDataToDeleteArray.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(ListAllAssetsName.ToString()));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnusedAssetsName.ToString()));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListAssetWithSameName.ToString()));

	GetTypeOfAssets(InArgs._AssetDataToStore);


	FSlateFontInfo TitleTextFontInfo = GetEmboseedTextFont();
	TitleTextFontInfo.Size = TitleInfoFontTextSize;

#pragma region SlateRegion
	/* clang-format off */
	ChildSlot[SNew(SVerticalBox)

				// Title of the widget
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("FolderCleaner")))
						.Font(TitleTextFontInfo)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FColor::White)
						.ToolTipText(FText::FromString(TEXT(" Folder Cleaner")))
				]

				// Combo Box for asset selection
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox) +
						SHorizontalBox::Slot()
						.AutoWidth()
						[
							ConstructComboBox()
						]

						+SHorizontalBox::Slot()
						.FillWidth(0.1f)
						[
							ConstructAssetComboBox()
						]


				// Help text for the combo box selection
				+ SHorizontalBox::Slot()
						.FillWidth(0.6f)
						[
							ConstructComboHelpTexts(TEXT("Specify the listing condition in the drop."), ETextJustify::Center)
						]

				// Current folder display								  
				+ SHorizontalBox::Slot()
						.FillWidth(0.1f)
						[
							ConstructComboHelpTexts(TEXT("Current Folder: \n") + InArgs._CurrentSelectedFolder, ETextJustify::Right)
						]


				]

				// Scrollable asset list view
				+ SVerticalBox::Slot().VAlign(VAlign_Fill)
				[
					SNew(SScrollBox) 
						+ SScrollBox::Slot()
						[
							ConstructAssetListView()
						]
				]

				// Control buttons: Delete All, Select All, Deselect All
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox) +
						SHorizontalBox::Slot()
						.FillWidth(10.0f)
						.Padding(5.0f)
						[
							ConstructDeleteAllButton()	// Constructs the button to delete all selected assets
						]

				+ SHorizontalBox::Slot()
						.FillWidth(10.0f)
						.Padding(5.0f)
						[
							ConstructSelectAllButton()  // Constructs the button to select all assets
						]

				+ SHorizontalBox::Slot().
						FillWidth(10.0f)
						.Padding(5.0f)
						[
							ConstructDeselectAllButton()	// Constructs the button to deselect all assets
						]
				]
	];
#pragma endregion SlateRegion
}

void SFolderCleaning::GetTypeOfAssets(TArray<TSharedPtr<FAssetData>> AssetData)
{
	TSet<FString> UniqueAssetNames; 
	for (const auto& AssetData : AssetData)
	{
		if (AssetData.IsValid())
		{
			FString AssetName = AssetData->AssetClassPath.GetAssetName().ToString();
	
			if (! UniqueAssetNames.Contains(AssetName))
			{
				UniqueAssetNames.Add(AssetName);							 
				ComboBoxAssetListItems.Add(MakeShared<FString>(AssetName));	 
			}
		}
	}
}


TSharedRef<SListView<TSharedPtr<FAssetData>>> SFolderCleaning::ConstructAssetListView()
{
	/* clang-format off */
	ConstructedAssetListView =
		SNew(SListView<TSharedPtr<FAssetData>>)
		.ItemHeight(24.0f)
		.ListItemsSource(&DisplayedAssetData)
		.OnGenerateRow(this, &SFolderCleaning::OnGenerateRowForList)
		.ToolTipText(FText::FromString(TEXT("Tool tip generated row for list ")))
		.SelectionMode(ESelectionMode::Multi)
		/*.OnSelectionChanged(this, &SFolderCleaning::OnAssetListChanged)*/;

	return ConstructedAssetListView.ToSharedRef();
}


TSharedRef<ITableRow> SFolderCleaning::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);

	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClassPath.GetAssetName().ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	const FString DisplayAssetPath = AssetDataToDisplay->AssetClassPath.GetPackageName().ToString();

	FSlateFontInfo AssetClassNameFont = GetEmboseedTextFont();
	AssetClassNameFont.Size = 10;

	FSlateFontInfo AssetNameFont = GetEmboseedTextFont();
	AssetNameFont.Size = 15;

#pragma region ListRowWidget
	// Create the row widget for the asset list
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(5.0f))
		.ToolTipText(FText::FromString(*DisplayAssetPath))
		[
			SNew(SHorizontalBox)
				// CheckBox for selecting the asset
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.FillWidth(0.05f)
				[
					ConstructCheckBox(AssetDataToDisplay)
				]

				// Slot for displaying the asset class name
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				.FillWidth(0.5f)
				[
					ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
				]

				// Slot for displaying the asset name
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				[
					ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
				]

				// Button to open the asset
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				.AutoWidth()
				[
					ConstructOpenAssetButtonForRowWidget(AssetDataToDisplay)
				]

				// Button to view references
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				.AutoWidth()
				[
					ConstructReferenceViewButtoWidget(AssetDataToDisplay)
				]

				// Button to generate size map
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				.AutoWidth()
				[
					ConstructSizeMapButtoWidget(AssetDataToDisplay)
				]

				// Button to delete the asset
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				.AutoWidth()
				[
					ConstructDeleteButtonForRowWidget(AssetDataToDisplay)
				]

				// Button to search the asset in the browser
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.AutoWidth()
				.FillWidth(.06f)
				[
					SearchAssetInBrowser(AssetDataToDisplay)
				]

		];

#pragma endregion ListRowWidget

	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SFolderCleaning::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SFolderCleaning::OnCheckBoxStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	CheckBoxArray.Add(ConstructedCheckBox);

	return ConstructedCheckBox;
}

TSharedRef<STextBlock> SFolderCleaning::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

void SFolderCleaning::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (AssetDataToDeleteArray.Contains(AssetData))
		{
			AssetDataToDeleteArray.Remove(AssetData);
		}
		break;

	case ECheckBoxState::Checked:
	{
		AssetDataToDeleteArray.AddUnique(AssetData);
	}
	break;

	case ECheckBoxState::Undetermined: break;

	default: break;
	}
}

TSharedRef<SComboBox<TSharedPtr<FString>>> SFolderCleaning::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSourceItems)
		.OnGenerateWidget(this, &SFolderCleaning::OnGenerateComboContent)
		.OnSelectionChanged(this, &SFolderCleaning::OnComboSelectionChange)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("List Assets Option")))
		];

	return ConstructedComboBox;
}


TSharedRef<SComboBox<TSharedPtr<FString>>> SFolderCleaning::ConstructAssetComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox =
	SNew(SComboBox<TSharedPtr<FString>>)
	.OptionsSource(&ComboBoxAssetListItems)
	.OnGenerateWidget(this, &SFolderCleaning::OnGenerateComboContent)
	.OnSelectionChanged(this, &SFolderCleaning::OnAssetSelectionChange)
	[
		SAssignNew(ComboAssetDisplayTextBlock, STextBlock)
			.Text(FText::FromString(TEXT(" Assets Types ")))
	];

	return ConstructedComboBox;
}

TSharedRef<STextBlock> SFolderCleaning::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmboseedTextFont();
	ButtonTextFont.Size = 15;

	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(ButtonTextFont)
		.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

TSharedRef<STextBlock> SFolderCleaning::ConstructComboHelpTexts(const FString& Text, ETextJustify::Type TextType)
{
	TSharedRef<STextBlock> ConstructHelpText =
		SNew(STextBlock)
		.Text(FText::FromString(Text))
		.Justification(TextType)
		.AutoWrapText(true);

	return ConstructHelpText;
}


void SFolderCleaning::OnComboSelectionChange(TSharedPtr<FString> SelecetedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromString(*SelecetedOption.Get()));

	FFolderCleanerModule& Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));

	if (*SelecetedOption.Get() == ListAllAssetsName)
	{
		DisplayedAssetData = StoredAssetsData;
		RefreshAssetListView();
	}
	else if (*SelecetedOption.Get() == ListUnusedAssetsName)
	{
		Module.ListUnusedAssetForAssetList(StoredAssetsData, DisplayedAssetData);
		RefreshAssetListView();
	}
	else if (*SelecetedOption.Get() == ListAssetWithSameName)
	{
		Module.ListSameNameAssetsForAssetList(StoredAssetsData, DisplayedAssetData);
		RefreshAssetListView();
	}
}

void SFolderCleaning::OnAssetSelectionChange(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	ComboAssetDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	if (SelectedOption.IsValid())
	{
		const FString SelectedAssetType = *SelectedOption;
		TArray<TSharedPtr<FAssetData>> FilteredAssetData;

		for (const TSharedPtr<FAssetData>& AssetData : StoredAssetsData)
		{
			if (AssetData.IsValid())
			{
				
				const FString AssetClassName = AssetData->AssetClassPath.GetAssetName().ToString();
				if (AssetClassName == SelectedAssetType)
				{
					FilteredAssetData.Add(AssetData); 
				}
			}
		}

		DisplayedAssetData = FilteredAssetData;
		RefreshAssetListView();
	}
}

void SFolderCleaning::RefreshAssetListView()
{
	AssetDataToDeleteArray.Empty();
	CheckBoxArray.Empty();

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

TSharedRef<SButton> SFolderCleaning::ConstructDeleteButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Delete")))
		.OnClicked(this, &SFolderCleaning::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructReferenceViewButtoWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Ref View")))
		.OnClicked(this, &SFolderCleaning::OnRefViewButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructSizeMapButtoWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("SizeMap")))
		.OnClicked(this, &SFolderCleaning::OnSizeMapButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructOpenAssetButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Open")))
		.OnClicked(this, &SFolderCleaning::OnOpenAssetButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.OnClicked(this, &SFolderCleaning::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete Selected")));

	return DeleteAllButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.OnClicked(this, &SFolderCleaning::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.OnClicked(this, &SFolderCleaning::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructDeleteAllEmtpyFolderButton()
{
	TSharedRef<SButton> DeleteAllEmptyFolderButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.ToolTipText(FText::FromString(TEXT(" Clears the given directory of empty folders and subfolders  ")))
		.OnClicked(this, &SFolderCleaning::OnDeselectAllEmptyFolderButtonClicked);

	DeleteAllEmptyFolderButton->SetContent(ConstructTextForTabButtons(TEXT("Clear Empty Folder")));

	return DeleteAllEmptyFolderButton;
}


TSharedRef<SButton> SFolderCleaning::RefreshListAssets()
{
	TSharedRef<SButton> RefreshList =
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ContentPadding(FMargin(5.0f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.ToolTipText(FText::FromString(TEXT(" Refresh list ")))
		.OnClicked(this, &SFolderCleaning::OnRefreshListAssets);

	RefreshList->SetContent(ConstructTextForTabButtons(TEXT("Refresh Asset List")));

	return RefreshList;
}

TSharedRef<SButton> SFolderCleaning::SearchAssetInBrowser(TSharedPtr<FAssetData>& AssetData)
{
	TSharedRef<SButton> SearchAssetButton =
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.ContentPadding(FMargin(2))
		.ToolTipText(FText::FromString(TEXT(" Find asset in content browser ")))
		.OnClicked_Lambda([AssetData]()
			{
				const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				ContentBrowserModule.Get().SyncBrowserToAssets({ *AssetData });
				return FReply::Handled();
			})
		[
			SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("SystemWideCommands.FindInContentBrowser.Small"))
		];

	return SearchAssetButton;
}


TSharedRef<SWidget> SFolderCleaning::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText =
		SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));

	return ConstructedComboText;
}

FReply SFolderCleaning::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	if (!ClickedAssetData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Asset Data"));
		return FReply::Handled();
	}

	FFolderCleanerModule& Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));
	const bool bAssetDeleted = Module.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		if (StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}
		if (DisplayedAssetData.Contains(ClickedAssetData))
		{
			DisplayedAssetData.Remove(ClickedAssetData);
		}

		RefreshAssetListView();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to delete asset: %s"), *ClickedAssetData.Get()->AssetName.ToString());
	}

	return FReply::Handled();
}

FReply SFolderCleaning::OnRefViewButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	if (!ClickedAssetData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Asset Data"));
		return FReply::Handled();
	}

	FFolderCleanerModule& Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(*ClickedAssetData.Get());

	Module.OnReferenceViewerButtonClicked(AssetDataArray);

	return FReply::Handled();
}

FReply SFolderCleaning::OnSizeMapButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	if (!ClickedAssetData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Asset Data"));
		return FReply::Handled();
	}

	FFolderCleanerModule& Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(*ClickedAssetData.Get());

	Module.OnSizeMapButtonClicked(AssetDataArray);

	return FReply::Handled();
}

FReply SFolderCleaning::OnOpenAssetButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FFolderCleanerModule& Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));
	const FAssetData* AssetData = ClickedAssetData.Get();
	const bool bAssetDeleted = Module.OpenAsset(*AssetData);

	return FReply::Handled();
}

FReply SFolderCleaning::OnDeleteAllButtonClicked()
{
	if (AssetDataToDeleteArray.Num() == 0)
	{
		FolderCleaner::ShowMessageDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetDataToDelete;

	for (const TSharedPtr<FAssetData>& Data : AssetDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	FFolderCleanerModule Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));
	const bool bAssetDeleted = Module.DeleteMultipleAssetsForAsssetList(AssetDataToDelete);

	if (bAssetDeleted)
	{
		for (const TSharedPtr<FAssetData>& Data : AssetDataToDeleteArray)
		{
			if (StoredAssetsData.Contains(Data))
			{
				StoredAssetsData.Remove(Data);
			}

			if (DisplayedAssetData.Contains(Data))
			{
				DisplayedAssetData.Remove(Data);
			}
		}

		RefreshAssetListView();
	}

	return FReply::Handled();
}

FReply SFolderCleaning::OnSelectAllButtonClicked()
{
	if (CheckBoxArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

FReply SFolderCleaning::OnDeselectAllButtonClicked()
{
	if (CheckBoxArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

FReply SFolderCleaning::OnDeselectAllEmptyFolderButtonClicked()
{
	FolderCleaner::ShowNotifyInfo(" Delete Empty Folder ");

	FFolderCleanerModule Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));
	Module.OnDeleteEmptyFolderButtonClicked();

	return FReply::Handled();
}

FReply SFolderCleaning::OnRefreshListAssets()
{
	FolderCleaner::ShowNotifyInfo(" Refresh List View ");
	RefreshAssetListView();
	return FReply::Handled();
}

