// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ClassViewerFilter.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Editor/PropertyEditor/Public/IDetailsView.h"
#include "SAssetSearchBox.h"
#include "Menu/IDataAssetManagerInterface.h"


class UDataAssetManagerSettings;
class SLayeredImage;

namespace DataAssetManager
{
	namespace ModuleName
	{
		constexpr const TCHAR* AssetTools		= TEXT("AssetTools");
		constexpr const TCHAR* AssetRegistry	= TEXT("AssetRegistry");
		constexpr const TCHAR* ContentBrowser	= TEXT("ContentBrowser");
		constexpr const TCHAR* MessageLog		= TEXT("MessageLog");
		constexpr const TCHAR* OutputLog		= TEXT("OutputLog");
		constexpr const TCHAR* Settings			= TEXT("Settings");
		constexpr const TCHAR* DataAssetManager = TEXT("DataAssetManager");
		constexpr const TCHAR* PropertyEditor	= TEXT("PropertyEditor");
	}
	
	namespace Private
	{
		/**
		 * Class used to filter asset classes based on certain conditions like class flags and blueprint base class restrictions.
		 * Implements the IClassViewerFilter interface to provide custom filtering logic for class viewer in the Unreal Editor.
		 */
		class FAssetClassParentFilter : public IClassViewerFilter
		{
		public:

			FAssetClassParentFilter()
			: DisallowedClassFlags(CLASS_None), bDisallowBlueprintBase(false) {}

			/**
			 * Set of allowed parent classes.
			 * All children of these classes will be included unless filtered out by another setting.
			 * This filter excludes classes that are not in this set unless specified otherwise.
			 */
			TSet< const UClass* > AllowedChildrenOfClasses;

			/**
			 * Disallowed class flags.
			 * Used to filter out classes that have the specified flags.
			 */
			EClassFlags DisallowedClassFlags;

			/**
			 * Flag that specifies whether blueprint base classes should be excluded from the class viewer.
			 * If set to true, blueprint base classes will be filtered out.
			 */
			bool bDisallowBlueprintBase;

			/**
			 * Determines whether the given class is allowed based on the filter settings.
			 *
			 * @param InInitOptions Initialization options for the class viewer.
			 * @param InClass The class to be checked.
			 * @param InFilterFuncs Filter functions that provide additional filtering logic.
			 *
			 * @return true if the class is allowed, false otherwise.
			 */
			virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
			{
				bool bAllowed = !InClass->HasAnyClassFlags(DisallowedClassFlags)
					&& InClass->CanCreateAssetOfClass()
					&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

				if (bAllowed && bDisallowBlueprintBase)
				{
					if (FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass))
					{
						return false;
					}
				}

				return bAllowed;
			}

			/**
			 * Determines whether an unloaded class is allowed based on the filter settings.
			 *
			 * @param InInitOptions Initialization options for the class viewer.
			 * @param InUnloadedClassData Data about the unloaded class to be checked.
			 * @param InFilterFuncs Filter functions that provide additional filtering logic.
			 *
			 * @return true if the unloaded class is allowed, false otherwise.
			 */
			virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
			{
				if (bDisallowBlueprintBase)
				{
					return false;
				}

				return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
					&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
			}
		};
	}
}

class DATAASSETMANAGER_API SDataAssetManagerWidget : public SCompoundWidget, public IDataAssetManagerInterface
{
	SLATE_BEGIN_ARGS(SDataAssetManagerWidget) {}
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs);
	~SDataAssetManagerWidget();

