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

#define ListAllAssets TEXT("All Available Assets")
#define UnusedAssets TEXT("Unused Assets")
#define AssetWithSameName TEXT("List Assets With Same Name")

/**
 * @brief Constructs the SFolderCleaning widget.
 *
 * This function initializes the SFolderCleaning widget with the provided arguments. It sets up
 * various member variables, constructs the UI components, and lays out the structure of the
 * folder cleaning interface, including a title, a combo box for asset selection, a scrollable
 * asset list view, and control buttons for deleting, selecting, and deselecting assets.
 *
 * @param InArgs The arguments used to initialize the widget, including the asset data to store
 *               and the currently selected folder.
 */
void SFolderCleaning::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetsData = InArgs._AssetDataToStore;
	DisplayedAssetData = StoredAssetsData;
	CheckBoxArray.Empty();
	AssetDataToDeleteArray.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(ListAllAssets));
	ComboBoxSourceItems.Add(MakeShared<FString>(UnusedAssets));
	ComboBoxSourceItems.Add(MakeShared<FString>(AssetWithSameName));

	FSlateFontInfo TitleTextFontInfo = GetEmboseedTextFont();
	TitleTextFontInfo.Size = 30;

	ChildSlot
		[
			SNew(SVerticalBox)

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
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							ConstructComboBox()
						]


						//TODO в будущем надо вынести эту кнопку в отдельное окно, либо сделать ее сверху возле директории 

						//+ SHorizontalBox::Slot()
						//.FillWidth(10.0f)
						//.Padding(5.0f)
						//[
						//    ConstructDeleteAllEmtpyFolderButton()
						//]

						// Help text for the combo box selection
						+SHorizontalBox::Slot()
						.FillWidth(0.6f)
						[
							ConstructComboHelpTexts(TEXT("Specify the listing condition in the drop. Left mouse click to go to where asset"), ETextJustify::Center)
						]

						// Current folder display
						+ SHorizontalBox::Slot()
						.FillWidth(0.1f)
						[
							ConstructComboHelpTexts(TEXT("Current Folder: \n") + InArgs._CurrentSelectedFolder, ETextJustify::Right)
						]
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

				// Control buttons: Delete All, Select All, Deselect All
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(10.0f)
						.Padding(5.0f)
						[
							ConstructDeleteAllButton() // Constructs the button to delete all selected assets
						]

						+ SHorizontalBox::Slot()
						.FillWidth(10.0f)
						.Padding(5.0f)
						[
							ConstructSelectAllButton() // Constructs the button to select all assets
						]

						+ SHorizontalBox::Slot()
						.FillWidth(10.0f)
						.Padding(5.0f)
						[
							ConstructDeselectAllButton()// Constructs the button to deselect all assets
						]
				]
		];
}



/**
 * @brief Constructs the asset list view for displaying assets.
 *
 * This function creates and returns a list view widget that displays asset data. The list view
 * is configured with item height, a source of displayed asset data, and a callback for generating
 * each row in the list. It also sets a tooltip for the list view.
 *
 * @return A shared reference to the SListView widget that displays the asset data.
 */
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


/**
 * @brief Generates a row for the asset list view.
 *
 * This function creates and returns a table row widget for the specified asset data. It configures
 * the layout of the row to display the asset class name, asset name, and buttons for various actions,
 * such as opening the asset, viewing references, generating a size map, and deleting the asset.
 *
 * @param AssetDataToDisplay A shared pointer to the asset data to display in the row.
 * @param OwnerTable A reference to the owner table that contains this row.
 * @return A shared reference to the STableRow widget representing the asset data.
 */
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


/**
 * @brief Constructs a checkbox for the specified asset data.
 *
 * This function creates a checkbox widget that allows the user to select or deselect the asset
 * associated with the provided asset data. It sets up a callback for state changes and adds the
 * checkbox to an internal array for tracking.
 *
 * @param AssetDataToDisplay A shared pointer to the asset data associated with the checkbox.
 * @return A shared reference to the constructed SCheckBox widget.
 */
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

