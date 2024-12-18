// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Misc/MessageDialog.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

DEFINE_LOG_CATEGORY_STATIC(FolderCleanerLog, All, All);

namespace FolderCleaner
{
	/**
	 * @brief The duration in seconds for which a notification will fade out.
	 */
	constexpr float NotificationFadeOutDuration = 7.0f;

	static void PrintGEngineScreen(const FString& Message, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 9.0f, Color, Message);
		}
	}

	static void PrintLogDebug(const FString& Message)
	{
		UE_LOG(FolderCleanerLog, Display, TEXT("AutomationLog message : %s"), *Message);
	}

	static void PrintScreenDebug(FString Method, uint32 Counter)
	{
		FString MessageStr = FString(".... No matching files found ....");
		FColor MessageColor = Counter > 0 ? FColor::Green : FColor::Red;

		if (Counter > 0)
		{
			MessageStr = Method.AppendChar(' ');
			MessageStr.AppendInt(Counter);
			MessageStr.Append(Counter == 1 ? TEXT(" File") : TEXT(" Files"));
		}

		PrintGEngineScreen(MessageStr, MessageColor);
	}

	/**
	 * @brief Displays a message dialog with a specified message and an optional warning title.
	 *
	 * @param MessageType The type of message dialog to be shown (e.g., Ok, YesNo, etc.).
	 * @param Message The message text to be displayed in the dialog.
	 * @param bShowMsgAsWarning If true, the dialog will display a warning title. Defaults to true.
	 * @return EAppReturnType::Type The return type of the dialog, which indicates which button the user clicked.
	 */
	static EAppReturnType::Type ShowMessageDialog(EAppMsgType::Type MessageType, const FString& Message, bool bShowMsgAsWarning = true)
	{
		//
		if (bShowMsgAsWarning)
		{
			FText MessageTitle = FText::FromString(TEXT("Warning"));
			return FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), &MessageTitle);
		}
		else
		{
			return FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
		}
	}

	/**
	 * @brief Displays a notification with a specified message.
	 *
	 * The notification uses a large font and will fade out after a duration defined by NotificationFadeOutDuration.
	 *
	 * @param Message The message text to be displayed in the notification.
	 */
	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo Info(FText::FromString(Message));
		Info.bUseLargeFont = true;
		Info.FadeOutDuration = NotificationFadeOutDuration;

		FSlateNotificationManager::Get().AddNotification(Info);
	}

}

/**
 * @class SFolderCleaning
 * @brief A widget for managing and cleaning up assets in a specified folder.
 *
 * This widget provides functionality for displaying asset data, selecting assets, and performing
 * actions such as deleting, refreshing, and viewing references of assets. It includes a combo box
 * for filtering assets and a scrollable list for displaying asset details.
 */
class SFolderCleaning : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SFolderCleaning) {}
	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetDataToStore)
	SLATE_ARGUMENT(FString, CurrentSelectedFolder)
	SLATE_END_ARGS()

