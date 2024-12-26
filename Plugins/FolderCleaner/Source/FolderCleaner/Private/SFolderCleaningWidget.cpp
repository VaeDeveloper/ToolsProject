// Fill out your copyright notice in the Description page of Project Settings.

#include "SFolderCleaningWidget.h"
#include "SlateBasics.h"
#include "FolderCleaner.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SToolTip.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

namespace FolderCleaner
{
	struct FAssetLists
	{
		static const FName AllAssets;
		static const FName UnusedAssets;
		static const FName SameNameAssets;
	};

	const FName FAssetLists::AllAssets = TEXT("All Available Assets");
	const FName FAssetLists::UnusedAssets = TEXT("List of Unused Assets");
	const FName FAssetLists::SameNameAssets = TEXT("List Assets With The Same Name");

	FName FolderCModuleName = TEXT("FolderCleaner");

	static constexpr float TitleInfoFontTextSize = 15.0f;
	static const FString TitleTextColor = "2A6FFFFF";

	static FFolderCleanerModule& GetFolderModule()
	{
		return FModuleManager::LoadModuleChecked<FFolderCleanerModule>(FolderCModuleName);
	}
}


void SFolderCleaning::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetList = InArgs._AssetDataToStore;
	VisibleAssetsList = StoredAssetList;
	CheckBoxList.Empty();
	DeletionAssetList.Empty();

	ComboBoxItemList.Add(MakeShared<FString>(FolderCleaner::FAssetLists::AllAssets.ToString()));
	ComboBoxItemList.Add(MakeShared<FString>(FolderCleaner::FAssetLists::UnusedAssets.ToString()));
	ComboBoxItemList.Add(MakeShared<FString>(FolderCleaner::FAssetLists::SameNameAssets.ToString()));

	GetTypeOfAssets(InArgs._AssetDataToStore);

#pragma region SlateRegion

	FSlateFontInfo TitleTextFontInfo = GetEmboseedTextFont();
	TitleTextFontInfo.Size = FolderCleaner::TitleInfoFontTextSize;

	/* clang-format off */
	ChildSlot[
		SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSeparator)
				]
				
				// Title of the widget
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.AutoWrapText(true)
						.Text(FText::FromString(TEXT("Folder Cleaner - Designed to optimize and manage folders and assets in a project")))
						.Font(TitleTextFontInfo)
						.Justification(ETextJustify::InvariantLeft)
						.ColorAndOpacity(FColor::FromHex(FolderCleaner::TitleTextColor))
				]
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0, 5)) 
				[
					SNew(SSeparator)
				]

				// Combo Box for asset selection
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.FillWidth(0.1f)
						.AutoWidth()
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("Refresh")))
							.ContentPadding(FMargin(5.0f))
							.Cursor(EMouseCursor::Hand)
							.ToolTipText(FText::FromString(TEXT("Performs the function of updating data in the Folder Cleaner interface.")))
							.OnClicked(this, &SFolderCleaning::OnRefreshButtonClicked)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							ConstructComboBox()
						]

						+ SHorizontalBox::Slot()
						.FillWidth(0.1f)
						.AutoWidth()
						[
							ConstructAssetComboBox()
						]

						+SHorizontalBox::Slot()
						.FillWidth(0.1f)
						.AutoWidth()
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("Delete Empty Folders")))
							.ContentPadding(FMargin(5.0f))
							.Cursor(EMouseCursor::Hand)
							.ToolTipText(FText::FromString(TEXT(" ")))
							.OnClicked(this, &SFolderCleaning::OnDeleteEmptyFolderButtonClicked)
						]

						+SHorizontalBox::Slot()
						.FillWidth(0.1f)
						.AutoWidth()
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("Update Redirectors")))
							.ContentPadding(FMargin(5.0f))
							.Cursor(EMouseCursor::Hand)
							.ToolTipText(FText::FromString(TEXT(" ")))
							.OnClicked(this, &SFolderCleaning::OnFixupRedirectorsButtonClicked)
						]


						// Current folder display
						+ SHorizontalBox::Slot()
						.FillWidth(0.1f)
						[
							ConstructComboHelpTexts(TEXT("Current Folder: \n") + InArgs._CurrentSelectedFolder, ETextJustify::Right)
						]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0, 5)) 
				[
					SNew(SSeparator)
				]

				// Scrollable asset list view
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				[
					SNew(SScrollBox) 
						+ SScrollBox::Slot()
						[
							ConstructAssetListView()
						]
				]
			
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0, 5)) 
				[
					SNew(SSeparator)
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

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0, 5)) 
				[
					SNew(SSeparator)
				]
	];
#pragma endregion SlateRegion
}

