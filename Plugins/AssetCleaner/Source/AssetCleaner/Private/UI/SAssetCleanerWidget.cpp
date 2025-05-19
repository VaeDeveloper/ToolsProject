// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SAssetCleanerWidget.h"
#include "AssetCleaner.h"
#include "RevisionControlStyle/RevisionControlStyle.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "SourceControlHelpers.h"
#include "UObject/ObjectSaveContext.h"
#include "SlateExtras.h"
#include "Widgets/Images/SLayeredImage.h"
#include "RevisionControlStyle/RevisionControlStyle.h"
#include "FrontendFilters.h"
#include "Editor/ContentBrowser/Private/SFilterList.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Widgets/Input/SSearchBox.h"
#include "ObjectTools.h"
#include "AssetManagerEditorModule.h"
#include "ContentBrowserModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Filters/SBasicFilterBar.h"
#include "Widgets/Images/SLayeredImage.h" 
#include "Filters/SBasicFilterBar.h"   
#include "Styling/SlateIconFinder.h"
#include "MaterialStatsCommon.h"
#include "MaterialEditingLibrary.h"
#include "SMetaDataView.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(SAssetCleanerWidgetLog, All, All)
DEFINE_LOG_CATEGORY_STATIC(SAssetCleanerTableRowLog, All, All)
DEFINE_LOG_CATEGORY_STATIC(SFilterContainerWidgetLog, All, All)

#define LOCTEXT_NAMESPACE "SAssetCleanerWidget"

/* clang-format off */
namespace AssetCleaner
{
	struct FAssetLists
	{
		static const FName AllAssets;
		static const FName UnusedAssets;
		static const FName DuplicateNameAssets;
	};

	const FName FAssetLists::AllAssets = TEXT("All Available Assets");
	const FName FAssetLists::UnusedAssets = TEXT("Unused Assets");
	const FName FAssetLists::DuplicateNameAssets = TEXT("Asset with duplicate name");


	FName AssetCleanerModuleName = TEXT("AssetCleaner");

	static constexpr float TitleInfoFontTextSize = 15.0f;
	static constexpr const TCHAR* TitleTextColor = TEXT("2A6FFFFF");
	
	static constexpr int32 MaxNameLength = 64;
	static FAssetCleanerModule& GetAssetCleanerModule()
	{
		return FModuleManager::LoadModuleChecked<FAssetCleanerModule>(AssetCleanerModuleName);
	}

	namespace IconStyle
	{
		static const FName AppStyle = FAppStyle::GetAppStyleSetName();
		static const FName RevisionControlStyle = FRevisionControlStyleManager::GetStyleSetName();
	}

	namespace Icons
	{
		// File Menu
		const FSlateIcon AddNewAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.AssetActions.ReimportAsset");
		const FSlateIcon SaveAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SaveAllCurrentFolder");
		const FSlateIcon SaveAll = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SaveAllCurrentFolder");
		const FSlateIcon Validate = FSlateIcon(IconStyle::AppStyle, "Icons.Adjust");

		// Assets Menu
		const FSlateIcon OpenAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ShowInExplorer");
		const FSlateIcon FindInCB = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ShowInExplorer");
		const FSlateIcon Copy = FSlateIcon(IconStyle::AppStyle, "GenericCommands.Copy");
		const FSlateIcon ReferenceViewer = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ReferenceViewer");
		const FSlateIcon SizeMap = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SizeMap");
		const FSlateIcon Audit = FSlateIcon(IconStyle::AppStyle, "Icons.Audit");
		const FSlateIcon RevisionControl = FSlateIcon(IconStyle::RevisionControlStyle, "RevisionControl.Actions.Diff");
		const FSlateIcon Duplicate = FSlateIcon(IconStyle::AppStyle, "Icons.Duplicate");
		const FSlateIcon Edit = FSlateIcon(IconStyle::AppStyle, "Icons.Edit");
		// Settings Menu
		const FSlateIcon MessageLog = FSlateIcon(IconStyle::AppStyle, "MessageLog.TabIcon");
		const FSlateIcon Visibility = FSlateIcon(IconStyle::AppStyle, "Icons.Visibility");
		const FSlateIcon Settings = FSlateIcon(IconStyle::AppStyle, "Icons.Settings");
		const FSlateIcon Refresh = FSlateIcon(IconStyle::AppStyle, "Icons.Refresh");
		const FSlateIcon OutputLog = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Log.TabIcon");

		// Help Menu
		const FSlateIcon Documentation = FSlateIcon(IconStyle::AppStyle, "GraphEditor.GoToDocumentation");
	}

	namespace Private
	{
		FString GetAssetDiskSize(const FAssetData& AssetData)
		{
			FString PackageFileName;
			if (FPackageName::DoesPackageExist(AssetData.PackageName.ToString(), &PackageFileName))
			{
				const int64 FileSize = IFileManager::Get().FileSize(*PackageFileName);
				
				if (FileSize == INDEX_NONE)
				{
				    return TEXT("Unknown");
				}

				const double SizeInKb = static_cast<double>(FileSize) / 1024.0;
				if (SizeInKb >= 1024)
				{
				    const double SizeInMb = SizeInKb / 1024.0;
				    return FString::Printf(TEXT("%.1f Mb"), SizeInMb);
				}
				
				return FString::Printf(TEXT("%.1f Kb"), SizeInKb);
			}
			return TEXT("Unknown");
		}

		static bool DeleteMultiplyAsset(const TArray<FAssetData>& Assets)
		{
			if (Assets.Num() == 0)
			{
				UE_LOG(SAssetCleanerTableRowLog, Warning, TEXT("%s No assets to delete!"), *FString(__FUNCTION__));
				return false;
			}

			int32 DeletedCount = ObjectTools::DeleteAssets(Assets);
			UE_LOG(SAssetCleanerTableRowLog, Log, TEXT("%s Deleted %d assets"),*FString(__FUNCTION__), DeletedCount);

			return DeletedCount > 0;
		}

		/**
		 * Processes asset data by converting it to asset identifiers and executing a callback.
		 * 
		 * @param RefAssetData Array of asset data to process
		 * @param ProcessFunction Callback function that receives processed asset identifiers
		 * 
		 * @note Uses IAssetManagerEditorModule to extract identifiers from asset data
		 * @see IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList()
		 */
		static void ProcessAssetData(const TArray<FAssetData>& RefAssetData, TFunction<void(const TArray<FAssetIdentifier>&)> ProcessFunction)
		{
			TArray<FAssetIdentifier> AssetIdentifiers;
			IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList(RefAssetData, AssetIdentifiers);
			ProcessFunction(AssetIdentifiers);
		}

		static bool IsExcludedFolder(const FString& FolderPath)
		{
			return FolderPath.Contains(TEXT("Developers")) 
				|| FolderPath.Contains(TEXT("Collections")) 
				|| FolderPath.Contains(TEXT("__ExternalActors__")) 
				|| FolderPath.Contains(TEXT("__ExternalObjects__"));
		}
	}
}

namespace AssetCleanerListColumns
{
	/** IDs for list columns */
	static const FName ColumnID_RC("RevisionControl");
	static const FName ColumnID_Name("Name");
	static const FName ColumnID_Type("Type");
	static const FName ColumnID_DiskSize("DiskSize");
	static const FName ColumnID_Path("Path");
}

class SAssetCleanerTableRow : public SMultiColumnTableRow<TSharedPtr<FAssetData>>
{
public:
	DECLARE_DELEGATE_ThreeParams(FOnAssetRenamed, TSharedPtr<FAssetData>, const FText&, ETextCommit::Type);
    DECLARE_DELEGATE_TwoParams(FOnCreateContextMenu, const FGeometry&, const FPointerEvent&);
    DECLARE_DELEGATE_TwoParams(FOnAssetDoubleClicked, const FGeometry&, const FPointerEvent&);
    DECLARE_DELEGATE_TwoParams(FOnRegisterEditableText, TSharedPtr<FAssetData>, TSharedRef<SEditableText>);
	DECLARE_DELEGATE_RetVal_TwoParams(FReply, FOnAssetMouseButtonDown, const FGeometry&, const FPointerEvent&);
public:
	SLATE_BEGIN_ARGS(SAssetCleanerTableRow){}
		SLATE_ARGUMENT(TSharedPtr<FAssetData>, Item)
		SLATE_ARGUMENT(TSharedPtr<SAssetCleanerWidget>, Owner)

		SLATE_EVENT(FOnAssetRenamed, OnAssetRenamed)
        SLATE_EVENT(FOnCreateContextMenu, OnCreateContextMenu)
        SLATE_EVENT(FOnAssetDoubleClicked, OnAssetDoubleClicked)
        SLATE_EVENT(FOnRegisterEditableText, OnRegisterEditableText)
		SLATE_EVENT(FOnAssetMouseButtonDown, OnMouseButtonDown)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
	    Item = InArgs._Item;
		AddDirtyEventHandler(Item->PackageName.ToString());
	    
		OnAssetRenamed = InArgs._OnAssetRenamed;
        OnCreateContextMenu = InArgs._OnCreateContextMenu;
        OnAssetDoubleClicked = InArgs._OnAssetDoubleClicked;
        OnRegisterEditableText = InArgs._OnRegisterEditableText;
		MouseButtonDown = InArgs._OnMouseButtonDown;