public:
	/**
	 * Constructs the SFolderCleaning widget.
	 *
	 * @param InArgs The arguments passed to the widget during its construction.
	 *
	 * This method initializes the SAdvancedDeletionTab widget, setting up its layout
	 * and initializing member variables. The widget includes a title, a combo box
	 * for asset selection, a scrollable list of assets, and buttons for deletion
	 * and selection actions.
	 */
	void Construct(const FArguments& InArgs);

	/**
	 * Retrieves the font style used for embossed text.
	 *
	 * @return A FSlateFontInfo object representing the embossed text font style.
	 */
	FSlateFontInfo GetEmboseedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

	/**
	 * Handles the state change of a checkbox associated with a specific asset.
	 *
	 * @param NewState The new state of the checkbox (Unchecked, Checked, or Undetermined).
	 * @param AssetData A shared pointer to the asset data associated with the checkbox.
	 *
	 * Depending on the new state of the checkbox, this method either adds the asset data
	 * to the list of assets to be deleted or removes it from the list.
	 */
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	/**
	 * Handles the selection change event for the combo box.
	 *
	 * @param SelecetedOption The newly selected option in the combo box.
	 * @param InSelectInfo Information about how the selection was made (e.g., user interaction, programmatic).
	 *
	 * This method updates the displayed text of the combo box and refreshes the list of assets
	 * based on the selected option. It uses the Automation module to filter assets accordingly.
	 */
	void OnComboSelectionChange(TSharedPtr<FString> SelecetedOption, ESelectInfo::Type InSelectInfo);

	/**
	 * Handles the selection asset change for the combo box  
	 * @param SelecetedOption The newly selected option in the combo box.
	 * @param InSelectInfo Information about how the selection was made (e.g., user interaction, programmatic).
	 */
	void OnAssetSelectionChange(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);

	/**
	 * Refreshes the asset list view to reflect the current set of displayed assets.
	 *
	 * This method clears the arrays storing selected assets and checkboxes, then rebuilds the list view
	 * if it has already been constructed. This ensures the UI accurately represents the current asset data.
	 */
	void RefreshAssetListView();

	/**
	 * Constructs a list view widget to display asset data.
	 *
	 * @return A shared reference to the constructed list view widget displaying assets.
	 */
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();

	/**
	 * Generates a row for the list view based on the provided asset data.
	 *
	 * @param AssetDataToDisplay The asset data to be displayed in the row.
	 * @param OwnerTable The table view that owns this row.
	 * @return A shared reference to the generated table row.
	 */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * Constructs a checkbox for each asset item in the list.
	 *
	 * @param AssetDataToDisplay The asset data associated with this checkbox.
	 * @return A shared reference to the constructed checkbox.
	 */
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/**
	 * Constructs a text block widget for displaying text within a row.
	 *
	 * @param TextContent The text content to display.
	 * @param FontToUse The font style to use for the text.
	 * @return A shared reference to the constructed text block.
	 */
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	/**
	 * Constructs a button widget for deleting an asset in the row.
	 *
	 * @param AssetDataToDisplay The asset data associated with this button.
	 * @return A shared reference to the constructed button.
	 */
	TSharedRef<SButton> ConstructDeleteButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/**
	 * @brief Constructs a button widget for viewing references of the specified asset.
	 *
	 * This function creates a button that, when clicked, triggers the action to view references
	 * for the given asset data. The button is labeled appropriately and is linked to a click event handler.
	 *
	 * @param AssetDataToDisplay A shared pointer to the asset data for which the reference view button is created.
	 * @return A shared reference to the constructed SButton widget for viewing references.
	 */
	TSharedRef<SButton> ConstructReferenceViewButtoWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/**
	 * @brief Constructs a button widget for generating a size map of the specified asset.
	 *
	 * This function creates a button that, when clicked, triggers the action to generate a size map
	 * for the given asset data. The button is labeled accordingly and is linked to a click event handler.
	 *
	 * @param AssetDataToDisplay A shared pointer to the asset data for which the size map button is created.
	 * @return A shared reference to the constructed SButton widget for generating a size map.
	 */
	TSharedRef<SButton> ConstructSizeMapButtoWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/**
	 * Constructs a button widget for a row in the asset deletion tab. The button
	 * is labeled "Open" and triggers the `OnOpenAssetButtonClicked` method when clicked.
	 *
	 * @param AssetDataToDisplay A shared pointer to the asset data that will be associated with the button.
	 * @return A shared reference to the constructed SButton widget.
	 */
	TSharedRef<SButton> ConstructOpenAssetButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/**
	 * Constructs a button widget for deleting all selected assets.
	 *
	 * @return A shared reference to the constructed delete all button.
	 */
	TSharedRef<SButton> ConstructDeleteAllButton();

	/**
	 * Constructs a button widget for selecting all assets in the list.
	 *
	 * @return A shared reference to the constructed select all button.
	 */
	TSharedRef<SButton> ConstructSelectAllButton();

	/**
	 * Constructs a button widget for deselecting all assets in the list.
	 *
	 * @return A shared reference to the constructed deselect all button.
	 */
	TSharedRef<SButton> ConstructDeselectAllButton();

	/**
	 * Constructs a button widget labeled "Clear Empty Folder" for the advanced deletion tab.
	 * The button is padded, displays a tooltip, and triggers the `OnDeselectAllEmptyFolderButtonClicked` method when clicked.
	 *
	 * @return A shared reference to the constructed SButton widget for clearing empty folders.
	 */
	TSharedRef<SButton> ConstructDeleteAllEmtpyFolderButton();

	/**
	 * @brief Constructs a button widget for refreshing the asset list.
	 *
	 * This function creates a button that, when clicked, triggers the action to refresh the displayed
	 * asset list. The button is labeled appropriately and is designed to fit within the user interface.
	 *
	 * @return A shared reference to the constructed SButton widget for refreshing the asset list.
	 */
	TSharedRef<SButton> RefreshListAssets();

	/**
	 * Constructs a button widget for searching the selected asset in the content browser.
	 *
	 * @param AssetData The asset data associated with this button.
	 * @return A shared reference to the constructed search button.
	 */
	TSharedRef<SButton> SearchAssetInBrowser(TSharedPtr<FAssetData>& AssetData);

	/**
	 * Constructs a text block widget for the tab buttons.
	 *
	 * @param TextContent The text content to display on the button.
	 * @return A shared reference to the constructed text block.
	 */
	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);

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
	TSharedRef<STextBlock> ConstructComboHelpTexts(const FString& Text, ETextJustify::Type TextType);

	/**
	 * Constructs a combo box for selecting different asset filtering options.
	 *
	 * @return A shared reference to the constructed combo box.
	 */
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();

	/**
	 *	Constructs a combo box for selecting differenct asset type filtering options
	 *	
	 *	@return A shared reference to the constructed combo box.
	 */
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructAssetComboBox();

	/**
	 * Generates the content for each item in the combo box.
	 *
	 * @param SourceItem The source item to display in the combo box.
	 * @return A shared reference to the constructed combo box content.
	 */
	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	/**
	 * Handles the click event for the delete button associated with a specific asset.
	 *
	 * @param ClickedAssetData The asset data to be deleted.
	 * @return A reply indicating the handling of the click event.
	 */
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	/**
	 * @brief Handles the click event for the reference view button.
	 *
	 * This function is triggered when the user clicks the button to view references for a specific asset.
	 * It checks if the provided asset data is valid and, if so, invokes the corresponding module function
	 * to handle the reference viewing action.
	 *
	 * @param ClickedAssetData A shared pointer to the asset data that was clicked.
	 * @return A reply indicating that the event has been handled.
	 */
	FReply OnRefViewButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	/**
	 * @brief Handles the click event for the size map button.
	 *
	 * This function is triggered when the user clicks the button to generate a size map for a specific asset.
	 * It checks if the provided asset data is valid and, if so, invokes the corresponding module function
	 * to handle the size mapping action.
	 *
	 * @param ClickedAssetData A shared pointer to the asset data that was clicked.
	 * @return A reply indicating that the event has been handled.
	 */
	FReply OnSizeMapButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	/**
	 * Handles the event when the "Open" asset button is clicked.
	 * Displays a notification indicating that the asset has been opened.
	 *
	 * @param ClickedAssetData A shared pointer to the asset data associated with the clicked button.
	 * @return An FReply object indicating that the event was handled.
	 */
	FReply OnOpenAssetButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	/**
	 * Handles the click event for deleting all selected assets.
	 *
	 * @return A reply indicating the handling of the click event.
	 */
	FReply OnDeleteAllButtonClicked();

	/**
	 * Handles the click event for selecting all assets in the list.
	 *
	 * @return A reply indicating the handling of the click event.
	 */
	FReply OnSelectAllButtonClicked();

	/**
	 * Handles the click event for deselecting all assets in the list.
	 *
	 * @return A reply indicating the handling of the click event.
	 */
	FReply OnDeselectAllButtonClicked();

	/**
	 * @brief Handles the click event for the "Deselect All Empty Folders" button.
	 *
	 * This function is triggered when the user clicks the button to deselect all folders that are currently
	 * marked as empty. It performs the necessary actions to update the UI accordingly.
	 *
	 * @return A reply indicating that the event has been handled.
	 */
	FReply OnDeselectAllEmptyFolderButtonClicked(); /* clang-format on */

	/**
	 * @brief Handles the click event for refreshing the asset list.
	 *
	 * This function is triggered when the user clicks the button to refresh the displayed asset list.
	 * It performs the necessary actions to reload and update the list of assets shown in the UI.
	 *
	 * @return A reply indicating that the event has been handled.
	 */
	FReply OnRefreshListAssets();


	FReply OnRefreshButtonClicked();

	FReply OnReopenButtonClicked();


	void GetTypeOfAssets(TArray<TSharedPtr<FAssetData>> AssetData);

	void OnAssetListChanged(TSharedPtr<FAssetData> NewSelection, ESelectInfo::Type SelectInfo);


#pragma region Data
	// --- data ---
private:
	/**
	 * An array of checkboxes corresponding to each asset item in the list view.
	 */
	TArray<TSharedRef<SCheckBox>> CheckBoxArray;

	/**
	 * An array containing all the stored asset data.
	 */
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;

	/**
	 * An array containing the asset data currently displayed in the list view.
	 */
	TArray<TSharedPtr<FAssetData>> DisplayedAssetData;

	/**
	 * An array containing asset data marked for deletion.
	 */
	TArray<TSharedPtr<FAssetData>> AssetDataToDeleteArray;

	/**
	 * An array containing the source items for the combo box options.
	 */
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;




	/**
	 * A text block widget used to display the currently selected combo box option.
	 */
	TSharedPtr<STextBlock> ComboDisplayTextBlock;

	/**
	 *
	 */
	TArray<TSharedPtr<FString>> ComboBoxAssetListItems;

	/**
	 *
	 */
	TSharedPtr<STextBlock> ComboAssetDisplayTextBlock;

	/**
	 * A reference to the constructed list view widget displaying the assets.
	 */
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
#pragma endregion Data
};