protected:
#pragma region IDataAssetManagerInterface
	/////////////////////////////////////////////////////////////////////////////
	/**
	 * Interface for managing Data Assets in the Unreal Editor.
	 *
	 * Provides core operations for:
	 * - Asset lifecycle (create/save/validate)
	 * - Editor integration (content browser sync, reference analysis)
	 * - Utility features (clipboard, documentation, plugin control)
	 *
	 * Implemented by the main Data Asset Manager system to expose functionality
	 * to both UI widgets and other editor modules.
	 */
	/////////////////////////////////////////////////////////////////////////////
	virtual void CreateNewDataAsset() override;
	virtual void OpenSelectedDataAssetInEditor() override;
	virtual void ToggleDataAssetListVisibility() override;
	virtual void ShowDocumentation() override;
	virtual void SaveDataAsset() override;
	virtual void SaveAllData() override;
	virtual void SyncContentBrowserToSelectedAsset() override;
	virtual void CopyToClipboard(bool bCopyPaths) override;
	virtual void OpenReferenceViewer() override;
	virtual void OpenSizeMap() override;
	virtual void OpenAuditAsset() override;
	virtual void OpenPluginSettings() override;
	virtual void ShowSourceControlDialog() override;
	virtual void RestartPlugin() override;
	virtual void OpenMessageLogWindow() override;
	virtual void OpenOutputLogWindow() override;
	virtual bool CanRename() const override;
	virtual void FocusOnSelectedAsset() override;
	virtual void DeleteDataAsset() override;
	virtual void ShowAssetMetaData() override;