	    SMultiColumnTableRow::Construct(FSuperRowType::FArguments()
			.Style(FAppStyle::Get(), "ContentBrowser.AssetListView.ColumnListTableRow"), InOwnerTable);
    }

	virtual ~SAssetCleanerTableRow()
	{
		if (OnPackageDirtyStateChangedHandle.IsValid())
		{
			UPackage::PackageDirtyStateChangedEvent.Remove(OnPackageDirtyStateChangedHandle);
		}
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnId) override
	{
		if (ColumnId == AssetCleanerListColumns::ColumnID_Name)
		{
			TSharedRef<SEditableText> EditableText = 
				SNew(SEditableText)
				.Cursor(EMouseCursor::Hand)
				.HintText(FText::FromName(Item->PackagePath))
				.Text_Lambda([this]() { return FText::FromName(Item->AssetName); })
				.SelectAllTextWhenFocused(true)
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type Type) {
					if (OnAssetRenamed.IsBound() && Type == ETextCommit::OnEnter)
					{
						OnAssetRenamed.Execute(Item, Text, Type);
					}
				});

			if (OnRegisterEditableText.IsBound())
			{
				OnRegisterEditableText.Execute(Item, EditableText);

			}

		return SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.AutoWidth()
				[
					SNew(SOverlay)
					+SOverlay::Slot()
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("ContentBrowser.ColumnViewAssetIcon"))
						.ColorAndOpacity(FColor::FromHex("616161FF"))
					]

					+SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Bottom)
					[
						SAssignNew(DirtyBrushWidget, SImage)
						.Image(FAppStyle::GetBrush("Icons.DirtyBadge"))
						.Visibility(EVisibility::Collapsed)					]
					]
				  
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SOverlay)
					+SOverlay::Slot()
					.VAlign(VAlign_Center)
					.Padding(6.0f, 0.0f, 0.0f, 0.0f)
					[
						EditableText
					]
					
					+ SOverlay::Slot()
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					[
						SNew(SBox)
						.Visibility(EVisibility::Visible)
					]

					+SOverlay::Slot()
					[
						SNew(SBorder)
						.Cursor(EMouseCursor::Hand)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.ColorAndOpacity(FColor::Transparent)
						.BorderBackgroundColor(FColor::Transparent)
						.OnMouseButtonDown(this, &SAssetCleanerTableRow::BorderMouseButtonDown)
						.OnMouseDoubleClick(this, &SAssetCleanerTableRow::BorderMouseDoubleClicked)
					]
				];
		}
		else if (ColumnId == AssetCleanerListColumns::ColumnID_Type)
		{
			CreateTextWidget(Item->GetClass()->GetName());
		}
		else if (ColumnId == AssetCleanerListColumns::ColumnID_DiskSize)
		{
			CreateTextWidget(AssetCleaner::Private::GetAssetDiskSize(Item.ToSharedRef().Get()));
		}
		else if (ColumnId == AssetCleanerListColumns::ColumnID_Path)
		{
			CreateTextWidget(Item->PackageName.ToString());
		}
		else if (ColumnId == AssetCleanerListColumns::ColumnID_RC)
		{
			const FString AssetPath = FPackageName::LongPackageNameToFilename(Item->PackageName.ToString(), FPackageName::GetAssetPackageExtension());
			FSourceControlStatePtr SourceControlState = ISourceControlModule::Get().GetProvider().GetState(AssetPath, EStateCacheUsage::Use);
			const FSlateBrush* IconBrush = FAppStyle::GetBrush("SourceControl.Generic");
			
			if (SourceControlState.IsValid())
			{
				if (SourceControlState->IsCheckedOut())
				{
					IconBrush = FAppStyle::GetBrush("SourceControl.CheckedOut");
				}
				else if (SourceControlState->IsModified())
				{
					IconBrush = FAppStyle::GetBrush("SourceControl.Modified");
				}
				else if (SourceControlState->IsSourceControlled())
				{
					IconBrush = FAppStyle::GetBrush("SourceControl.CheckedIn");
				}
				else
				{
					IconBrush = FAppStyle::GetBrush("SourceControl.NotUnderSourceControl");
				}
			}

			return SNew(SImage)
				.Image(IconBrush)
				.ColorAndOpacity(FColor::Transparent);
		}

		return SNullWidget::NullWidget;
    }

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		if (DirtyBrushWidget.IsValid())
		{
			DirtyBrushWidget->SetVisibility(bIsDirty ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

private:
	FReply BorderMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
	{
		if (OnCreateContextMenu.IsBound() && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
		{
		    OnCreateContextMenu.Execute(InGeometry, MouseEvent);
			return FReply::Handled();
		}

		if (MouseButtonDown.IsBound())
		{
		    return MouseButtonDown.Execute(InGeometry, MouseEvent);
		}
		
		return FReply::Unhandled();
	}

	FReply BorderMouseDoubleClicked(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
	{
		if (OnAssetDoubleClicked.IsBound())
		{
			OnAssetDoubleClicked.Execute(InGeometry, MouseEvent);
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	void AddDirtyEventHandler(const FString& PackageName)
	{
		if (Item.IsValid())
		{
			OnPackageDirtyStateChangedHandle = 
			UPackage::PackageDirtyStateChangedEvent.AddLambda(
			[this, PackageName](UPackage* DirtyPackage)
			{
				if (DirtyPackage && DirtyPackage->GetName() == PackageName)
				{
					bIsDirty = DirtyPackage->IsDirty();
				}
			});
		}
	}

	TSharedRef<SWidget> CreateTextWidget(const FString& Text)
	{
		return SNew(STextBlock).Text(FText::FromString(Text));
	}

private:
	bool bIsDirty = false;
    TSharedPtr<FAssetData> Item = nullptr;
	TSharedPtr<SImage> DirtyBrushWidget = nullptr;
	//UPackage* AssetPackage = nullptr;

	FOnAssetRenamed OnAssetRenamed{};
    FOnCreateContextMenu OnCreateContextMenu{};
    FOnAssetDoubleClicked OnAssetDoubleClicked{};
    FOnRegisterEditableText OnRegisterEditableText{};
	FOnAssetMouseButtonDown MouseButtonDown{};
	FDelegateHandle OnPackageDirtyStateChangedHandle{};
};

class SFilterContainerWidget : public SCompoundWidget
{
	/** Delegate for when filters have changed */
	DECLARE_DELEGATE_TwoParams(FOnFilterChanged, const FString& /*FilterName*/, bool /*bIsEnabled*/);
public:

	struct FNamedFilterData
	{
		FString FilterName;
		FString ToolTipText;
	
		FNamedFilterData(const FString& InName, const FString& InToolTipText)
			: FilterName(InName), ToolTipText(InToolTipText)
		{}
	};

	SLATE_BEGIN_ARGS(SFilterContainerWidget){}
		SLATE_ARGUMENT(TArray<FNamedFilterData>, FilterList)
		SLATE_EVENT(FOnFilterChanged, OnFilterChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		FilterList  = InArgs._FilterList;
		OnFilterChangedDelegate = InArgs._OnFilterChanged;
		ChildSlot
		[
			SAssignNew(ScrollBox, SScrollBox)
		];

		RebuildFilters();
	}

	/** Invoked when the filter toggled */
	FOnFilterChanged OnFilterChangedDelegate;
protected:
	class FCustomFilter : public SCompoundWidget
	{
		DECLARE_DELEGATE( FOnRequestDisableAll );

	public:
		SLATE_BEGIN_ARGS(FCustomFilter){}
			SLATE_ARGUMENT(FString, FilterName)
			SLATE_ARGUMENT(FString, ToolTipText)
			SLATE_EVENT(FOnFilterChanged, OnFilterChanged)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			OnFilterChanged = InArgs._OnFilterChanged;
			ToolTipText = InArgs._ToolTipText;
			Construct_Internal(InArgs._FilterName);
		}

	protected:
		void Construct_Internal(const FString& FilterName)
		{
			FilterDispayName = FilterName;

			TSharedPtr<SWidget> ContentWidget = SNew(SBorder)
			.Padding(1.0f)
			.BorderImage(FAppStyle::Get().GetBrush("FilterBar.FilterBackground"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					 SNew(SImage)
					 .Image(FAppStyle::Get().GetBrush("FilterBar.FilterImage"))
					 .ColorAndOpacity(this, &FCustomFilter::GetFilterImageColorAndOpacity)
				]

				+SHorizontalBox::Slot()
				.Padding(TAttribute<FMargin>(this, &FCustomFilter::GetFilterNamePadding))
				.VAlign(VAlign_Center)
				[
					SAssignNew(ToggleButtonPtr, SCheckBox)
					.Style(FAppStyle::Get(), "FilterBar.FilterButton")
					.CheckBoxContentUsesAutoWidth(false)
					.ToolTipText(FText::FromString(ToolTipText))
					.IsChecked(this, &FCustomFilter::IsChecked)
					.OnCheckStateChanged(this, &FCustomFilter::FilterToggled)
					.OnGetMenuContent(this, &FCustomFilter::GetRightClickMenuContent)
					[
						SNew(STextBlock)
						.Text(FText::FromString(GetFilterDisplayName()))
						.IsEnabled_Lambda([this]{ return bEnabled;})
					]
				]
			];

			ChildSlot
			[
				ContentWidget.ToSharedRef()
			];
		}

		
		/** Returns true if this filter contributes to the combined filter */
		bool IsEnabled() const
		{
			return bEnabled;
		}

		FString GetFilterDisplayName() const
		{
			return FilterDispayName;
		}

		FString GetToolTipText() const
		{
			return ToolTipText;
		}

		void SetToolTipText(const FString& InToolTipText)
		{
			ToolTipText = InToolTipText;
		}

		/** Handler to determine the padding of the checkbox text when it is pressed */
		FMargin GetFilterNamePadding() const
		{
			return ToggleButtonPtr->IsPressed() ? FMargin(4,2,4,0) : FMargin(4,1,4,1);
		}

		/** Handler to determine the "checked" state of the filter checkbox */
		ECheckBoxState IsChecked() const
		{
			return bEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		}

		void FilterToggled(ECheckBoxState NewState)
		{
			bEnabled = NewState == ECheckBoxState::Checked;
			OnFilterChanged.ExecuteIfBound(FilterDispayName, bEnabled);
		}

		/** Handler to determine the color of the checkbox when it is checked */
		FSlateColor GetFilterImageColorAndOpacity() const
		{
			return bEnabled ? FSlateColor(FColor::White) : FAppStyle::Get().GetSlateColor("Colors.Recessed");
		}

		TSharedRef<SWidget> GetRightClickMenuContent()
		{
			FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, NULL);

			MenuBuilder.BeginSection("FilterOptions", LOCTEXT("FilterContextHeading", "Filter Options"));
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("DisableAllFilters", "Disable All Filters"),
					LOCTEXT("DisableAllFiltersTooltip", "Disables all active filters."),
					FSlateIcon(),
					FUIAction( FExecuteAction::CreateSP(this, &FCustomFilter::DisableAllFilters)));
			}
			MenuBuilder.EndSection();

			return MenuBuilder.MakeWidget();
		}
		void DisableAllFilters()
		{
			OnRequestDisableAll.ExecuteIfBound();
		}

	private:
		/** Invoked when the filter toggled */
		FOnFilterChanged OnFilterChanged;
		FOnRequestDisableAll OnRequestDisableAll;

		/** The button to toggle the filter on or off */
		TSharedPtr<SCheckBox> ToggleButtonPtr;

		/** enabled toggle button*/
		bool bEnabled;

		/** Display Filter Name In text block*/
		FString FilterDispayName;

		FString ToolTipText;
	};

private:

	void RebuildFilters()
	{
		if (!ScrollBox.IsValid()) return;

		ScrollBox->ClearChildren();
		FilterWidgets.Empty();
		
		for (const FNamedFilterData& FilterData : FilterList)
		{
			TSharedPtr<FCustomFilter> NewFilter = SNew(FCustomFilter)
				.FilterName(FilterData.FilterName)
				.ToolTipText(FilterData.ToolTipText)
				.OnFilterChanged(OnFilterChangedDelegate);

			ScrollBox->AddSlot()
			.Padding(2.0f)
			[
				NewFilter.ToSharedRef()
			];

			FilterWidgets.Add(NewFilter);
		}
	}

	TSharedPtr<SScrollBox> ScrollBox;
	TArray<FNamedFilterData> FilterList;
	TArray<TSharedPtr<SCompoundWidget>> FilterWidgets;
};

void SAssetCleanerWidget::OnFilterChanged(const FString& FilterName, bool bIsEnabled)
{
	UE_LOG(SAssetCleanerWidgetLog, Warning, TEXT("Filter Changed: %s -> %s"), *FilterName, bIsEnabled ? TEXT("Enabled") : TEXT("Disabled"));

	if (bIsEnabled)
	{
		ActiveAdvancedFilters.Add(FilterName);

		if (FilterName == TEXT("Assets With Metadata"))
		{
			CollectMetadataAssets(); 
		}
		if (FilterName == TEXT("Textures Without Compression"))
		{
			CollectTexturesWithoutCompression();
		}
		if (FilterName == TEXT("Assets With Invalid References"))
		{
			CollectAssetsWithInvalidReferences();
		}
		if (FilterName == TEXT("Textures With Wrong Size (PoTwo Check)"))
		{
			CollectTexturesWithWrongSize();
			UpdateFilteredAssetList(); 
			return;
		}
		if (FilterName == TEXT("Materials With Too Many Instructions"))
		{
			CollectMaterialsInfoManyInstruction();
		}
		if (FilterName == TEXT("Materials With Too Many Expressions"))
		{
			CollectMaterialsInfoManyExpression();
		}
	}
	else
	{
		ActiveAdvancedFilters.Remove(FilterName);
	}

	UpdateFilteredAssetList();
}

void SAssetCleanerWidget::Construct(const FArguments &InArgs)
{
	bCanSupportFocus = true;
	SelectedDirectory = InArgs._CurrentSelectedFolder;

	LoadAssets();
	UpdateFilteredAssetList();
	InitializeAssetTypeComboBox(FilteredDataAssets);
	ColumnOrder.Add(AssetCleanerListColumns::ColumnID_RC);
	ColumnOrder.Add(AssetCleanerListColumns::ColumnID_Name);
	ColumnOrder.Add(AssetCleanerListColumns::ColumnID_Type);
	ColumnOrder.Add(AssetCleanerListColumns::ColumnID_DiskSize);
	ColumnOrder.Add(AssetCleanerListColumns::ColumnID_Path);
	InitializeColumnAdders();
	TSharedPtr<SLayeredImage> FilterImage = CreateFilterImage();
	InitializeAdvancedFilters();

	TArray<SFilterContainerWidget::FNamedFilterData> Filters =
	{
		// --- Assets Related ---
		{ TEXT("Assets Without References"), TEXT("Assets that are not referenced by any other asset in the project.") },
		{ TEXT("Assets With Missing References"), TEXT("Assets that reference other assets which no longer exist.") },
		{ TEXT("Assets With Metadata"), TEXT("Assets that contain metadata which might be unnecessary or outdated.") },
		{ TEXT("Assets With Long Names"), TEXT("Assets with excessively long names that may cause issues in some platforms.") },
		{ TEXT("Assets Outside of Source Control"), TEXT("Assets that are not currently tracked by source control.") },
		{ TEXT("Assets With Redirectors"), TEXT("Assets that are redirectors and may need to be fixed up.") },
		{ TEXT("Assets With Non-Standard Folder Name"), TEXT("Assets located in folders that don't follow naming conventions.") },
		{ TEXT("Assets In Temporary Folders"), TEXT("Assets placed in folders marked as temporary or working folders.") },
		{ TEXT("Assets With Invalid References"), TEXT("Assets that contain broken or invalid references.") },
		{ TEXT("Assets With Circular References"), TEXT("Assets that form circular dependencies, which can cause load issues.") },
		{ TEXT("Assets With Default Name"), TEXT("Assets that still use their auto-generated default names.") },
		{ TEXT("Assets Without Tags"), TEXT("Assets that are not tagged with any metadata or category tags.") },
		{ TEXT("Assets Without LODs"), TEXT("Static meshes that are missing Level of Detail (LOD) versions.") },

		// --- Textures Related ---
		{ TEXT("Textures Without Compression"), TEXT("Textures that do not use any compression, increasing memory usage.") },
		{ TEXT("Textures With Wrong Size (PoTwo Check)"), TEXT("Textures whose dimensions are not powers of two (PoT), which can cause rendering or memory issues.") },

		// --- Materials Related ---
		{ TEXT("Materials With Too Many Instructions"), TEXT("Materials that exceed a safe number of instructions, which can affect performance.") },
		{ TEXT("Materials Without Usage Flags"), TEXT("Materials missing usage flags, which may prevent them from compiling properly for all scenarios.") },
		{ TEXT("Materials With Too Many Expressions"), TEXT("Materials containing too many expression nodes, possibly affecting performance or readability.") },

		// --- Meshes and Skeletal ---
		{ TEXT("Skeletal Meshes Without Physics Asset"), TEXT("Skeletal meshes missing a physics asset, required for collision or simulation.") },
		{ TEXT("Static Meshes Without Collision"), TEXT("Static meshes that lack collision settings or geometry.") },
		{ TEXT("Meshes With Nanite Disabled"), TEXT("Meshes that do not have Nanite enabled, missing potential rendering optimization.") },

		// --- Sounds ---
		{ TEXT("Sound Cues Without Sound"), TEXT("Sound cues that do not reference any sound assets.") },
		{ TEXT("Sound Cues Without Output"), TEXT("Sound cues that do not route audio to any output node.") },
		{ TEXT("Sounds With High File Size"), TEXT("Sound files that are larger than recommended size and may impact memory or loading.") },
		{ TEXT("Sounds Without Attenuation Settings"), TEXT("Sound assets lacking attenuation settings, which affects how sound is heard spatially.") },

		// --- Blueprints ---
		{ TEXT("Blueprints Without Node Logic"), TEXT("Blueprints that have no node logic in the event graph.") },
		{ TEXT("Blueprints With Compile Errors"), TEXT("Blueprints that have compilation errors and cannot run.") },
		{ TEXT("Blueprints Without Construction Script"), TEXT("Blueprints that do not implement a construction script.") },
		{ TEXT("Blueprints Without Comment Blocks"), TEXT("Blueprints with no comment blocks, making them harder to read and maintain.") },

		// --- Levels & Animation ---
		{ TEXT("Levels With Unused Actors"), TEXT("Levels containing actors that are not used and can be safely deleted.") },
		{ TEXT("Anim Sequences Without Skeleton"), TEXT("Animation sequences that do not have an assigned skeleton.") },
		{ TEXT("Montages Without Sections"), TEXT("Animation montages missing section markers for playback control.") },
		{ TEXT("Animations With Missing Notifie"), TEXT("Animation assets missing notifies that may be required for gameplay or effects.") }
	};

	TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(2.0f, 6.0f, 0.0f, 6.0f)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(600.0f) 
				[
					SNew(SMenuAnchor)
					.Placement(EMenuPlacement::MenuPlacement_ComboBoxRight)
					[
						SAssignNew(ListViewSearchBox, SFilterSearchBox)
						.HintText(LOCTEXT("SearchDetailsHint", "Search"))
						.Cursor(EMouseCursor::Hand)
						.OnTextChanged(this, &SAssetCleanerWidget::OnSearchTextChanged)
						.DelayChangeNotificationsWhileTyping(true)
						.AddMetaData<FTagMetaData>(TEXT("Details.Search"))
						.ShowSearchHistory(true)
					]
				]
			]

			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SAssignNew(ComboButton, SComboButton)
				.ComboButtonStyle(&FAppStyle::Get().GetWidgetStyle<FComboButtonStyle>("SimpleComboButtonWithIcon"))
				.ForegroundColor(FSlateColor::UseStyle())
				.ContentPadding(FMargin(1, 0))
				.ButtonContent() [ FilterImage.ToSharedRef()]
				.MenuContent()[ CreateComboButtonContent() ]
			]

		]

		+ SVerticalBox::Slot()
		[
			SAssignNew(AssetListView, SListView<TSharedPtr<FAssetData>>)
			.ListItemsSource(&FilteredDataAssets)
			.OnGenerateRow(this, &SAssetCleanerWidget::GenerateAssetListRow)
			.OnSelectionChanged(this, &SAssetCleanerWidget::OnAssetSelected)
			.SelectionMode(ESelectionMode::Multi)
			.HeaderRow( GenerateHeaderRow() )

		]

		+ SVerticalBox::Slot()
		.FillHeight(0.6f)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]() { return GetSelectedTextBlockInfo();})
		];

	TSharedPtr<SSplitter> Splitter = SNew(SSplitter)
		+ SSplitter::Slot()
		.SizeRule(SSplitter::ESizeRule::SizeToContent)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(0.05f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.VAlign(VAlign_Center)

					[
						SNew(STextBlock)
						.Text(FText::FromString("Advanced Filters"))
					]
			]

			+ SVerticalBox::Slot()
			.Padding(FMargin(2.0f))
			[
				SNew(SFilterContainerWidget)
				.FilterList(Filters)
				.OnFilterChanged(this, &SAssetCleanerWidget::OnFilterChanged)
			]
		]

		+ SSplitter::Slot()
		[
			VerticalBox
		];
	

	ChildSlot
	[
		Splitter.ToSharedRef()
	];

}