void SFolderCleaning::GetTypeOfAssets(TArray<TSharedPtr<FAssetData>> AssetDatas)
{
	TSet<FString> UniqueAssetNames; 
	for (const auto& AssetData : AssetDatas)
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
		.ListItemsSource(&VisibleAssetsList)
		.OnGenerateRow(this, &SFolderCleaning::OnGenerateRowForList)
		.SelectionMode(ESelectionMode::Single);

	return ConstructedAssetListView.ToSharedRef();
}

TSharedRef<ITableRow> SFolderCleaning::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);

	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClassPath.GetAssetName().ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	const FString FullAssetPath = AssetDataToDisplay->ObjectPath.ToString();
	const FString AssetFolderPath = FPackageName::GetLongPackagePath(FullAssetPath);

	FSlateFontInfo ToolTipFont = GetEmboseedTextFont();
	ToolTipFont.Size = 5;

	FSlateFontInfo AssetClassNameFont = GetEmboseedTextFont();
	AssetClassNameFont.Size = 10;

	FSlateFontInfo AssetNameFont = GetEmboseedTextFont();
	AssetNameFont.Size = 15;
	
#pragma region ListRowWidget
	// Create the row widget for the asset list
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(5.0f))
		.ToolTip(
				SNew(SToolTip)
				.RenderOpacity(0.5f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(*AssetFolderPath))
						.Font(AssetClassNameFont)
						.ColorAndOpacity(FLinearColor::Green)
                    ]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0, 5))
					[
					    SNew(SSeparator)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(*DisplayAssetClassName))
						.Font(AssetClassNameFont)
						.ColorAndOpacity(FLinearColor::White)
						.AutoWrapText(true)
					]
				])

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
					ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont, FColor::Emerald)
				]

				// Slot for displaying the asset name
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				
				[
					ConstructTextForRowWidget(DisplayAssetName, AssetNameFont, FColor::White)
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
				.FillWidth(0.06f)
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

	CheckBoxList.Add(ConstructedCheckBox);

	return ConstructedCheckBox;
}

TSharedRef<STextBlock> SFolderCleaning::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse, const FColor& Color)
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(Color);

	return ConstructedTextBlock;
}

void SFolderCleaning::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
	{
		if (DeletionAssetList.Contains(AssetData))
		{
			DeletionAssetList.Remove(AssetData);
		}
		break;
	}
	case ECheckBoxState::Checked:
	{
		DeletionAssetList.AddUnique(AssetData);
	}
	break;

	default: 
		break;
	}
}

TSharedRef<SComboBox<TSharedPtr<FString>>> SFolderCleaning::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxItemList)
		.OnGenerateWidget(this, &SFolderCleaning::OnGenerateComboContent)
		.OnSelectionChanged(this, &SFolderCleaning::OnComboSelectionChange)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
				.Text(FText::FromString(TEXT(" List Assets Option ")))
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

	FFolderCleanerModule& Module = FolderCleaner::GetFolderModule();

	if (*SelecetedOption.Get() == FolderCleaner::FAssetLists::AllAssets)
	{
		VisibleAssetsList = StoredAssetList;
		RefreshAssetListView();
	}
	else if (*SelecetedOption.Get() == FolderCleaner::FAssetLists::UnusedAssets)
	{
		Module.FilterUnusedAssets(StoredAssetList, VisibleAssetsList);
		RefreshAssetListView();
	}
	else if (*SelecetedOption.Get() == FolderCleaner::FAssetLists::SameNameAssets)
	{
		Module.FilterDuplicateAssets(StoredAssetList, VisibleAssetsList);
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

		for (const TSharedPtr<FAssetData>& AssetData : StoredAssetList)
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

		VisibleAssetsList = FilteredAssetData;
		RefreshAssetListView();
	}
}