/**
 * Constructs a text block widget for displaying text within a row of the list view.
 *
 * @param TextContent The string content to be displayed in the text block.
 * @param FontToUse The font to be used for the text.
 * @return A shared reference to the constructed STextBlock widget.
 *
 * This method creates a text block with the specified content and font.
 * The text block is used to display asset information such as the asset name or class name in the list view.
 */
TSharedRef<STextBlock> SFolderCleaning::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

/**
 * Handles the state change event for checkboxes in the asset list.
 *
 * @param NewState The new state of the checkbox (Unchecked, Checked, or Undetermined).
 * @param AssetData A shared pointer to the asset data associated with the checkbox.
 *
 * This method updates the array of assets marked for deletion based on the checkbox's new state.
 * If the checkbox is unchecked, the associated asset is removed from the deletion list.
 * If the checkbox is checked, the asset is added to the deletion list, ensuring no duplicates.
 */
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

/**
 * Constructs a combo box widget for the advanced deletion tab.
 * The combo box uses a predefined list of options and provides functionality for generating its content
 * and handling selection changes.
 *
 * @return A shared reference to the constructed SComboBox widget.
 */
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

/**
 * Constructs a text block widget for tab buttons, with customized font and center justification.
 * The font size is set to 15.
 *
 * @param TextContent The text content to display within the text block.
 * @return A shared reference to the constructed STextBlock widget.
 */
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

/**
 * @brief Constructs a help text block with the specified text and justification.
 *
 * This function creates a text block widget that displays help or informational text. It allows
 * for automatic text wrapping and sets the justification based on the provided parameters.
 *
 * @param Text The text to be displayed in the help text block.
 * @param TextType The justification of the text (e.g., left, center, right).
 * @return A shared reference to the constructed STextBlock widget.
 */
TSharedRef<STextBlock> SFolderCleaning::ConstructComboHelpTexts(const FString& Text, ETextJustify::Type TextType)
{
	TSharedRef<STextBlock> ConstructHelpText =
		SNew(STextBlock)
		.Text(FText::FromString(Text))
		.Justification(TextType)
		.AutoWrapText(true);

	return ConstructHelpText;
}


/**
 * Handles the selection change event for the combo box in the deletion tab.
 *
 * @param SelectedOption A shared pointer to the string representing the newly selected option.
 * @param InSelectInfo The type of selection (e.g., directly selected by the user or programmatically changed).
 *
 * This method updates the display text of the combo box and refreshes the asset list view based on the selected option.
 * It categorizes assets into three groups: all assets, unused assets, and assets with the same name.
 * The asset list view is refreshed to display the selected category of assets.
 */
void SFolderCleaning::OnComboSelectionChange(TSharedPtr<FString> SelecetedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromString(*SelecetedOption.Get()));

	FFolderCleanerModule& Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));

	if (*SelecetedOption.Get() == ListAllAssets)
	{
		DisplayedAssetData = StoredAssetsData;
		RefreshAssetListView();
	}
	else if (*SelecetedOption.Get() == UnusedAssets)
	{
		Module.ListUnusedAssetForAssetList(StoredAssetsData, DisplayedAssetData);
		RefreshAssetListView();
	}
	else if (*SelecetedOption.Get() == AssetWithSameName)
	{
		Module.ListSameNameAssetsForAssetList(StoredAssetsData, DisplayedAssetData);
		RefreshAssetListView();
	}
}

/**
 * Refreshes the asset list view by clearing the current data and rebuilding the list.
 *
 * This method clears the arrays that store the data for assets marked for deletion and the checkboxes.
 * It then checks if the asset list view is valid and triggers a rebuild of the list view to reflect any changes.
 */