#pragma endregion IDataAssetManagerInterface	
private:
	/**
	 * Registers an editable text widget for the specified asset.
	 *
	 * Associates the given SEditableText widget with an asset for later reference.
	 * This allows easy retrieval and updates of the text input corresponding to the asset.
	 *
	 * @param AssetData     A shared pointer to the asset data associated with the widget.
	 * @param EditableText  A shared reference to the editable text widget used for this asset.
	 */
	void RegisterEditableText(TSharedPtr<FAssetData> AssetData, TSharedRef<SEditableText> EditableText);

	/**
	 * Handles asset renaming via text input commit.
	 *
	 * Triggered when the user finishes editing the asset name in the UI.
	 * This method updates the asset with the new name based on the input text and commit type.
	 *
	 * @param AssetData     A shared pointer to the asset data being renamed.
	 * @param InText        The new text entered by the user.
	 * @param CommitMethod  The method used to commit the text (e.g., Enter, Focus loss).
	 */
	void HandleAssetRename(TSharedPtr<FAssetData> AssetData, const FText& InText, ETextCommit::Type CommitMethod);
	
	/**
	 * Handles double-click events on an asset row.
	 *
	 * Typically used to open the asset or show its details in the editor when double-clicked.
	 *
	 * @param InGeometry    The geometry of the widget receiving the event.
	 * @param MouseEvent    The mouse event containing double-click data.
	 */	
	void HandleAssetDoubleClick(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);
	
	/**
	 * Creates a context menu for the selected data asset.
	 *
	 * This method generates a context menu that appears when the user interacts with an asset in the UI.
	 *
	 * @param InGeometry The geometry for the context menu.
	 * @param MouseEvent Information about the mouse event triggering the context menu.
	 * @return A reply object representing the result of the context menu action.
	 */
	void CreateContextMenuFromDataAsset(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);

	/**
	 * Handles mouse button down events on a row widget.
	 *
	 * Called when the user presses a mouse button while hovering over a specific row.
	 * This can be used to trigger row selection, drag detection, or context menu logic.
	 *
	 * @param InGeometry     The geometry of the widget receiving the event.
	 * @param MouseEvent     Describes the mouse event, including button, position, and modifiers.
	 * @return               An FReply indicating how the event was handled.
	 */
	FReply HandleRowMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);

	/**
	 * Initializes the Asset Registry.
	 *
	 * This method sets up the asset registry to manage the assets in the project. It is typically called during the
	 * initialization phase of the widget to ensure the registry is available for asset management operations.
	 */
	void SubscribeToAssetRegistryEvent();

	/**
	 * Initializes the font information for rendering text.
	 *
	 * This method configures the font style, size, and other properties required for rendering text within the
	 * user interface.
	 */
	void InitializeTextFontInfo();

	/**
	 * Creates the Details View for asset information.
	 *
	 * This method sets up the details view widget that will display detailed information about a selected asset.
	 */
	void CreateDetailsView();

	/**
	 * Creates and configures parameters for Details View panel.
	 * 
	 * @return Configured FDetailsViewArgs structure with default settings
	 *         for asset property display in this widget.
	 *         
	 * @note Sets common view options like bAllowSearch, bHideSelectionTip, etc.
	 * @see SDetailsView, IDetailsView
	 */
	FDetailsViewArgs CreateDetailsViewArgs() const;

	/**
	 * Loads the data assets based on the plugin settings.
	 *
	 * This method retrieves and loads data assets according to the configuration specified in the plugin settings.
	 *
	 * @param PluginSettings The settings used to load the data assets.
	 */
	void LoadDataAssets(const UDataAssetManagerSettings* PluginSettings);

	/**
	 * Updates the list of assets based on the applied filter.
	 *
	 * This method filters the assets according to the current filter and updates the displayed list of assets.
	 */
	void UpdateFilteredAssetList();

	/**
	 * Called when an asset is selected in the asset list.
	 *
	 * This method handles the selection of an asset in the asset list and can trigger updates to other UI components
	 * based on the selection.
	 *
	 * @param SelectedItem The selected asset data.
	 * @param SelectInfo Information about the selection.
	 */
	void OnAssetSelected(TSharedPtr<FAssetData> SelectedItem, ESelectInfo::Type SelectInfo);

	/**
	 * Opens the details panel for the selected asset.
	 *
	 * This method opens a detailed view for the selected asset, allowing the user to inspect and interact with
	 * asset-specific properties.
	 *
	 * @param SelectedItem The selected asset data.
	 */
	void OpenDetailViewPanelForAsset(TSharedPtr<FAssetData> SelectedItem);

	/**
	 * Called when the search text is changed in the search box.
	 *
	 * This method updates the asset list based on the new search query.
	 *
	 * @param InText The updated search query text.
	 */
	void OnSearchTextChanged(const FText& InText);

	/**
	 * Generates a row for the asset list view.
	 *
	 * This method creates a new row for the asset list view, representing an individual asset.
	 *
	 * @param Item The asset data for the row.
	 * @param OwnerSTable The table view that owns this row.
	 * @return A reference to the generated table row widget.
	 */
	TSharedRef<ITableRow> GenerateAssetListRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerSTable);

	/**
	 * Initializes the asset type combo box.
	 *
	 * This method populates and sets up the combo box for selecting asset types from the filtered list of assets.
	 *
	 * @param AssetDataList The list of assets to populate the combo box.
	 */
	void InitializeAssetTypeComboBox(TArray<TSharedPtr<FAssetData>> AssetDataList);

	/**
	 * Saves all data assets.
	 *
	 * This method saves all the data assets, ensuring any modifications are persisted to storage.
	 *
	 * @return A boolean indicating whether the save operation was successful.
	 */
	bool SaveAllDataAsset();

	/**
	 * Processes the asset data and applies a specified function.
	 *
	 * This method processes the provided asset data using the specified function for each asset.
	 *
	 * @param RefAssetData The asset data to be processed.
	 * @param ProcessFunction A function that defines how the asset data should be processed.
	 */
	void ProcessAssetData(const TArray<FAssetData>& RefAssetData, TFunction<void(const TArray<FAssetIdentifier>&)> ProcessFunction);

	/**
	 * Called when a new asset is added to the registry.
	 *
	 * This method updates the internal asset list when a new asset is added to the project.
	 *
	 * @param NewAssetData The newly added asset data.
	 */
	void OnAssetAdded(const FAssetData& NewAssetData);

	/**
	 * Called when an asset is removed from the registry.
	 *
	 * This method updates the internal asset list when an asset is removed from the project.
	 *
	 * @param AssetToRemoved The asset data for the asset being removed.
	 */
	void OnAssetRemoved(const FAssetData& AssetToRemoved);

	/**
	 * Called when an asset is renamed in the registry.
	 *
	 * This method updates the asset list when an asset's name is changed.
	 *
	 * @param NewAssetData The renamed asset data.
	 * @param Name The new name of the asset.
	 */
	void OnAssetRenamed(const FAssetData& NewAssetData, const FString& Name);

	/**
	 * Creates a filter image for asset filtering.
	 *
	 * This method generates an image used for filtering assets in the UI.
	 *
	 * @return A shared pointer to the filter image widget.
	 */
	TSharedPtr<SLayeredImage> CreateFilterImage();

	/**
	 * Retrieves the current selection mode for the asset list.
	 *
	 * Used to determine whether the list allows:
	 * - Single-selection (for focused editing)
	 * - Multi-selection (for batch operations)
	 *
	 * @return Current selection mode (ESelectionMode::Single/Multi/None)
	 */
	ESelectionMode::Type GetAssetListSelectionMode() const;

	/**
	 * Determines visibility state of the search box.
	 *
	 * Typically bound to UI visibility properties to:
	 * - Show/hide based on window size
	 * - Temporarily hide during operations
	 * - Respect user preferences
	 *
	 * @return EVisibility::Visible/Collapsed/Hidden based on current logic
	 */
	EVisibility GetVisibilitySearchBox() const;

	/**
	 * Callback for when an item in the combo box is clicked.
	 *
	 * This method is triggered when an item in the combo box is selected, and it can be used to handle the selected
	 * asset type.
	 *
	 * @param SourceItem The selected item from the combo box.
	 * @return A reply object representing the result of the item click.
	 */
	FReply OnItemClicked(TSharedPtr<FString> SourceItem);

	/**
	 * Creates the content for the combo button.
	 *
	 * This method generates the widget content displayed in the combo button.
	 *
	 * @return A reference to the widget that will be used as content for the combo button.
	 */
	TSharedRef<SWidget> CreateComboButtonContent();

	/**
	 * Updates the content of the combo button.
	 *
	 * This method updates the content of the combo button, typically after a change in asset type or filter.
	 */
	void UpdateComboButtonContent();

	/**
	 * Retrieves the list of selected assets.
	 *
	 * This method returns the currently selected items from the asset list.
	 *
	 * @return An array of selected asset data.
	 */
	TArray<TSharedPtr<FAssetData>> GetAssetListSelectedItem() const;

	/**
	 * Checks if the currently selected asset is valid.
	 * 
	 * @param CustomMessage Optional custom error message to display if the asset is invalid. 
	 *        If empty, a default message will be used.
	 * @return true if the selected asset is valid and can be used, false otherwise.
	 * 
	 * @note This method logs a warning message with the function name when the asset is invalid.
	 * @see SelectedAsset, SDataAssetManagerWidgetLog
	 * 
	 * Example usage:
	 * @code
	 * if (!IsSelectedAssetValid("Custom error for save operation")) {
	 *     return;
	 * }
	 * @endcode
	 */
	bool IsSelectedAssetValid(const FString& CustomMessage = "") const;

	/**
	 * Sets focus and selection on a newly added asset in the data asset manager.
	 * 
	 * @param NewAssetData The asset data of the newly added asset to focus on.
	 * 
	 * @note This method typically triggers UI updates to highlight the new asset.
	 * @warning Ensure the asset is fully loaded before calling this method.
	 */
	void FocusOnNewlyAddedAsset(const FAssetData& NewAssetData);


	/**
	 * Retrieves the brush icon used for the Revision Control column.
	 *
	 * @return Pointer to the slate brush representing the column badge.
	 */
	const FSlateBrush* GetRevisionControlColumnIconBadge() const;
	
	/**
	 * Generates the full header row widget for the asset table.
	 *
	 * Constructs and returns a shared reference to the header row,
	 * populated with all visible columns.
	 *
	 * @return Shared reference to the constructed SHeaderRow.
	 */
	TSharedRef<SHeaderRow> GenerateHeaderRow();
	
	/**
	 * Creates the arguments used to define the Revision Control column.
	 *
	 * Used when dynamically adding the column to the header row.
	 *
	 * @return Configured column argument structure.
	 */
	SHeaderRow::FColumn::FArguments CreateRevisionControlColumn();
	
	/**
	 * Updates the visibility of all columns in the header row.
	 *
	 * Evaluates internal visibility flags and shows or hides columns accordingly.
	 */
	void UpdateColumnVisibility();
	
	/**
	 * Handles mouse button events on column-related buttons.
	 *
	 * Used for toggling column visibility or invoking context actions.
	 *
	 * @param InGeometry    The geometry of the widget receiving the click.
	 * @param MouseEvent    The mouse event to handle.
	 * @return A handled or unhandled FReply.
	 */
	FReply ColumnButtonClicked(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);
	
	/**
	 * Adds a new column to the provided header row.
	 *
	 * Dynamically appends a column with the specified ID, label, and fill width.
	 *
	 * @param InHeaderRow   The header row to modify.
	 * @param ColumnId      Unique identifier for the column.
	 * @param Label         Text label displayed in the header.
	 * @param FillWidth     The fill ratio for the column’s width.
	 */
	void AddColumnToHeader(TSharedPtr<SHeaderRow> InHeaderRow, const FName& ColumnId, const FString& Label, const float FillWidth);
	
	/**
	 * Initializes the map of column adder functions.
	 *
	 * Each entry in the map corresponds to a column and its adder logic.
	 * Used during header row construction.
	 */
	void InitializeColumnAdders();
	
	/**
	 * Retrieves the text block info related to the currently selected item(s).
	 *
	 * Returns user-facing information about the selected assets or context.
	 *
	 * @return Localized text for display in the UI.
	 */
	FText GetSelectedTextBlockInfo() const;