void SAssetCleanerWidget::SubscribeToAssetRegistryEvent()
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetCleaner::ModuleName::AssetRegistry);

	const auto SubscribeDelegates = [this, &AssetRegistryModule]()
	{
		AssetAddedDelegateHandle	= AssetRegistryModule.Get().OnAssetAdded().AddRaw(this, &SAssetCleanerWidget::OnAssetAdded);
		AssetRemovedDelegateHandle	= AssetRegistryModule.Get().OnAssetRemoved().AddRaw(this, &SAssetCleanerWidget::OnAssetRemoved);
		AssetRenamedDelegateHandle	= AssetRegistryModule.Get().OnAssetRenamed().AddRaw(this, &SAssetCleanerWidget::OnAssetRenamed);
		AssetUpdateDelegateHandle	= AssetRegistryModule.Get().OnAssetUpdated().AddRaw(this, &SAssetCleanerWidget::OnAssetUpdated);
	};

	if (AssetRegistryModule.Get().IsLoadingAssets())
	{
		FilesLoadedHandle = AssetRegistryModule.Get().OnFilesLoaded().AddLambda(
		[this, SubscribeDelegates]()
		{
			SubscribeDelegates();
		});
	}
	else
	{
		SubscribeDelegates();
	}
}

void SAssetCleanerWidget::OnAssetAdded(const FAssetData& NewAssetData)
{
	//LoadAssets();
	//UpdateFilteredAssetList();
}