void SFolderCleaning::RefreshAssetListView()
{
	DeletionAssetList.Empty();
	CheckBoxList.Empty();

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
		.Cursor(EMouseCursor::Hand)
		.ToolTipText(FText::FromString(TEXT("Delete Asset")))
		.OnClicked(this, &SFolderCleaning::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructReferenceViewButtoWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Ref View")))
		.Cursor(EMouseCursor::Hand)
		.ToolTipText(FText::FromString(TEXT("Open Reference Viewer Window ")))
		.OnClicked(this, &SFolderCleaning::OnRefViewButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructSizeMapButtoWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("SizeMap")))
		.Cursor(EMouseCursor::Hand)
		.ToolTipText(FText::FromString(TEXT("Open Size Map with Asset ")))
		.OnClicked(this, &SFolderCleaning::OnSizeMapButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructOpenAssetButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Open")))
		.Cursor(EMouseCursor::Hand)
		.ToolTipText(FText::FromString(TEXT(" Open the asset in a separate window ")))
		.OnClicked(this, &SFolderCleaning::OnOpenAssetButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.Cursor(EMouseCursor::Hand)
		.ToolTipText(FText::FromString(TEXT(" Delete All Selected Assets ")))
		.OnClicked(this, &SFolderCleaning::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete Selected")));

	return DeleteAllButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructSelectAllButton()
{
	const TSharedRef<SButton> SelectAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.Cursor(EMouseCursor::Hand)
		.ToolTipText(FText::FromString(TEXT(" Select All Asset for delete ")))
		.OnClicked(this, &SFolderCleaning::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructDeselectAllButton()
{
	const TSharedRef<SButton> DeselectAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.Cursor(EMouseCursor::Hand)
		.ToolTipText(FText::FromString(TEXT(" Deselect all assets ")))
		.OnClicked(this, &SFolderCleaning::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

TSharedRef<SButton> SFolderCleaning::ConstructDeleteAllEmtpyFolderButton()
{
	const TSharedRef<SButton> DeleteAllEmptyFolderButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.ToolTipText(FText::FromString(TEXT(" Clears the given directory of empty folders and subfolders  ")))
		.OnClicked(this, &SFolderCleaning::OnDeselectAllEmptyFolderButtonClicked);

	DeleteAllEmptyFolderButton->SetContent(ConstructTextForTabButtons(TEXT("Clear Empty Folder")));

	return DeleteAllEmptyFolderButton;
}


TSharedRef<SButton> SFolderCleaning::RefreshListAssets()
{
	const TSharedRef<SButton> RefreshList =
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
	const TSharedRef<SButton> SearchAssetButton =
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
	const TSharedRef<STextBlock> ConstructedComboText =
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

	FFolderCleanerModule& Module = FolderCleaner::GetFolderModule();
	const bool bAssetDeleted = Module.DeleteSingleAsset(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		if (StoredAssetList.Contains(ClickedAssetData))
		{
			StoredAssetList.Remove(ClickedAssetData);
		}
		if (VisibleAssetsList.Contains(ClickedAssetData))
		{
			VisibleAssetsList.Remove(ClickedAssetData);
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

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(*ClickedAssetData.Get());

	FolderCleaner::GetFolderModule().OnReferenceViewerButtonClicked(AssetDataArray);

	return FReply::Handled();
}

FReply SFolderCleaning::OnSizeMapButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	if (!ClickedAssetData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Asset Data"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(*ClickedAssetData.Get());

	FolderCleaner::GetFolderModule().OnSizeMapButtonClicked(AssetDataArray);

	return FReply::Handled();
}

FReply SFolderCleaning::OnOpenAssetButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	const FAssetData* AssetData = ClickedAssetData.Get();
	const bool bAssetDeleted = FolderCleaner::GetFolderModule().OpenAsset(*AssetData);

	return FReply::Handled();
}

FReply SFolderCleaning::OnDeleteAllButtonClicked()
{
	if (DeletionAssetList.Num() == 0)
	{
		FolderCleaner::ShowMessageDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetDataToDelete;

	for (const TSharedPtr<FAssetData>& Data : DeletionAssetList)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	const bool bAssetDeleted = FolderCleaner::GetFolderModule().DeleteMultiplyAsset(AssetDataToDelete);

	if (bAssetDeleted)
	{
		for (const TSharedPtr<FAssetData>& Data : DeletionAssetList)
		{
			if (StoredAssetList.Contains(Data))
			{
				StoredAssetList.Remove(Data);
			}

			if (VisibleAssetsList.Contains(Data))
			{
				VisibleAssetsList.Remove(Data);
			}
		}

		RefreshAssetListView();
	}

	return FReply::Handled();
}

FReply SFolderCleaning::OnSelectAllButtonClicked()
{
	if (CheckBoxList.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxList)
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
	if (CheckBoxList.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxList)
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
	FolderCleaner::GetFolderModule().OnDeleteEmptyFolderButtonClicked();

	return FReply::Handled();
}

FReply SFolderCleaning::OnRefreshListAssets()
{
	FolderCleaner::ShowNotifyInfo(" Refresh List View ");
	RefreshAssetListView();
	return FReply::Handled();
}

FReply SFolderCleaning::OnRefreshButtonClicked()
{
	ComboDisplayTextBlock->SetText(FText::FromString(" List Assets Option "));
	ComboAssetDisplayTextBlock->SetText(FText::FromString(" Assets Types "));
	VisibleAssetsList = StoredAssetList;
	RefreshAssetListView();

	return FReply::Handled();
}

FReply SFolderCleaning::OnDeleteEmptyFolderButtonClicked()
{
	FolderCleaner::GetFolderModule().OnDeleteEmptyFolderButtonClicked();

	return FReply::Handled();
}

FReply SFolderCleaning::OnFixupRedirectorsButtonClicked()
{
	FolderCleaner::GetFolderModule().FixupRedirectors();
	return FReply::Handled();
}