#pragma region Data
	/**
	 * Combo button for asset type selection dropdown.
	 *
	 * Handles display and interaction with asset category filters.
	 */
	TSharedPtr<SComboButton> ComboButton = nullptr;

	/**
	 * Complete collection of discovered assets in the project.
	 *
	 * Populated during initial scan and updated via asset registry delegates.
	 * Contains raw FAssetData before any filtering is applied.
	 */
	TArray<TSharedPtr<FAssetData>> DataAssets = {};

	/**
	 * Subset of DataAssets that pass current filter criteria.
	 *
	 * Dynamically updated based on:
	 * - Search text matches
	 * - Asset type filters
	 * - Custom filter conditions
	 */
	TArray<TSharedPtr<FAssetData>> FilteredDataAssets = {};

	/**
	 * Assets queued for deferred deletion.
	 *
	 * Maintains references until deletion is confirmed
	 * or canceled via transaction.
	 */
	TArray<TSharedPtr<FAssetData>> DeletionDataAssets = {};

	/**
	 * Primary list view control for asset display.
	 *
	 * Binds to FilteredDataAssets and renders scrollable
	 * list with custom item widgets.
	 */
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView = nullptr;

	/**
	 * Property inspector for selected assets.
	 *
	 * Displays detailed UObject properties when
	 * assets are selected in the list view.
	 */
	TSharedPtr<IDetailsView> DetailsView = nullptr;

	/**
	 * Resizable layout divider between list and details.
	 *
	 * Allows runtime adjustment of panel widths
	 * with persistent position saving.
	 */
	TSharedPtr<SSplitter> Splitter = nullptr;

	/**
	 * Interactive search field for asset filtering.
	 *
	 * Provides real-time text search with optional
	 * syntax (e.g. "type:mesh name:hero").
	 */
	TSharedPtr<SFilterSearchBox> ListViewSearchBox = nullptr;

	/**
	 * Current active search query text.
	 *
	 * Bound bidirectionally to search box widget
	 * and filter logic.
	 */
	TAttribute<FText> SearchText = TAttribute<FText>();

	/**
	 * Typography settings for UI text elements.
	 *
	 * Defines font family, size, and style used
	 * throughout the asset manager UI.
	 */
	FSlateFontInfo TextFontInfo = {};

	/**
	 * Registration handle for asset creation events.
	 *
	 * Used to safely unregister from AssetRegistry
	 * callbacks during shutdown.
	 */
	FDelegateHandle AssetAddedDelegateHandle{};

	/**
	 * Registration handle for asset deletion events.
	 *
	 * Tracks removal notifications from AssetRegistry.
	 */
	FDelegateHandle AssetRemovedDelegateHandle{};

	/**
	 * Registration handle for asset rename events.
	 *
	 * Maintains consistency when assets are
	 * renamed in content browser.
	 */
	FDelegateHandle AssetRenamedDelegateHandle{};

	/** 
	 * Delegate handle for the OnFilesLoaded event subscription.
	 * Used to safely unsubscribe when the widget is destroyed.
	 * @see SubscribeToAssetRegistryEvent(), UnsubscribeFromAssetRegistryEvents()
	 */
	FDelegateHandle FilesLoadedHandle{};

	/**
	 * Currently highlighted asset in UI.
	 *
	 * Synchronized between:
	 * - List view selection
	 * - Details view target
	 * - External selections
	 */
	TSharedPtr<FAssetData> SelectedAsset = nullptr;

	/**
	 * Active text editor for asset name modification.
	 *
	 * When valid, indicates the UI is in asset rename/edit mode.
	 */
	TSharedPtr<SEditableText> EditableTextWidget = nullptr;

	/**
	 * Mapping of editable text widgets used for asset property editing.
	 *
	 * Key:   TPair<PackagePath, AssetName> - Unique identifier combining asset location and name
	 * Value: TSharedPtr<SEditableText>     - Shared reference to the editable text widget
	 *
	 * Usage:
	 * - Provides O(1) access to text widgets by asset identifier
	 * - Maintains widget lifetime through shared pointers
	 * - Keys remain valid even if widget text changes
	 *
	 * @note Widgets are typically created during UI construction and removed when assets are unloaded.
	 * @see FAssetData, SEditableText
	 */
	TMap<TPair<FName, FName>, TSharedPtr<SEditableText>> EditableTextWidgets = {};

	/**
	 * Combo box asset list items.
	 *
	 * An array of shared pointers to FString objects representing the list of asset types available in the combo box.
	 */
	TArray<TSharedPtr<FString>> ComboBoxAssetListItems = {};

	/**
	 * Selected asset type.
	 *
	 * A shared pointer to the selected asset type as a string.
	 */
	TSharedPtr<FString> SelectedAssetType = nullptr;

	/**
	 * Menu bar widget.
	 *
	 * A shared pointer to the widget representing the menu bar in the user interface.
	 */
	TSharedPtr<SWidget> MenuBar = nullptr;

	/**
	 * Indicates whether assets can be renamed.
	 *
	 * This boolean flag determines if renaming of assets is allowed in the current context.
	 */
	bool bCanRename = true;

	/**
	 * Indicates whether the renaming progress is in progress.
	 *
	 * This boolean flag indicates if the renaming operation is ongoing.
	 */
	bool bRenamedProgress = false;

	/**
	 * Indicates whether the slot view is visible.
	 *
	 * This boolean flag determines if the asset slot view is visible in the UI.
	 */
	bool bIsSlotVisible = true;

	// Column visibility flags
	/** Whether the asset type column is currently visible. */
	bool bShowTypeColumn = true;
	
	/** Whether the disk size column is currently visible. */
	bool bShowDiskSizeColumn = true;
	
	/** Whether the asset path column is currently visible. */
	bool bShowPathColumn = true;
	
	/** Whether the revision control column is currently visible. */
	bool bShowRevisionColumn = true;

	/**
	 * The value for the splitter position.
	 *
	 * This attribute determines the position of the splitter dividing the UI sections.
	 */
	TAttribute<float> SplitterValue = 0.4f;

	// Fixed-size allocator constants
	/** Maximum number of column adder functions. */
	static constexpr uint32 NumColumnAdders = 8;
	
	/**
	 * Mapping of column IDs to column adder functions.
	 *
	 * Key:   Column ID (FName)
	 * Value: Function that adds the column to a header row
	 *
	 * Uses a fixed-size allocator for performance and memory efficiency.
	 */
	TMap<FName, TFunction<void(TSharedPtr<SHeaderRow>)>, TFixedSetAllocator<NumColumnAdders>> ColumnAdders;
	
	/** Maximum number of ordered columns. */
	static constexpr uint32 NumColumnOrder = 6;
	
	/**
	 * Ordered list of column IDs defining display order.
	 *
	 * Used to maintain consistent column placement in the UI.
	 * Uses fixed-size memory for predictable layout.
	 */
	TArray<FName, TFixedAllocator<NumColumnOrder>> ColumnOrder;

	/**
	 * Holds the currently active filters in the data asset manager.
	 * Each filter is represented as a string.
	 */
	TSet<FString> ActiveFilters;	

#pragma endregion Data
};