void SAssetCleanerWidget::OnAssetRemoved(const FAssetData& AssetToRemoved)
{
	//LoadAssets();
	//UpdateFilteredAssetList();
}

void SAssetCleanerWidget::OnAssetRenamed(const FAssetData& NewAssetData, const FString& Name)
{
	//LoadAssets();
	//UpdateFilteredAssetList();
}

void SAssetCleanerWidget::OnAssetUpdated(const FAssetData& AssetData)
{
	// LoadAssets();
	// UpdateFilteredAssetList();
}

void SAssetCleanerWidget::OnSearchTextChanged(const FText& InText)
{
	SearchText.Set(InText);
	UpdateFilteredAssetList();
}

 FText SAssetCleanerWidget::GetSelectedTextBlockInfo() const
 {
	const FString SelectedStrItems = GetAssetListSelectedItem().Num() > 0
		? FString::Printf(TEXT("(%d selected)"), GetAssetListSelectedItem().Num())
		: TEXT("");

	return FText::FromString(FString::Printf(TEXT("   %d items %s"), FilteredDataAssets.Num(), *SelectedStrItems));
 }

TSharedRef<SWidget> SAssetCleanerWidget::CreateComboButtonContent()
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection*/ false, nullptr);

	MenuBuilder.BeginSection("ResetSection", FText::FromString("Actions"));
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Reset Filters"),
			FText::FromString("Clear all selected filters."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this] ()
				{
					ActiveFilters.Empty();
					UpdateFilteredAssetList();
				}))
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.AddWidget(
		SNew(SBox)
		.Padding(FMargin(5.f, 7.f))
		[
			SNew(SSeparator)
		],
		FText::GetEmpty());

	for(TSharedPtr<FString> FilterItem : ComboBoxAssetListItems)
	{
		const FString FilterName = *FilterItem;
		
		FUIAction Action(
			FExecuteAction::CreateLambda([this, FilterName] ()
				{
					if(ActiveFilters.Contains(FilterName))
					{
						ActiveFilters.Remove(FilterName);
					}
					else
					{
						ActiveFilters.Add(FilterName);
					}

					UpdateFilteredAssetList();
				}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this, FilterName] ()
				{
					return ActiveFilters.Contains(FilterName);
				})
		);

		MenuBuilder.AddMenuEntry(
			FText::FromString(FilterName),
			FText::GetEmpty(),
			FSlateIcon(),
			Action,
			NAME_None,
			EUserInterfaceActionType::ToggleButton 
		);
	}

	return MenuBuilder.MakeWidget();
}

void SAssetCleanerWidget::InitializeAssetTypeComboBox(TArray<TSharedPtr<FAssetData>> AssetDataList)
{
	if (!ComboBoxAssetListItems.IsEmpty())
	{
		ComboBoxAssetListItems.Reset();
	}

	TSet<FString> UniqueAssetNames;
	for (const auto& AssetData : AssetDataList)
	{
		if (AssetData.IsValid())
		{
			const FString AssetName = AssetData->AssetClassPath.GetAssetName().ToString();
			if (!UniqueAssetNames.Contains(AssetName))
			{
				
				UniqueAssetNames.Add(AssetName);
				ComboBoxAssetListItems.Add(MakeShared<FString>(AssetName));
			}
		}
	}

	// Sorting asset by name 
	ComboBoxAssetListItems.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B)
		{ 
			return *A < *B; 
		});

	// Debug !!!
	UE_LOG(LogTemp, Warning, TEXT("%s FilteredDataAssets: %i"), *FString(__FUNCTION__), FilteredDataAssets.Num());
}

bool SAssetCleanerWidget::HasCycle(const FAssetData& Asset)
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	const FAssetIdentifier StartId(Asset.PackageName);

	// Этап 1: собрать все зависимости
	TSet<FAssetIdentifier> TempVisited;
	TArray<FAssetIdentifier> AllIdentifiersToScan;

	TFunction<void(const FAssetIdentifier&)> CollectAllDeps;
	CollectAllDeps = [&](const FAssetIdentifier& CurrentId)
	{
		if (TempVisited.Contains(CurrentId)) return;
		TempVisited.Add(CurrentId);
		AllIdentifiersToScan.Add(CurrentId);

		TArray<FAssetDependency> Deps;
		if (AssetRegistry.GetDependencies(CurrentId, Deps,
			UE::AssetRegistry::EDependencyCategory::Package,
			UE::AssetRegistry::FDependencyQuery(UE::AssetRegistry::EDependencyQuery::Hard)))
		{
			for (const FAssetDependency& Dep : Deps)
			{
				CollectAllDeps(Dep.AssetId);
			}
		}
	};
	CollectAllDeps(StartId);

	FScopedSlowTask SlowTask((float)AllIdentifiersToScan.Num(), FText::FromString(TEXT("Scanning for circular references...")));
	SlowTask.MakeDialog(true);

	TSet<FAssetIdentifier> Visited;
	TSet<FAssetIdentifier> RecursionStack;

	TFunction<bool(const FAssetIdentifier&)> HasCycle;
	HasCycle = [&](const FAssetIdentifier& CurrentId) -> bool
	{
		if (RecursionStack.Contains(CurrentId)) return true;
		if (Visited.Contains(CurrentId)) return false;

		Visited.Add(CurrentId);
		RecursionStack.Add(CurrentId);

		TArray<FAssetDependency> Dependencies;
		if (AssetRegistry.GetDependencies(CurrentId, Dependencies,
			UE::AssetRegistry::EDependencyCategory::Package,
			UE::AssetRegistry::FDependencyQuery(UE::AssetRegistry::EDependencyQuery::Hard)))
		{
			for (const FAssetDependency& Dep : Dependencies)
			{
				if (HasCycle(Dep.AssetId))
				{
					return true;
				}
			}
		}

		RecursionStack.Remove(CurrentId);
		return false;
	};

	for (const FAssetIdentifier& Id : AllIdentifiersToScan)
	{
		if (SlowTask.ShouldCancel()) break;

		SlowTask.EnterProgressFrame(1.f, FText::FromString(Id.ToString()));

		
		Visited.Reset();
		RecursionStack.Reset();

		if (HasCycle(Id))
		{
			return true;
		}
	}

	return false;
}

void SAssetCleanerWidget::CollectAssetsWithInvalidReferences()
{
	AssetsWithInvalidReferences.Empty();

	FScopedSlowTask SlowTask(StoredAssetList.Num(), FText::FromString(TEXT("Checking assets for invalid references...")));
	SlowTask.MakeDialog(true);

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	for (const TSharedPtr<FAssetData>& Asset : StoredAssetList)
	{
		SlowTask.EnterProgressFrame(1.f);

		if (!Asset.IsValid()) continue;

		TArray<FAssetIdentifier> References;
		AssetRegistry.GetReferencers(Asset->GetPrimaryAssetId(),
			References, UE::AssetRegistry::EDependencyCategory::All);

		for (const FAssetIdentifier& Ref : References)
		{
			// ignore self-reference
			if (Ref == Asset->GetPrimaryAssetId()) continue;

			FString PackageName = Ref.PackageName.ToString();
			if (!FPackageName::DoesPackageExist(PackageName))
			{
				AssetsWithInvalidReferences.Add(Asset->PackageName);
				break;
			}
		}
	}
}