void SFolderCleaning::RefreshAssetListView()
{
	AssetDataToDeleteArray.Empty();
	CheckBoxArray.Empty();

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

/**
 * Constructs a button widget for deleting an asset from the list view.
 *
 * @param AssetDataToDisplay A shared pointer to the asset data associated with this button.
 * @return A shared reference to the constructed SButton widget.
 *
 * This method creates a delete button and binds its click event to the appropriate handler.
 * The button is used within each row of the list view to allow users to delete specific assets.
 */
TSharedRef<SButton> SFolderCleaning::ConstructDeleteButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Delete")))
		.OnClicked(this, &SFolderCleaning::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

/**
 * @brief Creates a button for displaying the reference view of the specified asset.
 *
 * This function constructs and returns a button labeled "Ref View", which, when clicked,
 * will open the reference viewer for the asset represented by the given AssetDataToDisplay.
 *
 * @param AssetDataToDisplay A shared pointer to the asset data that will be displayed in the reference view.
 * @return A shared reference to the SButton widget for opening the reference view.
 */
TSharedRef<SButton> SFolderCleaning::ConstructReferenceViewButtoWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Ref View")))
		.OnClicked(this, &SFolderCleaning::OnRefViewButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

/**
 * @brief Creates a button for displaying the size map of the specified asset.
 *
 * This function constructs and returns a button labeled "SizeMap", which, when clicked,
 * will open the size map view for the asset represented by the given AssetDataToDisplay.
 *
 * @param AssetDataToDisplay A shared pointer to the asset data that will be displayed in the size map view.
 * @return A shared reference to the SButton widget for opening the size map view.
 */
TSharedRef<SButton> SFolderCleaning::ConstructSizeMapButtoWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("SizeMap")))
		.OnClicked(this, &SFolderCleaning::OnSizeMapButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

/**
 * Constructs a button widget labeled "Open" for a row widget in the advanced deletion tab.
 * When clicked, the button triggers the `OnOpenAssetButtonClicked` method, passing the specified asset data.
 *
 * @param AssetDataToDisplay A shared pointer to the asset data to be displayed when the button is clicked.
 * @return A shared reference to the constructed SButton widget.
 */
TSharedRef<SButton> SFolderCleaning::ConstructOpenAssetButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Open")))
		.OnClicked(this, &SFolderCleaning::OnOpenAssetButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

/**
 * Constructs a "Delete All" button for the advanced deletion tab.
 *
 * @return A shared reference to the constructed SButton widget.
 *
 * This method creates a button that allows users to delete all selected assets.
 * The button's content is set to "Delete All" and its click event is bound to the appropriate handler.
 */
TSharedRef<SButton> SFolderCleaning::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.OnClicked(this, &SFolderCleaning::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete Selected")));

	return DeleteAllButton;
}

/**
 * Handles the click event for the "Delete All" button.
 *
 * @return A value of type FReply indicating the handling status of the event.
 *
 * This method removes all assets currently marked for deletion from the list view.
 * The list of assets to delete is cleared and the list view is refreshed.
 */
TSharedRef<SButton> SFolderCleaning::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.OnClicked(this, &SFolderCleaning::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

/**
 * Handles the click event for individual delete buttons in the asset list view.
 *
 * @param AssetDataToDelete A shared pointer to the asset data associated with the delete button.
 * @return A value of type FReply indicating the handling status of the event.
 *
 * This method removes the selected asset from the list of displayed assets and updates the list view.
 */
TSharedRef<SButton> SFolderCleaning::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.OnClicked(this, &SFolderCleaning::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

/**
 * Constructs a button widget labeled "Clear Empty Folder" for the advanced deletion tab.
 * This button has a content padding of 5.0f, displays a tooltip explaining its purpose,
 * and triggers the `OnDeselectAllEmptyFolderButtonClicked` method when clicked.
 *
 * @return A shared reference to the constructed SButton widget that clears all empty folders and subfolders.
 */
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


/**
 * @brief Creates a button to refresh the asset list in the folder cleaning UI.
 *
 * This function constructs and returns a button widget that, when clicked, refreshes the list of assets
 * in the folder cleaning view. The button is styled with a simple look, aligned both horizontally and vertically
 * in the center, and has a tooltip explaining its function.
 *
 * @return A shared reference to the SButton widget that refreshes the asset list.
 */
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

/**
 * Searches for the specified asset in the content browser.
 *
 * @param AssetDataToDisplay A shared pointer to the asset data to be searched.
 * @return A shared reference to the constructed SButton widget.
 *
 * This method creates a "Find in Browser" button that allows users to locate the asset in the content browser.
 * The button's click event is bound to trigger the search in the browser.
 */
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


/**
 * Generates the content for each item in the combo box by creating a text block for the given source item.
 *
 * @param SourceItem A shared pointer to the string representing the content of a combo box item.
 * @return A shared reference to the constructed SWidget containing the text block for the combo box item.
 */
TSharedRef<SWidget> SFolderCleaning::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText =
		SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));

	return ConstructedComboText;
}

/**
 * Handles the click event for the delete button associated with an asset.
 * Deletes the selected asset from the list if the deletion is successful and refreshes the asset list view.
 *
 * @param ClickedAssetData A shared pointer to the asset data that was clicked.
 * @return An FReply object indicating that the event was handled.
 */
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

/**
 * @brief Handles the click event for the reference view button.
 *
 * This function is triggered when the "Ref View" button is clicked. It checks if the asset data
 * is valid, and if so, it loads the FolderCleaner module and triggers the reference viewer
 * for the specified asset.
 *
 * @param ClickedAssetData A shared pointer to the asset data that was clicked.
 * @return An FReply indicating that the event was handled.
 */
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

/**
 * @brief Handles the click event for the size map button.
 *
 * This function is triggered when the "SizeMap" button is clicked. It checks if the asset data
 * is valid, and if so, it loads the FolderCleaner module and triggers the size map view
 * for the specified asset.
 *
 * @param ClickedAssetData A shared pointer to the asset data that was clicked.
 * @return An FReply indicating that the event was handled.
 */
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

/**
 * Handles the click event for the "Open" asset button in the advanced deletion tab.
 * This method displays a notification indicating that the asset has been opened.
 *
 * @param ClickedAssetData A shared pointer to the asset data that was clicked.
 * @return An FReply object signaling that the event was successfully handled.
 */
FReply SFolderCleaning::OnOpenAssetButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FFolderCleanerModule& Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));
	const FAssetData* AssetData = ClickedAssetData.Get();
	const bool bAssetDeleted = Module.OpenAsset(*AssetData);

	return FReply::Handled();
}

