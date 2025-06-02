// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "ClassViewerFilter.h"
#include "SAssetSearchBox.h"
#include "AssetCleanerTypes.h"


enum class EAssetCleanerViewMode : uint8
{
	AssetsAndFilters,
	Statistics
};

namespace AssetCleaner
{
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



class ASSETCLEANER_API SAssetCleanerWidget final : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAssetCleanerWidget) {}
		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, DiscoveredAssets)
		SLATE_ARGUMENT(FString, CurrentSelectedFolder)
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs);

	TSharedRef<ITableRow> OnTreeGenerateRow(TSharedPtr<FAssetTreeFolderNode> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	FText TreeSearchText;
	TSharedPtr<FTabManager> TabManager;
	TSharedRef<SDockTab> SpawnAssetsAndFiltersTab(const FSpawnTabArgs& SpawnTabArgs);

	TSharedRef<SDockTab> SpawnStatisticsTab(const FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<STreeView<TSharedPtr<FAssetTreeFolderNode>>> TreeListView;
	TArray<TSharedPtr<FAssetTreeFolderNode>> TreeListItems;

	TSharedPtr<FAssetTreeFolderNode> RootItem;

	TSharedRef<SHeaderRow> GetTreeHeaderRow();
	void OnTreeGetChildren(TSharedPtr<FAssetTreeFolderNode> Item, TArray<TSharedPtr<FAssetTreeFolderNode>>& OutChildren);
	void OnTreeSelectionChanged(TSharedPtr<FAssetTreeFolderNode> Selection, ESelectInfo::Type SelectInfo);
	void UpdateFolderTree();

	void InitializeColumns();

	TSet<FName> SelectedPaths;
	FName LastSortedColumn;
	EColumnSortMode::Type ColumnPathSortMode = EColumnSortMode::None;
	EColumnSortMode::Type ColumnAssetsTotalSortMode = EColumnSortMode::None;
	EColumnSortMode::Type ColumnAssetsUsedSortMode = EColumnSortMode::None;
	EColumnSortMode::Type ColumnAssetsUnusedSortMode = EColumnSortMode::None;
	EColumnSortMode::Type ColumnUnusedPercentSortMode = EColumnSortMode::None;
	EColumnSortMode::Type ColumnUnusedSizeSortMode = EColumnSortMode::None;


	void SortTreeItems(const bool UpdateSortingOrder);

	bool TreeItemIsExpanded(const TSharedPtr<FAssetTreeFolderNode>& Item, const TSet<TSharedPtr<FAssetTreeFolderNode>>& CachedItems) const;

	bool TreeItemContainsSearchText(const TSharedPtr<FAssetTreeFolderNode>& Item) const;
	TSharedPtr<SWidget> GetTreeContextMenu();
	void OnTreeExpansionChanged(TSharedPtr<FAssetTreeFolderNode> Item, bool bIsExpanded);
private:
	FSlateFontInfo GetWidgetTextFont() const;

	const FMargin HeaderMargin{ 5.0f };


	void InitializeColumnAdders(); 

	void AddColumnToHeader(TSharedPtr<SHeaderRow> InHeaderRow, const FName& ColumnId, const FString& Label, const float FillWidth);

	const FSlateBrush* GetRevisionControlColumnIconBadge() const;
	
	void LoadAssets();

	void InitializeAssetTypeComboBox(TArray<TSharedPtr<FAssetData>> AssetDataList);

	SHeaderRow::FColumn::FArguments CreateRevisionControlColumn();

	void OnSearchTextChanged(const FText& InText);

	void HandleAssetDoubleClick(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);
	void OpenSelectedDataAssetInEditor();
	void UpdateFilteredAssetList();
	FReply HandleRowMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);
	TSharedRef<SWidget> GenerateColumnMenu();
	void RegisterEditableText(TSharedPtr<FAssetData> AssetData, TSharedRef<SEditableText> EditableText);
	FReply ColumnButtonClicked(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);
	TSharedPtr<class SLayeredImage> CreateFilterImage();
	TSharedRef<SHeaderRow> GenerateHeaderRow();
	TSharedRef<ITableRow> GenerateAssetListRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerSTable);
	void CreateContextMenuFromDataAsset(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);
	void GenerateAssetActionsSubMenu(FMenuBuilder& SubMenuBuilder);
	bool CanAction() const;
	void HandleAssetRename(TSharedPtr<FAssetData> AssetData, const FText& InText, ETextCommit::Type CommitMethod);
	bool IsSelectedAssetValid(const FString& CustomMessage = "") const;
	void FocusOnSelectedAsset();
	void DeleteAsset();
	void OnAssetSelected(TSharedPtr<FAssetData> SelectedItem, ESelectInfo::Type SelectInfo);
	void UpdateColumnVisibility();
	TArray<TSharedPtr<FAssetData>> GetAssetListSelectedItem() const;
	TSharedRef<SWidget> CreateComboButtonContent();
	FText GetSelectedTextBlockInfo() const;
	void OpenAuditAsset();
	void SaveAsset();
	void OpenReferenceViewer();
	void SyncContentBrowserToSelectedAsset();
	void CopyToClipboard(bool bCopyPaths);
	void OpenSizeMap();
	void OpenSelectedAssetInExplorer();
	void ShowMetaDataDialog();

	void OnFilterChanged(const FString& FilterName, bool bIsEnabled);
	void InitializeAdvancedFilters();
	void CollectTexturesWithoutCompression();
	void CollectAssetsWithInvalidReferences();
	void CollectMaterialsInfoManyInstruction();
	void CollectMaterialsInfoManyExpression();
	void SubscribeToAssetRegistryEvent();
	void OnAssetAdded(const FAssetData& AssetToRemoved);
	void OnAssetRemoved(const FAssetData& AssetToRemoved);
	void OnAssetRenamed(const FAssetData& NewAssetData, const FString& Name);
	void OnAssetUpdated(const FAssetData& AssetData);
	bool HasCycle(const FAssetData& Asset);
	void EditSelectionInPropertyMatrix();


	TSet<FString> ActiveFilters;

	TMap<FString, TFunction<bool(const FAssetData&)>> AdvancedFilterPredicates;
	TSet<FString> ActiveAdvancedFilters;

	TSet<FName> AssetsWithMetadata;
	TSet<FName> TexturesWithoutCompression;
	TSet<FName> AssetsWithInvalidReferences;
	TSet<FName> TexturesWithWrongSize;
	TSet<FName> FilteredMaterials;

	TSharedPtr<FTabManager::FLayout> TabLayout;

	FName CurrentSortColumn = NAME_None;
	EColumnSortMode::Type CurrentSortMode = EColumnSortMode::None;

	EColumnSortMode::Type GetColumnSortMode(FName ColumnId) const;
	void OnColumnSortModeChanged(EColumnSortPriority::Type SortPriority, const FName& ColumnId, EColumnSortMode::Type NewSortMode);

	void SortAssetList();

	TSharedRef<SWidget> CreateStatisticsLayout();

	EAssetCleanerViewMode CurrentViewMode = EAssetCleanerViewMode::AssetsAndFilters;
	/** 
	 * Delegate handle for the OnFilesLoaded event subscription.
	 * Used to safely unsubscribe when the widget is destroyed.
	 * @see SubscribeToAssetRegistryEvent(), UnsubscribeFromAssetRegistryEvents()
	 */
	FDelegateHandle FilesLoadedHandle{};

	/**
	 * Registration handle for asset creation events.
	 *
	 * Used to safely unregister from AssetRegistry
	 * callbacks during shutdown.
	 */
	FDelegateHandle AssetAddedDelegateHandle{};

	FDelegateHandle AssetUpdateDelegateHandle;

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
	 * An array containing all the stored asset data.
	 */
	TArray<TSharedPtr<FAssetData>> StoredAssetList;

	/**
	 *
	 */
	FString SelectedDirectory;

	/**
	 * An array containing the asset data currently displayed in the list view.
	 */
	TArray<TSharedPtr<FAssetData>> FilteredDataAssets;

	/**
	 * An array containing asset data marked for deletion.
	 */
	TArray<TSharedPtr<FAssetData>> DeletionAssetList;

	/**
	 * An array of checkboxes corresponding to each asset item in the list view.
	 */
	TArray<TSharedRef<SCheckBox>> CheckBoxList;

	/**
	 * An array containing the source items for the combo box options.
	 */
	TArray<TSharedPtr<FString>> ComboBoxItemList;

	/**
	 * A text block widget used to display the currently selected option in the combo box.
	 */
	TSharedPtr<STextBlock> ComboDisplayTextBlock;

	/**
	 * An array of shared pointers to strings, representing the list of items available in the combo box.
	 */
	TArray<TSharedPtr<FString>> ComboBoxAssetListItems;

	/**
	 * A text block widget used to display the currently selected asset in the combo box.
	 */
	TSharedPtr<STextBlock> ComboAssetDisplayTextBlock;

	/**
	 * Interactive search field for asset filtering.
	 *
	 * Provides real-time text search with optional
	 * syntax (e.g. "type:mesh name:hero").
	 */
	TSharedPtr<SFilterSearchBox> ListViewSearchBox = nullptr;

	/**
	 * Combo button for asset type selection dropdown.
	 *
	 * Handles display and interaction with asset category filters.
	 */
	TSharedPtr<SComboButton> ComboButton = nullptr;

	/**
	 * Current active search query text.
	 *
	 * Bound bidirectionally to search box widget
	 * and filter logic.
	 */
	TAttribute<FText> SearchText = TAttribute<FText>();
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
	 * A reference to the constructed list view widget displaying the assets.
	 */
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
	/**
	 * Selected asset type.
	 *
	 * A shared pointer to the selected asset type as a string.
	 */
	TSharedPtr<FString> SelectedAssetType = nullptr;

	/**
	 * Primary list view control for asset display.
	 *
	 * Binds to FilteredDataAssets and renders scrollable
	 * list with custom item widgets.
	 */
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView = nullptr;

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

	TSharedPtr<FTextFilterExpressionEvaluator> AssetTypeTextFilter;
	// Column visibility flags
	/** Whether the asset type column is currently visible. */
	bool bShowTypeColumn = true;
	
	/** Whether the disk size column is currently visible. */
	bool bShowDiskSizeColumn = true;

	bool bRenamedProgress = false;
	
	/** Whether the asset path column is currently visible. */
	bool bShowPathColumn = true;
	bool bCanRename = false;
	/** Whether the revision control column is currently visible. */
	bool bShowRevisionColumn = true;
};