void SAssetCleanerWidget::CollectTexturesWithWrongSize()
{
	TexturesWithWrongSize.Empty();

	TArray<TSharedPtr<FAssetData>> TextureAssets;
	for (const TSharedPtr<FAssetData>& Asset : StoredAssetList)
	{
		if (Asset.IsValid() && Asset->AssetClassPath == UTexture2D::StaticClass()->GetClassPathName())
		{
			TextureAssets.Add(Asset);
		}
	}

	FScopedSlowTask SlowTask(TextureAssets.Num(), FText::FromString(TEXT("Scanning textures with wrong size (Non-Po2)...")));
	SlowTask.MakeDialog(true);

	auto IsPowerOfTwo = [](int32 Value) -> bool {
		return Value > 0 && (Value & (Value - 1)) == 0;
	};

	for (const TSharedPtr<FAssetData>& Asset : TextureAssets)
	{
		SlowTask.EnterProgressFrame(1);

		UObject* LoadedObject = Asset->GetAsset();
		if (!LoadedObject) continue;

		if (UTexture2D* Texture = Cast<UTexture2D>(LoadedObject))
		{
			int32 Width = Texture->GetSizeX();
			int32 Height = Texture->GetSizeY();

			if (!IsPowerOfTwo(Width) || !IsPowerOfTwo(Height))
			{
				TexturesWithWrongSize.Add(Asset->PackageName);
			}
		}
	}
}

void SAssetCleanerWidget::CollectMaterialsInfoManyInstruction()
{
	FilteredMaterials.Empty();

	FScopedSlowTask SlowTask(StoredAssetList.Num(), 
		FText::FromString(TEXT("Scanning Materials With Too Many Instructions...")));
	SlowTask.MakeDialog(true);

	constexpr int32 InstructionLimit = 500;

	for (const TSharedPtr<FAssetData>& Asset : StoredAssetList)
	{
		SlowTask.EnterProgressFrame(1.f, FText::FromString(Asset.Get()->AssetName.ToString()));
		if (!Asset.IsValid()) continue;

		UObject* LoadedObject = Asset.Get()->GetAsset();
		UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(LoadedObject);
		
		if (!MaterialInterface) continue;
		const FMaterialStatistics MaterialStats = UMaterialEditingLibrary::GetStatistics(MaterialInterface);

		if ((MaterialStats.NumVertexShaderInstructions > InstructionLimit) || 
			(MaterialStats.NumPixelShaderInstructions > InstructionLimit))
		{
			FilteredMaterials.Add(Asset->PackageName);
		}
	}

}
void SAssetCleanerWidget::CollectMaterialsInfoManyExpression()
{
	FilteredMaterials.Empty();

	FScopedSlowTask SlowTask(StoredAssetList.Num(), 
		FText::FromString(TEXT("Scanning Materials With Too Many Samplers...")));
	SlowTask.MakeDialog(true);

	constexpr int32 ExpressionLimit = 100;

	for (const TSharedPtr<FAssetData>& Asset : StoredAssetList)
	{
		SlowTask.EnterProgressFrame(1.f, FText::FromString(Asset.Get()->AssetName.ToString()));
		if (!Asset.IsValid()) continue;

		UObject* LoadedObject = Asset.Get()->GetAsset();
		UMaterial* Material = Cast<UMaterial>(LoadedObject);
		if (!Material) continue;
		
		const int32 MaterialExpressionCount = UMaterialEditingLibrary::GetNumMaterialExpressions(Material);
		if (MaterialExpressionCount > ExpressionLimit)
		{
			FilteredMaterials.Add(Asset->PackageName);
		}
	}
}

void SAssetCleanerWidget::CollectMetadataAssets()
{
	AssetsWithMetadata.Empty();

	FScopedSlowTask SlowTask(StoredAssetList.Num(), FText::FromString(TEXT("Collecting assets with metadata...")));
	SlowTask.MakeDialog(true);

	for(const TSharedPtr<FAssetData>& Asset : StoredAssetList)
	{
		SlowTask.EnterProgressFrame(1);

		if(!Asset.IsValid()) continue;

		FSoftObjectPath SoftPath = Asset->ToSoftObjectPath();
		UObject* LoadedObject = SoftPath.ResolveObject();

		if(!LoadedObject)
		{
			LoadedObject = SoftPath.TryLoad();
		}

		if(!LoadedObject || LoadedObject->HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad))
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping %s — not fully loaded."), *Asset->GetObjectPathString());
			continue;
		}

		if(const TMap<FName, FString>* MetaMap = UMetaData::GetMapForObject(LoadedObject))
		{
			if(MetaMap->Num() > 0)
			{
				AssetsWithMetadata.Add(Asset->PackageName);
			}
		}
	}
}

void SAssetCleanerWidget::InitializeAdvancedFilters()
{
	AdvancedFilterPredicates = 
	{
		{ TEXT("Assets Without References"), [](const FAssetData& Asset) -> bool {
			IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

			TArray<FName> Referencers;
			AssetRegistry.GetReferencers(Asset.PackageName, Referencers);

			return Referencers.Num() == 0;
		}},
		{ TEXT("Assets With Missing References"), [](const FAssetData& Asset) -> bool {
			IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

			TArray<FAssetDependency> Dependencies;
			const FAssetIdentifier AssetId(Asset.PackageName);

			const bool bGotDeps = AssetRegistry.GetDependencies(
				AssetId,
				Dependencies,
				UE::AssetRegistry::EDependencyCategory::Package,
				UE::AssetRegistry::FDependencyQuery(UE::AssetRegistry::EDependencyQuery::Hard)
			);

			if (!bGotDeps) return false;

			for (const FAssetDependency& Dependency : Dependencies)
			{
				TArray<FAssetData> DependentAssets;
				AssetRegistry.GetAssetsByPackageName(Dependency.AssetId.PackageName, DependentAssets);
				if (DependentAssets.Num() == 0)
				{
					return true;
				}
			}

			return false;
		}},
		{ TEXT("Assets With Metadata"), [this](const FAssetData& Asset) -> bool {
			
			return AssetsWithMetadata.Contains(Asset.PackageName);
		}},
		{ TEXT("Assets With Long Names"), [](const FAssetData& Asset) -> bool {
			return Asset.AssetName.ToString().Len() > AssetCleaner::MaxNameLength; // Пример длины
		}},
		{ TEXT("Assets Outside of Source Control"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверить интеграцию с Source Control
			return false;
		}},
		{ TEXT("Assets With Redirectors"), [](const FAssetData& Asset) -> bool {
			return Asset.AssetClassPath.GetAssetName() == TEXT("Redirector");
		}},
		{ TEXT("Assets With Non-Standard Folder Name"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка папки
			return false;
		}},
		{ TEXT("Assets In Temporary Folders"), [](const FAssetData& Asset) -> bool {
			const FString Path = Asset.PackagePath.ToString();
			return Path.Contains(TEXT("/Temp")) || Path.Contains(TEXT("/Temporary"));
		}},
		{ TEXT("Assets With Invalid References"), [this](const FAssetData& Asset) -> bool {
			return AssetsWithInvalidReferences.Contains(Asset.PackageName);
		}},
		{ TEXT("Assets With Circular References"), [this](const FAssetData& Asset) -> bool {
			return HasCycle(Asset);
		}},
		{ TEXT("Assets With Default Name"), [](const FAssetData& Asset) -> bool {
			const FString Name = Asset.AssetName.ToString();
			return Name.StartsWith(TEXT("New")) 
				|| Name.StartsWith(TEXT("My"));
		}},
		{ TEXT("Textures Without Compression"), [this](const FAssetData& Asset) -> bool {
			return TexturesWithoutCompression.Contains(Asset.PackageName);
		}},
		{ TEXT("Textures With Wrong Size (PoTwo Check)"), [this](const FAssetData& Asset) -> bool {
			return TexturesWithWrongSize.Contains(Asset.PackageName);
		}},
		{ TEXT("Assets Without Tags"), [](const FAssetData& Asset) -> bool {
			return Asset.TagsAndValues.Num() == 0;
		}},
		{ TEXT("Materials With Too Many Instructions"), [this](const FAssetData& Asset) -> bool {
			return FilteredMaterials.Contains(Asset.PackageName);
		}},
		{ TEXT("Materials Without Usage Flags"), [](const FAssetData& Asset) -> bool {
			return false;
		}},

		{ TEXT("Materials With Too Many Expressions"), [this](const FAssetData& Asset) -> bool {
			return FilteredMaterials.Contains(Asset.PackageName);
		}},

		{ TEXT("Skeletal Meshes Without Physics Asset"), [](const FAssetData& Asset) -> bool {
			// TODO: Загрузить SkeletalMesh и проверить PhysicsAsset
			return false;
		}},
		{ TEXT("Static Meshes Without Collision"), [](const FAssetData& Asset) -> bool {
			// TODO: Загрузить StaticMesh и проверить collision
			return false;
		}},
		{ TEXT("Meshes With Nanite Disabled"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка на Nanite
			return false;
		}},
		{ TEXT("Assets Without LODs"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка LOD
			return false;
		}},
		{ TEXT("Sound Cues Without Sound"), [](const FAssetData& Asset) -> bool {
			// TODO: Загрузить SoundCue и проверить наличие Wave
			return false;
		}},
		{ TEXT("Blueprints Without Node Logic"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка графа на узлы
			return false;
		}},
		{ TEXT("Blueprints With Compile Errors"), [](const FAssetData& Asset) -> bool {
			// TODO: Загрузить BP и проверить CompileStatus
			return false;
		}},
		{ TEXT("Blueprints Without Construction Script"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка наличия Construction Script
			return false;
		}},
		{ TEXT("Blueprints Without Comment Blocks"), [](const FAssetData& Asset) -> bool {
			// TODO: Анализ узлов на наличие комментов
			return false;
		}},
		{ TEXT("Sound Cues Without Output"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка SoundCue Output
			return false;
		}},
		{ TEXT("Sounds With High File Size"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка размера
			return false;
		}},
		{ TEXT("Sounds Without Attenuation Settings"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка Attenuation
			return false;
		}},
		{ TEXT("Levels With Unused Actors"), [](const FAssetData& Asset) -> bool {
			// TODO: Загрузить Level и проверить Actors
			return false;
		}},
		{ TEXT("Anim Sequences Without Skeleton"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка Skeleton в AnimSequence
			return false;
		}},
		{ TEXT("Montages Without Sections"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка Sections в Montage
			return false;
		}},
		{ TEXT("Animations With Missing Notifie"), [](const FAssetData& Asset) -> bool {
			// TODO: Проверка Notifies
			return false;
		}}
	};
}