/**
 * Handles the event when the "Delete All" button is clicked.
 *
 * This method checks if there are any assets selected for deletion. If no assets are selected,
 * it shows a message dialog informing the user. If assets are selected, it proceeds to delete
 * them by using the Automation module. After successful deletion, it updates the stored and
 * displayed asset lists and refreshes the asset list view.
 *
 * @return FReply::Handled() to indicate the event was handled.
 */
FReply SFolderCleaning::OnDeleteAllButtonClicked()
{
	if (AssetDataToDeleteArray.Num() == 0)
	{
		Automation::ShowMessageDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
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

/**
 * Handles the event when the "Select All" button is clicked.
 *
 * This method iterates through all checkboxes and selects them if they are not already selected.
 *
 * @return FReply::Handled() to indicate the event was handled.
 */
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

/**
 * Handles the event when the "Deselect All" button is clicked.
 *
 * This method iterates through all checkboxes and deselects them if they are currently selected.
 *
 * @return FReply::Handled() to indicate the event was handled.
 */
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

/**
 * Handles the event when the "Deselect All Empty Folders" button is clicked.
 *
 * This method simply shows a notification indicating that empty folders have been deleted.
 *
 * @return FReply::Handled() to indicate the event was handled.
 */
FReply SFolderCleaning::OnDeselectAllEmptyFolderButtonClicked()
{
	Automation::ShowNotifyInfo(" Delete Empty Folder ");

	FFolderCleanerModule Module = FModuleManager::LoadModuleChecked<FFolderCleanerModule>(TEXT("FolderCleaner"));
	Module.OnDeleteEmptyFolderButtonClicked();

	return FReply::Handled();
}

/**
 * @brief Handles the click event for the "Refresh List" button.
 *
 * This function is triggered when the "Refresh List" button is clicked. It displays a notification
 * informing the user that the list is being refreshed, and then updates the asset list view.
 *
 * @return An FReply indicating that the event was handled.
 */
FReply SFolderCleaning::OnRefreshListAssets()
{
	Automation::ShowNotifyInfo(" Refresh List View ");
	RefreshAssetListView();
	return FReply::Handled();
}