void SAssetCleanerWidget::CollectTexturesWithoutCompression()
{
    TexturesWithoutCompression.Empty();

    IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    FTopLevelAssetPath TextureClassPath(TEXT("/Script/Engine.Texture2D"));
    
    TArray<FAssetData> AllTextures;
    AssetRegistry.GetAssetsByClass(TextureClassPath, AllTextures);

    FScopedSlowTask SlowTask(AllTextures.Num(), FText::FromString(TEXT("Scanning Textures Without Compression...")));
    SlowTask.MakeDialog(true);

    for (const FAssetData& TextureAsset : AllTextures)
    {
        SlowTask.EnterProgressFrame(1.f, FText::FromString(TextureAsset.AssetName.ToString()));

        if (UTexture2D* Texture2D = Cast<UTexture2D>(TextureAsset.GetAsset()))
        {
			if (Texture2D->CompressionSettings == TextureCompressionSettings::TC_VectorDisplacementmap || 
				Texture2D->CompressionSettings == TextureCompressionSettings::TC_Grayscale)
			{
				TexturesWithoutCompression.Add(TextureAsset.PackageName);
			}
		}
	}
}


TArray<TSharedPtr<FAssetData>> SAssetCleanerWidget::GetAssetListSelectedItem() const
{
	TArray<TSharedPtr<FAssetData>> SelectedItems;
	AssetListView->GetSelectedItems(SelectedItems);

	return SelectedItems;
}

TSharedPtr<SLayeredImage> SAssetCleanerWidget::CreateFilterImage()
{
	return SNew(SLayeredImage)
		.Image(FAppStyle::Get().GetBrush("Icons.Filter"))
		.ColorAndOpacity(FSlateColor(FColor::White));
}

void SAssetCleanerWidget::OpenReferenceViewer()
{
	if (!IsSelectedAssetValid()) return;

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(*SelectedAsset.Get());

	AssetCleaner::Private::ProcessAssetData(AssetDataArray, [](const TArray<FAssetIdentifier>& AssetIdentifiers)
		{
			IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetIdentifiers);
		});
}

void SAssetCleanerWidget::OnAssetSelected(TSharedPtr<FAssetData> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		SelectedAsset = SelectedItem;
		if (!SelectedItem.IsValid())
		{
			return;
		}

		const TArray<TSharedPtr<FAssetData>> SelectedItems = GetAssetListSelectedItem();

		if (SelectedItems.Num() > 1)
		{
			bCanRename = false;
		}
		else if (SelectedItems.Num() == 1)
		{
			bCanRename = true;
		}
	}
}

FORCEINLINE FSlateFontInfo SAssetCleanerWidget::GetWidgetTextFont() const 
{
	return FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
}

const FSlateBrush* SAssetCleanerWidget::GetRevisionControlColumnIconBadge() const
{
	if (ISourceControlModule::Get().IsEnabled())
	{
		return FRevisionControlStyleManager::Get().GetBrush("RevisionControl.Icon.ConnectedBadge");
	}
	else
	{
		return nullptr;
	}
}

void SAssetCleanerWidget::LoadAssets()
{
	UE_LOG(SAssetCleanerWidgetLog, Log, TEXT("Start Loading Assets "));

	if (SelectedDirectory.IsEmpty()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected directory is empty."));
		return;
	}

	const FAssetRegistryModule& AssetRegistryModule = 
	FModuleManager::GetModuleChecked<FAssetRegistryModule>(AssetCleaner::ModuleName::AssetRegistry);
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FString RelativePath = SelectedDirectory;
	TArray<FAssetData> AssetDataArray;
	FName PathName(*RelativePath);
	AssetRegistry.GetAssetsByPath(PathName, AssetDataArray, true);

	if (AssetDataArray.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No assets found in the directory: %s"), *SelectedDirectory);
		return;
	}

	for (const FAssetData& AssetData : AssetDataArray)
	{
		const FString AssetPath = AssetData.PackagePath.ToString();
		if (!AssetCleaner::Private::IsExcludedFolder(AssetPath))
		{
			StoredAssetList.Add(MakeShared<FAssetData>(AssetData));
		}
	}

	StoredAssetList.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B) 
	{
		return A->AssetName.LexicalLess(B->AssetName);
	});

	UE_LOG(SAssetCleanerWidgetLog, Log, TEXT("Found %d assets in directory: %s"), StoredAssetList.Num(), *SelectedDirectory);
}

void SAssetCleanerWidget::UpdateFilteredAssetList()
{
	FilteredDataAssets.Empty();
	const FString SearchString = SearchText.Get().ToString();
	const bool bFilterByType = ActiveFilters.Num() > 0;
	const bool bHasAdvancedFilters = ActiveAdvancedFilters.Num() > 0;

	for (const TSharedPtr<FAssetData>& AssetData : StoredAssetList)
	{
		if (!AssetData.IsValid()) continue;

		const FString& AssetName = AssetData->AssetName.ToString();
		const FString AssetClassName = AssetData->AssetClassPath.GetAssetName().ToString();

		const bool bNameMatches = SearchString.IsEmpty() || AssetName.Contains(SearchString);
		const bool bTypeMatches = !bFilterByType || ActiveFilters.Contains(AssetClassName);

		// Advanced filters now use OR logic
		bool bAdvancedMatches = !bHasAdvancedFilters;

		if (bHasAdvancedFilters)
		{
			for (const FString& FilterName : ActiveAdvancedFilters)
			{
				const TFunction<bool(const FAssetData&)>* Predicate = AdvancedFilterPredicates.Find(FilterName);
				if (Predicate && (*Predicate)(*AssetData))
				{
					bAdvancedMatches = true;
					break;
				}
			}
		}

		if (bNameMatches && bTypeMatches && bAdvancedMatches)
		{
			FilteredDataAssets.Add(AssetData);
		}
	}

	if (AssetListView.IsValid())
	{
		AssetListView->RequestListRefresh();
	}
}

void SAssetCleanerWidget::InitializeColumnAdders()
{
	ColumnAdders.Add(AssetCleanerListColumns::ColumnID_RC, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		if (bShowRevisionColumn)
		{
			HeaderRow->AddColumn(CreateRevisionControlColumn());
		}
	});

	ColumnAdders.Add(AssetCleanerListColumns::ColumnID_Name, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		AddColumnToHeader(HeaderRow, AssetCleanerListColumns::ColumnID_Name, TEXT("Name"), 0.4f);
	});

	ColumnAdders.Add(AssetCleanerListColumns::ColumnID_Type, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		if (bShowTypeColumn)
		{
			AddColumnToHeader(HeaderRow, AssetCleanerListColumns::ColumnID_Type, TEXT("Type"), 0.3f);
		}
	});

	ColumnAdders.Add(AssetCleanerListColumns::ColumnID_DiskSize, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		if (bShowDiskSizeColumn)
		{
			AddColumnToHeader(HeaderRow, AssetCleanerListColumns::ColumnID_DiskSize, TEXT("DiskSize"), 0.15f);
		}
	});

	ColumnAdders.Add(AssetCleanerListColumns::ColumnID_Path, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		if (bShowPathColumn)
		{
			AddColumnToHeader(HeaderRow, AssetCleanerListColumns::ColumnID_Path, TEXT("Path"), 0.3f);
		}
	});
}

SHeaderRow::FColumn::FArguments SAssetCleanerWidget::CreateRevisionControlColumn()
{
	TSharedRef<SLayeredImage> RevisionControlColumnIcon = SNew(SLayeredImage)
	.ColorAndOpacity(FSlateColor::UseForeground())
	.Image(FRevisionControlStyleManager::Get().GetBrush("RevisionControl.Icon"));

    RevisionControlColumnIcon->AddLayer(TAttribute<const FSlateBrush*>::CreateSP(this,
        &SAssetCleanerWidget::GetRevisionControlColumnIconBadge));

    return SHeaderRow::Column(AssetCleanerListColumns::ColumnID_RC)
        .FixedWidth(30.0f)
        .HAlignHeader(HAlign_Center)
        .VAlignHeader(VAlign_Center)
        .HAlignCell(HAlign_Center)
        .VAlignCell(VAlign_Center)
        .DefaultLabel(LOCTEXT("Column_RC", "Revision Control"))
        [
            RevisionControlColumnIcon
        ];
}

void SAssetCleanerWidget::AddColumnToHeader(TSharedPtr<SHeaderRow> InHeaderRow, const FName& ColumnId, const FString& Label, const float FillWidth)
{
	InHeaderRow->AddColumn(SHeaderRow::FColumn::FArguments()
		.ColumnId(ColumnId)
		.DefaultLabel(FText::FromString(Label))
		.FillWidth(FillWidth)
		.HeaderContent()
		[
			SNew(SBorder)
			.BorderBackgroundColor(FSlateColor(FColor::Transparent))
			.OnMouseButtonDown(this, &SAssetCleanerWidget::ColumnButtonClicked)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
			]
		]);
}

TSharedRef<SHeaderRow> SAssetCleanerWidget::GenerateHeaderRow()
{
	TSharedRef<SHeaderRow> HeaderRow = SNew(SHeaderRow);
	const TSharedPtr<SHeaderRow> HeaderRowPtr = HeaderRow;

	for (const FName& ColumnId : ColumnOrder)
	{
		if (const TFunction<void(TSharedPtr<SHeaderRow>)>* AddFunc = ColumnAdders.Find(ColumnId))
		{
			(*AddFunc)(HeaderRowPtr);
		}
	}

	return HeaderRow;
}

void SAssetCleanerWidget::UpdateColumnVisibility()
{
	TSharedPtr<SHeaderRow> HeaderRow = AssetListView->GetHeaderRow();
	HeaderRow->ClearColumns();

	// Add columns in order
	for (const FName& ColumnId : ColumnOrder)
	{
		if (const TFunction<void(TSharedPtr<SHeaderRow>)>* AddFunc = ColumnAdders.Find(ColumnId))
		{
			(*AddFunc)(HeaderRow); // Invoke the column addition
		}
	}

	AssetListView->RequestListRefresh();
}

FReply SAssetCleanerWidget::ColumnButtonClicked(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FMenuBuilder MenuBuilder(true, nullptr);

		MenuBuilder.BeginSection("AdditionalActions", LOCTEXT("AdditionalActionsSection", "Additional Actions"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ToggleAllColumns", "Hide All Custom Columns"),
				LOCTEXT("ToggleAllColumnsTooltip", "Hide or show all custom columns at once"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([this]()
					{
						const bool bShouldHide = bShowDiskSizeColumn || bShowPathColumn 
							|| bShowTypeColumn || bShowRevisionColumn;

						bShowDiskSizeColumn = !bShouldHide;
						bShowPathColumn		= !bShouldHide;
						bShowTypeColumn		= !bShouldHide;
						bShowRevisionColumn = !bShouldHide;

						UpdateColumnVisibility();
					}),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this]()
					{
						// Checked if all columns are hidden
						return !bShowDiskSizeColumn && !bShowPathColumn && !bShowTypeColumn && !bShowRevisionColumn;
					})
				),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("ColumnVisibility", LOCTEXT("ColumnVisibilitySection", "Visible Columns"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ShowDiskSize", "Show Type"),
				LOCTEXT("ShowDiskSizeTooltip", "Toggle the visibility of the Disk Size column"),
				FSlateIcon(), 
				FUIAction(
					FExecuteAction::CreateLambda([this]() { bShowTypeColumn = !bShowTypeColumn; UpdateColumnVisibility(); }),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this]() { return bShowTypeColumn; })
				),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("ShowPath", "Show Path"),
				LOCTEXT("ShowPathTooltip", "Toggle the visibility of the Path column"),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateLambda([this]() { bShowPathColumn = !bShowPathColumn; UpdateColumnVisibility();}),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this]() { return bShowPathColumn; })
				),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
			
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ShowDiskSize", "Show Disk Size"),
				LOCTEXT("ShowDiskSizeTooltip", "Toggle the visibility of the Disk Size column"),
				FSlateIcon(), 
				FUIAction(
					FExecuteAction::CreateLambda([this]() { bShowDiskSizeColumn = !bShowDiskSizeColumn; UpdateColumnVisibility(); }),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this]() { return bShowDiskSizeColumn; })
				),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("RevisionControl", "Revision Control"),
				LOCTEXT("RevisionControlTooltip", "Toggle the visibility of the Revision control column"),
				FSlateIcon(), 
				FUIAction(
					FExecuteAction::CreateLambda([this]() { bShowRevisionColumn = !bShowRevisionColumn; UpdateColumnVisibility(); }),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this]() { return bShowRevisionColumn; })
				),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
		}
		MenuBuilder.EndSection();

		FSlateApplication::Get().PushMenu(
			SharedThis(this),
			FWidgetPath(),
			MenuBuilder.MakeWidget(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

bool SAssetCleanerWidget::CanAction() const
{
	return bCanRename;
}

void SAssetCleanerWidget::CreateContextMenuFromDataAsset(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FMenuBuilder MenuBuilder(true, nullptr);

		MenuBuilder.BeginSection("Common", LOCTEXT("CommonSection", "Common"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Edit...             "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::Edit),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::OpenSelectedDataAssetInEditor),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Duplicate             "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::Duplicate),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::OpenSelectedDataAssetInEditor),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("RenameMenuEntry", "Rename             "),
				LOCTEXT("RenameMenuTooltip", "Rename this item"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Rename"),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::FocusOnSelectedAsset),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Delete              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Delete"),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::DeleteAsset)));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Save              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::SaveAsset),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::SaveAsset),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));

			MenuBuilder.AddSubMenu(
				LOCTEXT("AssetActionsLabel", "Asset Actions"),
				LOCTEXT("AssetActionsTooltip", "Various actions you can perform on this asset"),
				FNewMenuDelegate::CreateSP(this, &SAssetCleanerWidget::GenerateAssetActionsSubMenu),
				false,
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("Explore", LOCTEXT("ExploreSection", "Explore"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Show In Folder View              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::FindInCB),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::SyncContentBrowserToSelectedAsset),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));
			
			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Show In Explorer              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.ShowInExplorer"),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::OpenSelectedAssetInExplorer),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));

		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("References", LOCTEXT("ReferencesSection", "References"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Copy Reference              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::Copy),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::CopyToClipboard,false),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));
			
			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Copy FilePath              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::Copy),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::CopyToClipboard, true),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Reference Viewer...              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::ReferenceViewer),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::OpenReferenceViewer),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Size Map              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::SizeMap),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::OpenSizeMap),
					FCanExecuteAction::CreateLambda([this]() { return CanAction(); })));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteMenuEntry", "Audit Asset              "),
				LOCTEXT("DeleteMenuTooltip", "Delete this item"),
				FSlateIcon(AssetCleaner::Icons::Audit),
				FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::OpenAuditAsset)));

		}
		MenuBuilder.EndSection();

		FSlateApplication::Get().PushMenu(
			AsShared(),
			FWidgetPath(),
			MenuBuilder.MakeWidget(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
	}
}

void SAssetCleanerWidget::GenerateAssetActionsSubMenu(FMenuBuilder& SubMenuBuilder)
{
	SubMenuBuilder.AddMenuEntry(
		LOCTEXT("DeleteMenuEntry", "Edit Selection In Property Matrix              "),
		LOCTEXT("DeleteMenuTooltip", "Delete this item"),
		FSlateIcon(AssetCleaner::Icons::ReferenceViewer),
		FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::EditSelectionInPropertyMatrix)));
	
	SubMenuBuilder.AddMenuEntry(
		LOCTEXT("ShowMetadata", "Show MetaData              "),
		LOCTEXT("ShowMetaDataToolTip", "Delete this item"),
		FSlateIcon(AssetCleaner::Icons::ReferenceViewer),
		FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::ShowMetaDataDialog)));

	//SubMenuBuilder.AddMenuEntry(
	//	LOCTEXT("DeleteMenuEntry", "Audit Asset              "),
	//	LOCTEXT("DeleteMenuTooltip", "Delete this item"),
	//	FSlateIcon(AssetCleaner::Icons::Audit),
	//	FUIAction(FExecuteAction::CreateSP(this, &SAssetCleanerWidget::OpenAuditAsset)));

}

void SAssetCleanerWidget::ShowMetaDataDialog()
{
		for (const TSharedPtr<FAssetData>& AssetData : GetAssetListSelectedItem())
	{
		const UObject* Asset = AssetData.Get()->GetAsset();
		if (Asset)
		{
			const TMap<FName, FString>* TagValues = UMetaData::GetMapForObject(Asset);
			if (TagValues)
			{
				// Create and display a resizable window to display the MetaDataView for each asset with metadata
				const FString Title = FString::Printf(TEXT("Metadata: %s"), *AssetData.Get()->AssetName.ToString());

				TSharedPtr< SWindow > Window = SNew(SWindow)
					.Title(FText::FromString(Title))
					.SupportsMaximize(false)
					.SupportsMinimize(false)
					.MinWidth(500.0f)
					.MinHeight(250.0f)
					[
						SNew(SBorder)
						.Padding(4.f)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SMetaDataView, *TagValues)
						]
					];
				
				FSlateApplication::Get().AddWindow(Window.ToSharedRef());
			}
			else
			{
				FNotificationInfo Info(FText::Format(LOCTEXT("NoMetaDataFound", "No metadata found for asset {0}."), FText::FromString(Asset->GetName())));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);
			}
		}
	}
}

void SAssetCleanerWidget::EditSelectionInPropertyMatrix()
{
	TArray<TSharedPtr<FAssetData>> SelectedAssets = GetAssetListSelectedItem();
	TArray<UObject*> ObjectsForPropertiesMenu;

	const bool bSkipRedirectors = true;

	for (const TSharedPtr<FAssetData>& AssetData : SelectedAssets)
	{
		if (AssetData.IsValid())
		{
			UObject* Asset = AssetData->GetAsset();
			if (Asset)
			{
				ObjectsForPropertiesMenu.Add(Asset);
			}
		}
	}

	if (ObjectsForPropertiesMenu.Num() > 0)
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.CreatePropertyEditorToolkit(TSharedPtr<IToolkitHost>(), ObjectsForPropertiesMenu);
	}
}

void SAssetCleanerWidget::OpenSelectedAssetInExplorer()
{
	if (!IsSelectedAssetValid()) return;

	UObject* AssetObject = SelectedAsset->GetAsset();
	if (!IsValid(AssetObject)) return;

	UPackage* AssetPackage = AssetObject->GetOutermost();
	if (!IsValid(AssetPackage)) return;

	const FString PackageFilePath = FPackageName::LongPackageNameToFilename(
		AssetPackage->GetName(),
		FPackageName::GetAssetPackageExtension()
	);

	if (FPaths::FileExists(PackageFilePath))
	{
		FPlatformProcess::ExploreFolder(*PackageFilePath);
		UE_LOG(LogTemp, Log, TEXT("Opening in Explorer: %s"), *PackageFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Asset file not found on disk: %s"), *PackageFilePath);
	}
}

void SAssetCleanerWidget::OpenAuditAsset()
{
	if (!IsSelectedAssetValid()) return;

	TArray<FName> SelectedAssetPackageNames;
	for (const TSharedPtr<FAssetData> Items : GetAssetListSelectedItem())
	{
		const FName PackageName = Items->PackageName;
		SelectedAssetPackageNames.Add(PackageName);
	}

	IAssetManagerEditorModule::Get().OpenAssetAuditUI(SelectedAssetPackageNames);
}

void SAssetCleanerWidget::CopyToClipboard(bool bCopyPaths)
{
	if (!IsSelectedAssetValid()) return;

	TArray<FAssetData> SelectedPackages;
	SelectedPackages.Add(*SelectedAsset);

	SelectedPackages.Sort([](const FAssetData& One, const FAssetData& Two)
		{
			return One.PackagePath.Compare(Two.PackagePath) < 0;
		});

	const FString ClipboardText = FString::JoinBy(SelectedPackages, LINE_TERMINATOR, [bCopyPaths](const FAssetData& Item)
		{
			if (bCopyPaths)
			{
				const FString ItemFilename = FPackageName::LongPackageNameToFilename(Item.PackageName.ToString(), FPackageName::GetAssetPackageExtension());
				if (FPaths::FileExists(ItemFilename))
				{
					return FPaths::ConvertRelativePathToFull(ItemFilename);
				}
				else
				{
					return FString::Printf(TEXT("%s: No file on disk"), *Item.AssetName.ToString());
				}
			}
			else
			{
				return Item.GetExportTextName();
			}
		});

	FPlatformApplicationMisc::ClipboardCopy(*ClipboardText);
}

void SAssetCleanerWidget::SaveAsset()
{
	if (!IsSelectedAssetValid()) return;

	UObject* AssetObject = SelectedAsset->GetAsset();
	if (!IsValid(AssetObject)) return;

	AssetObject->MarkPackageDirty();

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().AssetCreated(AssetObject);

	UPackage* AssetPackage = AssetObject->GetOutermost();
	if (!IsValid(AssetPackage)) return;

	const FString PackageFileName = FPackageName::LongPackageNameToFilename(
		AssetPackage->GetName(),
		FPackageName::GetAssetPackageExtension()
	);

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = EObjectFlags::RF_NoFlags;
	SaveArgs.Error = GError;
	SaveArgs.SaveFlags = SAVE_NoError;
	SaveArgs.bWarnOfLongFilename = false;

	if (UPackage::SavePackage(AssetPackage, AssetObject, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("Asset saved successfully: %s"), *PackageFileName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save asset: %s"), *PackageFileName);
	}

}

void SAssetCleanerWidget::SyncContentBrowserToSelectedAsset()
{
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	if (!IsSelectedAssetValid()) return;

	ContentBrowserModule.Get().SyncBrowserToAssets({ *SelectedAsset });
}

void SAssetCleanerWidget::OpenSizeMap()
{
	if (!IsSelectedAssetValid()) return;

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(*SelectedAsset.Get());

	AssetCleaner::Private::ProcessAssetData(AssetDataArray, [](const TArray<FAssetIdentifier>& AssetIdentifiers)
		{
			IAssetManagerEditorModule::Get().OpenSizeMapUI(AssetIdentifiers);
		});
}

void SAssetCleanerWidget::DeleteAsset()
{
	TArray<FAssetData> AssetsToDelete;
	TArray<FAssetData> LockedAssets;

	for (const TSharedPtr<FAssetData>& Item : GetAssetListSelectedItem())
	{
		if (!Item.IsValid()) continue;

		const FAssetData AssetData = *Item;
		const FString PackageFilename = USourceControlHelpers::PackageFilename(AssetData.PackageName.ToString());
		const FSourceControlState FileState = USourceControlHelpers::QueryFileState(PackageFilename);

		bool bIsLocked = false;
		if (FileState.bIsValid)
		{
			bIsLocked = FileState.bIsCheckedOut || FileState.bIsCheckedOutOther || (FileState.bIsSourceControlled && !FileState.bCanCheckIn);            
		}

		if (bIsLocked)
		{
			LockedAssets.Add(AssetData);
		}
		else
		{
			AssetsToDelete.Add(AssetData);
		}
	}

	if (LockedAssets.Num() > 0)
	{
		FString LockedAssetsList;
		for (const FAssetData& Asset : LockedAssets)
		{
			LockedAssetsList += FString::Printf(TEXT("\n• %s"), *Asset.AssetName.ToString());
	
			FString PackageFilename = USourceControlHelpers::PackageFilename(Asset.PackageName.ToString());
			FSourceControlState State = USourceControlHelpers::QueryFileState(PackageFilename);

			if (State.bIsCheckedOutOther)
			{
				LockedAssetsList += TEXT(" (Checked out by another user)");
			}
			else if (State.bIsCheckedOut)
			{
				LockedAssetsList += TEXT(" (Checked out by you)");
			}
			else if (!State.bCanCheckIn)
			{
				LockedAssetsList += TEXT(" (Pending review or locked)");
			}
		}

		FMessageDialog::Open(EAppMsgType::Ok,FText::Format(LOCTEXT("CannotDeleteLockedAssets", "Cannot delete assets locked in Revision Control:{0}\n\nPlease check them in or unlock first."),
		FText::FromString(LockedAssetsList)));
	}

	bCanRename = false;
	if (AssetsToDelete.Num() > 0)
	{
		AssetCleaner::Private::DeleteMultiplyAsset(AssetsToDelete);
	}
}

TSharedRef<ITableRow> SAssetCleanerWidget::GenerateAssetListRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerSTable)
{	
	return SNew(SAssetCleanerTableRow, OwnerSTable)
		.Item(Item)
		.OnAssetRenamed(this, &SAssetCleanerWidget::HandleAssetRename)
		.OnCreateContextMenu(this, &SAssetCleanerWidget::CreateContextMenuFromDataAsset)
		.OnAssetDoubleClicked(this, &SAssetCleanerWidget::HandleAssetDoubleClick)
		.OnRegisterEditableText(this, &SAssetCleanerWidget::RegisterEditableText)
		.OnMouseButtonDown(this, &SAssetCleanerWidget::HandleRowMouseButtonDown);
}

FReply SAssetCleanerWidget::HandleRowMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }
    return FReply::Handled(); 
}

void SAssetCleanerWidget::HandleAssetDoubleClick(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OpenSelectedDataAssetInEditor();
	}
}

void SAssetCleanerWidget::OpenSelectedDataAssetInEditor()
{
	const UObject* AssetObject = SelectedAsset->GetAsset();
	if (!AssetObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected Asset Object is not valid "));
		return;
	}

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetObject);
}

void SAssetCleanerWidget::HandleAssetRename(TSharedPtr<FAssetData> AssetData, const FText& InText, ETextCommit::Type CommitMethod)
{
	if (!SelectedAsset.IsValid() || InText.IsEmpty())
	{
		bRenamedProgress = false;
		return;
	}

	if (CommitMethod == ETextCommit::OnEnter)
	{
		UObject* Asset = SelectedAsset->GetAsset();
		if (!IsValid(Asset))
		{
			UE_LOG(LogTemp, Warning, TEXT("Asset is not valid"));
			return;
		}

		FString NewName = InText.ToString();
		FString PackagePath = Asset->GetPathName();
		PackagePath = FPaths::GetPath(PackagePath);
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>(AssetCleaner::ModuleName::AssetTools).Get();
		if (AssetTools.RenameAssets({ FAssetRenameData(Asset, PackagePath, NewName) }))
		{
			bRenamedProgress = false;
			UE_LOG(LogTemp, Log, TEXT("Asset renamed %s"), *Asset->GetName());
		}
	}
}

void SAssetCleanerWidget::FocusOnSelectedAsset()
{
	if (!IsSelectedAssetValid()) return;

	UE_LOG(LogTemp, Warning, TEXT("%s EditableTextWidgets counts %d"), *FString(__FUNCTION__), EditableTextWidgets.Num());

	TSharedPtr<FAssetData> FoundAsset = nullptr;
	for (const TSharedPtr<FAssetData>& DataAsset : StoredAssetList)
	{
		if (!DataAsset->PackageName.IsEqual(SelectedAsset->PackageName)) continue;

		FoundAsset = DataAsset;
		break;
	}

	const TPair<FName, FName> WidgetKey(FoundAsset->PackagePath, FoundAsset->AssetName);

	if (TSharedPtr<SEditableText>* FoundWidget = EditableTextWidgets.Find(WidgetKey))
	{
		if (FoundWidget->IsValid())
		{
			bRenamedProgress = true;
			(*FoundWidget)->SetIsReadOnly(false);
			FSlateApplication::Get().SetKeyboardFocus(*FoundWidget, EFocusCause::SetDirectly);
			EditableTextWidget = *FoundWidget;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget not found in EditableTextMap"));
	}
}

void SAssetCleanerWidget::RegisterEditableText(TSharedPtr<FAssetData> AssetData, TSharedRef<SEditableText> EditableText)
{
	EditableTextWidgets.Add({ AssetData->PackagePath,AssetData->AssetName }, EditableText);
}

bool SAssetCleanerWidget::IsSelectedAssetValid(const FString& CustomMessage) const
{
	if (SelectedAsset.IsValid()) return true;

	const FString ErrorMsg = CustomMessage.IsEmpty() 
		? FString::Printf(TEXT("%s Selected Asset is not valid"), *FString(__FUNCTION__))
		: CustomMessage;

	UE_LOG(LogTemp, Warning, TEXT("%s"), *ErrorMsg);
	return false;
}

#undef LOCTEXT_NAMESPACE
