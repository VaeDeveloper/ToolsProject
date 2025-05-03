// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SDataAssetManagerWidget.h"
#include "Widgets/SWidget.h" 
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Interfaces/IPluginManager.h"
#include "AssetViewUtils.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "SourceControlHelpers.h"
#include "SPositiveActionButton.h"
// #include "StateTree.h"
#include "DataAssetManager.h"
#include "Misc/Attribute.h"                        
#include "HAL/PlatformApplicationMisc.h"
#include "Algo/Transform.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Widgets/Images/SLayeredImage.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWidget.h"
#include "Framework/SlateDelegates.h"
#include "Misc/DataValidation.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Menu/DataAssetManagerMenu.h"
#include "Algo/AnyOf.h"
#include "UObject/AssetRegistryTagsContext.h"
#include "UObject/MetaData.h"
#include "SMetaDataView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Selection.h"
#include "RevisionControlStyle/RevisionControlStyle.h"
#include "UObject/ObjectSaveContext.h"
// Editor-only 
#if WITH_EDITOR
#include "AssetManagerEditorModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DataValidationModule.h"
#include "DeveloperSettings/DataAssetManagerSettings.h"
#include "Editor.h"
#include "Editor/UnrealEd/Classes/Factories/DataAssetFactory.h"
#include "FileHelpers.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "ISettingsModule.h"
#include "Kismet2/SClassPickerDialog.h"
#include "MessageLogModule.h"
#include "ObjectTools.h"
#include "OutputLogModule.h"
#include "ToolMenus.h"
#include "WidgetDrawerConfig.h"
#include "Editor/ContentBrowser/Private/ContentBrowserSingleton.h"
#endif 

/* clang-format off */

#define LOCTEXT_NAMESPACE "SDataAssetManagerWidget"

DEFINE_LOG_CATEGORY_STATIC(SDataAssetManagerWidgetLog, All, All);

namespace DataAssetManager
{
	constexpr float ItemHeigth = 24.0f;
	constexpr float DataAssetFontSize = 10.0f;
	constexpr float SearchBoxHideThreshold = 0.01f;
	constexpr float DefaultSplitterValueWhenVisible = 0.25f;  
	constexpr float SplitterValueWhenHidden = 0.0f;

	namespace Private
	{
		FString GetAssetDiskSize (const FAssetData& AssetData)
		{
			FString PackageFileName;
			if (FPackageName::DoesPackageExist(AssetData.PackageName.ToString(), &PackageFileName))
			{
				const int64 FileSize = IFileManager::Get ().FileSize (*PackageFileName);
				
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

		/**
		 * Retrieves the plugin settings from the default instance of UDataAssetManagerSettings.
		 *
		 * @return A constant pointer to the default UDataAssetManagerSettings instance.
		 */
		static const UDataAssetManagerSettings* GetPluginSettings()
		{
			const UDataAssetManagerSettings* Settings = GetDefault<UDataAssetManagerSettings>();
			return Settings;
		}

		static bool DeleteMultiplyAsset(const TArray<FAssetData>& Assets)
		{
			if (Assets.Num() == 0)
			{
				UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s No assets to delete!"), *FString(__FUNCTION__));
				return false;
			}

			int32 DeletedCount = ObjectTools::DeleteAssets(Assets);
			UE_LOG(SDataAssetManagerWidgetLog, Log, TEXT("%s Deleted %d assets"),*FString(__FUNCTION__), DeletedCount);

			return DeletedCount > 0;
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
		 * Creates a new DataAsset of the specified class in the selected or provided directory.
		 * The function checks the validity of the class and ensures it is a subclass of UDataAsset.
		 * It then attempts to generate a unique name for the new asset by checking for existing assets
		 * with the same base name in the selected directory. The new asset is created using the AssetTools module
		 * and synchronized with the Content Browser to reflect the creation.
		 *
		 * @param AssetClass The class of the DataAsset to create. It must be a subclass of UDataAsset.
		 * @param Directory The directory where the new asset will be created. If no directory is selected,
		 *													the provided directory is used as the default.
		 */
		static void CreateNewDataAsset(UClass* AssetClass, const FString& Directory)
		{
			if (!AssetClass || !AssetClass->IsChildOf(UDataAsset::StaticClass()))
			{
				UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s Invalid class provided for DataAsset creation."), *FString(__FUNCTION__));
				return;
			}

			const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(DataAssetManager::ModuleName::ContentBrowser);
			TArray<FString> SelectedPaths;
			ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedPaths);

			const FString AssetPath = SelectedPaths.Num() > 0 ? SelectedPaths[0] : Directory;
			const FString BaseAssetName = TEXT("NewDataAsset");
			FString ExistingPackageName = AssetPath + TEXT("/") + BaseAssetName;
			ExistingPackageName = FPackageName::ObjectPathToPackageName(ExistingPackageName);

			const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(DataAssetManager::ModuleName::AssetRegistry);
			int32 Suffix = 1;
			while (AssetRegistryModule.Get().GetAssetByObjectPath(FName(*ExistingPackageName)).IsValid())
			{
				ExistingPackageName = AssetPath + TEXT("/") + BaseAssetName + FString::Printf(TEXT("_%d"), Suffix);
				ExistingPackageName = FPackageName::ObjectPathToPackageName(ExistingPackageName);
				Suffix++;
			}

			const FString FinalAssetName = FPaths::GetBaseFilename(ExistingPackageName);
			const FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(DataAssetManager::ModuleName::AssetTools);
			UObject* NewAsset = AssetToolsModule.Get().CreateAsset(FinalAssetName, AssetPath, AssetClass, nullptr);

			if (NewAsset)
			{
				
				UE_LOG(SDataAssetManagerWidgetLog, Log, TEXT("Created new DataAsset: %s"), *ExistingPackageName);
			}
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
	}
}


#pragma region IDataAssetManagerInterface
void SDataAssetManagerWidget::CreateNewDataAsset()
{
	const FString NewAssetDirectory = DataAssetManager::Private::GetPluginSettings()->DefaultAssetCreationDirectory.Path;

	UClass* DataAssetClass = nullptr;

	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	TSharedPtr<DataAssetManager::Private::FAssetClassParentFilter> Filter = MakeShareable(new DataAssetManager::Private::FAssetClassParentFilter);
	Options.ClassFilters.Add(Filter.ToSharedRef());
	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;
	Filter->AllowedChildrenOfClasses.Add(UDataAsset::StaticClass());

	const FText TitleText = LOCTEXT("CreateDataAssetOptions", "Pick Class For Data Asset Instance");
	UClass* ChosenClass = nullptr;
	if (SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UDataAsset::StaticClass()))
	{
		DataAssetClass = ChosenClass;
		UE_LOG(SDataAssetManagerWidgetLog, Log, TEXT("Selected Data Asset Class: %s"), *DataAssetClass->GetName());
		DataAssetManager::Private::CreateNewDataAsset(DataAssetClass, NewAssetDirectory);
	}
}

void SDataAssetManagerWidget::OpenSelectedDataAssetInEditor()
{
	const UObject* AssetObject = SelectedAsset->GetAsset();
	if (!AssetObject)
	{
		UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("Selected Asset Object is not valid "));
		return;
	}

	const UDataAsset* DataAsset = CastChecked<UDataAsset>(AssetObject);
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(DataAsset);
}

void SDataAssetManagerWidget::ToggleDataAssetListVisibility()
{
	float CacheSpliterValue = SplitterValue.Get();
	bIsSlotVisible = !bIsSlotVisible;
	SplitterValue.Set(bIsSlotVisible ? DataAssetManager::DefaultSplitterValueWhenVisible : DataAssetManager::SplitterValueWhenHidden);
}

void SDataAssetManagerWidget::OpenAuditAsset()
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

void SDataAssetManagerWidget::ShowDocumentation()
{
	//FString AbsolutePath = FPaths::ConvertRelativePathToFull(
	//FPaths::Combine(FPaths::ProjectDir(), TEXT("Documents/DataAssetManagerDoc.docx")));
	//
	//if (FPaths::FileExists(AbsolutePath))
	//{
	//	FPlatformProcess::LaunchFileInDefaultExternalApplication(*AbsolutePath);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("File not found: %s"), *AbsolutePath);
	//}
	FPlatformProcess::LaunchURL(
		TEXT("https://docs.google.com/document/d/1RxlCTzxBwLvOreQw0OWlSajq8Fx5KWnwMHdbUQ7FSDs/edit?usp=sharing"), 
		nullptr, 
		nullptr);
}

void SDataAssetManagerWidget::SaveDataAsset()
{
	if (!IsSelectedAssetValid()) return;

	UDataAsset* const DataAsset = CastChecked<UDataAsset>(SelectedAsset->GetAsset());
	DataAsset->MarkPackageDirty();

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(DataAssetManager::ModuleName::AssetRegistry);
	AssetRegistryModule.Get().AssetCreated(DataAsset);
	UPackage* const AssetPackage = DataAsset->GetOutermost();

	const FString PackageFileName = FPackageName::LongPackageNameToFilename(AssetPackage->GetName(), FPackageName::GetAssetPackageExtension());
	if (UPackage::SavePackage(AssetPackage, DataAsset, EObjectFlags::RF_NoFlags, *PackageFileName))
	{
		UE_LOG(SDataAssetManagerWidgetLog, Log, TEXT("DataAsset saved successfully: %s"), *PackageFileName);
	}

}

void SDataAssetManagerWidget::SaveAllData()
{
	if (SaveAllDataAsset())
	{
		UE_LOG(SDataAssetManagerWidgetLog, Log, TEXT("Save All Data"));
	}
}

void SDataAssetManagerWidget::SyncContentBrowserToSelectedAsset()
{
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(DataAssetManager::ModuleName::ContentBrowser);
	if (!IsSelectedAssetValid()) return;

	ContentBrowserModule.Get().SyncBrowserToAssets({ *SelectedAsset });
}

void SDataAssetManagerWidget::CopyToClipboard(bool bCopyPaths)
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

void SDataAssetManagerWidget::OpenReferenceViewer()
{
	if (!IsSelectedAssetValid()) return;

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(*SelectedAsset.Get());

	DataAssetManager::Private::ProcessAssetData(AssetDataArray, [](const TArray<FAssetIdentifier>& AssetIdentifiers)
		{
			IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetIdentifiers);
		});
}

void SDataAssetManagerWidget::OpenSizeMap()
{
	if (!IsSelectedAssetValid()) return;

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(*SelectedAsset.Get());

	DataAssetManager::Private::ProcessAssetData(AssetDataArray, [](const TArray<FAssetIdentifier>& AssetIdentifiers)
		{
			IAssetManagerEditorModule::Get().OpenSizeMapUI(AssetIdentifiers);
		});
}

void SDataAssetManagerWidget::OpenPluginSettings()
{
	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>(DataAssetManager::ModuleName::Settings);
	SettingsModule.ShowViewer("Project", "Plugins", "DataAssetManager");
}

void SDataAssetManagerWidget::ShowSourceControlDialog()
{
	ISourceControlModule::Get().ShowLoginDialog(FSourceControlLoginClosed(), ELoginWindowMode::Modeless);
}
void SDataAssetManagerWidget::RestartPlugin()
{
	FDataAssetManagerModule& Module = FModuleManager::LoadModuleChecked<FDataAssetManagerModule>(DataAssetManager::ModuleName::DataAssetManager);
	Module.RestartWidget();
}
void SDataAssetManagerWidget::OpenMessageLogWindow()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(DataAssetManager::ModuleName::MessageLog);
	MessageLogModule.OpenMessageLog("AssetCheck");
}

void SDataAssetManagerWidget::OpenOutputLogWindow()
{
	FOutputLogModule& OutputLogModule = FModuleManager::LoadModuleChecked<FOutputLogModule>(DataAssetManager::ModuleName::OutputLog);
	OutputLogModule.OpenOutputLog();
}

bool SDataAssetManagerWidget::CanRename() const
{
	return bCanRename;
}
#pragma endregion IDataAssetManagerInterface

namespace DataAssetListColumns
{
	/** IDs for list columns */
	static const FName ColumnID_RC("RevisionControl");
	static const FName ColumnID_Name("Name");
	static const FName ColumnID_Type("Type");
	static const FName ColumnID_DiskSize("DiskSize");
	static const FName ColumnID_Path("Path");
}

class SDataAssetTableRow : public SMultiColumnTableRow<TSharedPtr<FAssetData>>
{
public:
	DECLARE_DELEGATE_ThreeParams(FOnAssetRenamed, TSharedPtr<FAssetData>, const FText&, ETextCommit::Type);
    DECLARE_DELEGATE_TwoParams(FOnCreateContextMenu, const FGeometry&, const FPointerEvent&);
    DECLARE_DELEGATE_TwoParams(FOnAssetDoubleClicked, const FGeometry&, const FPointerEvent&);
    DECLARE_DELEGATE_TwoParams(FOnRegisterEditableText, TSharedPtr<FAssetData>, TSharedRef<SEditableText>);
	DECLARE_DELEGATE_RetVal_TwoParams(FReply, FOnAssetMouseButtonDown, const FGeometry&, const FPointerEvent&);
public:
	SLATE_BEGIN_ARGS(SDataAssetTableRow){}
		SLATE_ARGUMENT(TSharedPtr<FAssetData>, Item)
		SLATE_ARGUMENT(TSharedPtr<SDataAssetManagerWidget>, Owner)

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

	virtual ~SDataAssetTableRow()
	{
		if (OnPackageDirtyStateChangedHandle.IsValid())
		{
			UPackage::PackageDirtyStateChangedEvent.Remove(OnPackageDirtyStateChangedHandle);
		}
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnId) override
	{
		if (ColumnId == DataAssetListColumns::ColumnID_Name)
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
						.OnMouseButtonDown_Lambda([this](const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
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
						)
						.OnMouseDoubleClick_Lambda([this](const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
							{
								if (OnAssetDoubleClicked.IsBound())
								{
								    OnAssetDoubleClicked.Execute(InGeometry, MouseEvent);
									return FReply::Handled();
								}
								return FReply::Unhandled();
                                
							}
						)
					]
				];
		}
		else if (ColumnId == DataAssetListColumns::ColumnID_Type)
		{
			return SNew(STextBlock) // bug fix in 5.5 version GetClass() returned nullptr on some asset classes
				.Text(FText::FromName(Item.IsValid() ? Item->AssetClassPath.GetAssetName() : NAME_None)); 
		}
		else if (ColumnId == DataAssetListColumns::ColumnID_DiskSize)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(DataAssetManager::Private::GetAssetDiskSize(Item.ToSharedRef().Get())));
		}
		else if (ColumnId == DataAssetListColumns::ColumnID_Path)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(Item->PackagePath.ToString()));
		}
		else if (ColumnId == DataAssetListColumns::ColumnID_RC)
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

private:
	bool bIsDirty = false;
    TSharedPtr<FAssetData> Item = nullptr;
	TSharedPtr<SImage> DirtyBrushWidget = nullptr;
	UPackage* AssetPackage = nullptr;
	FOnAssetRenamed OnAssetRenamed{};
    FOnCreateContextMenu OnCreateContextMenu{};
    FOnAssetDoubleClicked OnAssetDoubleClicked{};
    FOnRegisterEditableText OnRegisterEditableText{};
	FOnAssetMouseButtonDown MouseButtonDown{};

	FDelegateHandle OnPackageDirtyStateChangedHandle{};
};

void SDataAssetManagerWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	SubscribeToAssetRegistryEvent();
	LoadDataAssets(DataAssetManager::Private::GetPluginSettings());
	UpdateFilteredAssetList();
	InitializeAssetTypeComboBox(FilteredDataAssets);
	InitializeTextFontInfo();
	CreateDetailsView();

	MenuBar = FDataAssetManagerMenuFactory::CreateMenuBar(SharedThis(this));
	TSharedPtr<SLayeredImage> FilterImage = CreateFilterImage();

	ColumnOrder.Add(DataAssetListColumns::ColumnID_RC);
	ColumnOrder.Add(DataAssetListColumns::ColumnID_Name);
	ColumnOrder.Add(DataAssetListColumns::ColumnID_Type);
	ColumnOrder.Add(DataAssetListColumns::ColumnID_DiskSize);
	ColumnOrder.Add(DataAssetListColumns::ColumnID_Path);

	InitializeColumnAdders();

	ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(5.0f))
			.BorderBackgroundColor(FColor::Transparent)
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MenuBar.ToSharedRef()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSeparator)
						.Orientation(Orient_Vertical)
						.Thickness(1.0f)
						.ColorAndOpacity(FColor::Transparent)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FMargin(2))
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ToolTipText(LOCTEXT("SaveButtonTooltip", "Click to save changes."))
						.OnClicked_Lambda([this]()
							{
								SaveDataAsset();
								return FReply::Handled();
							})
						[
							SNew(SImage)
							.Cursor(EMouseCursor::Hand)
							.Image(FAppStyle::Get().GetBrush("Icons.Save"))
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.Cursor(EMouseCursor::Hand)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						.ContentPadding(FMargin(2))
						.ToolTipText(LOCTEXT("FindAssetToolTip", "Find asset in content browser"))
						.OnClicked_Lambda([this]()
						{
							SyncContentBrowserToSelectedAsset();
							return FReply::Handled();
						})
						[
							SNew(SImage)
							.Cursor(EMouseCursor::Hand)
							.Image(FAppStyle::GetBrush("Icons.Search"))
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SNew(SPositiveActionButton)
						.Icon(FAppStyle::Get().GetBrush("Icons.Plus"))
						.Text(FText::FromString(TEXT("Add")))
						.Cursor(EMouseCursor::Hand)
						.ToolTipText(LOCTEXT("AddDataAssetTooltip", "Click to add a new Data Asset."))
						.OnClicked_Lambda([this]()
						{
							CreateNewDataAsset();
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SNew(SPositiveActionButton)
						.Icon(FAppStyle::Get().GetBrush("MainFrame.SaveAll"))
						.Text(FText::FromString(TEXT("Save All")))
						.Cursor(EMouseCursor::Hand)
						.ToolTipText(LOCTEXT("SaveAllDataAsset", "Save All Data Assets"))
						.OnClicked_Lambda([this]()
						{
							SaveAllData();
							return FReply::Handled();
						})
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSeparator)
					.Orientation(Orient_Vertical)
					.Thickness(0.1f)
					.ColorAndOpacity(FColor::Transparent)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.FillHeight(1.0f)
				[
					SAssignNew(Splitter, SSplitter)
					.Orientation(EOrientation::Orient_Horizontal)
					
					+ SSplitter::Slot()
					.Value_Lambda([&]() { return SplitterValue.Get(); })
					.OnSlotResized_Lambda([&](float NewSize) { SplitterValue.Set(NewSize); })
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(2.0f, 6.0f, 0.0f, 6.0f)
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(4.0f, 0.0f, 4.0f, 0.0f)
							[
								SNew(SMenuAnchor)
								.Placement(EMenuPlacement::MenuPlacement_ComboBoxRight)
								[
									SAssignNew(ListViewSearchBox, SFilterSearchBox)
									.HintText(LOCTEXT("SearchDetailsHint", "Search"))
									.Cursor(EMouseCursor::Hand)
									.OnTextChanged(this, &SDataAssetManagerWidget::OnSearchTextChanged)
									.DelayChangeNotificationsWhileTyping(true)
									.AddMetaData<FTagMetaData>(TEXT("Details.Search"))
									.Visibility_Raw(this, &SDataAssetManagerWidget::GetVisibilitySearchBox)
								]
							]

							+SHorizontalBox::Slot()
							.HAlign(HAlign_Right)
							.AutoWidth()
							.Padding(4.0f, 0.0f, 0.0f, 0.0f)
							[
								SAssignNew(ComboButton, SComboButton)
								.ComboButtonStyle(&FAppStyle::Get().GetWidgetStyle<FComboButtonStyle>("SimpleComboButtonWithIcon"))
								.ForegroundColor(FSlateColor::UseStyle())
								.ContentPadding(FMargin(1, 0))
								.ButtonContent() [ FilterImage.ToSharedRef() ]
								.MenuContent()[ CreateComboButtonContent() ]
							]
						]
						+SVerticalBox::Slot()
						[
							SAssignNew(AssetListView, SListView<TSharedPtr<FAssetData>>)
							.ListItemsSource(&FilteredDataAssets)
							.OnGenerateRow(this, &SDataAssetManagerWidget::GenerateAssetListRow)
							.OnSelectionChanged(this, &SDataAssetManagerWidget::OnAssetSelected)
							.SelectionMode(ESelectionMode::Multi)
							.HeaderRow( GenerateHeaderRow() )
						]

						+ SVerticalBox::Slot()
						.FillHeight(0.6f)
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.AutoWidth()
							[
								SNew(STextBlock)
								.Text_Lambda([this]() { return GetSelectedTextBlockInfo();})
							]
						]
					]

					+SSplitter::Slot()
					.Value(0.6)
					[
						SNew(SBox)
						[
							DetailsView.ToSharedRef()
						]
					]
				]
			]
		];

	if (FilteredDataAssets.Num() > 0)
	{
		AssetListView->SetSelection(FilteredDataAssets[0]);
		OnAssetSelected(FilteredDataAssets[0], ESelectInfo::Direct);
	}
 }

 FText SDataAssetManagerWidget::GetSelectedTextBlockInfo() const
 {
	const FString SelectedStrItems = GetAssetListSelectedItem().Num() > 0
		? FString::Printf(TEXT("(%d selected)"), GetAssetListSelectedItem().Num())
		: TEXT("");

	return FText::FromString(FString::Printf(TEXT("   %d items %s"), FilteredDataAssets.Num(), *SelectedStrItems));
 }

SDataAssetManagerWidget::~SDataAssetManagerWidget()
{
	if (const FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>(DataAssetManager::ModuleName::AssetRegistry))
	{
		const auto SafeRemove = [&](FDelegateHandle& Handle, auto&& Event)
		{
		    if (Handle.IsValid())
		    {
		        Event.Remove(Handle);
		        Handle.Reset();
		    }
		};

		SafeRemove(AssetAddedDelegateHandle, AssetRegistryModule->Get().OnAssetAdded());
		SafeRemove(AssetRemovedDelegateHandle, AssetRegistryModule->Get().OnAssetRemoved());
		SafeRemove(AssetRenamedDelegateHandle, AssetRegistryModule->Get().OnAssetRenamed());
		SafeRemove(FilesLoadedHandle, AssetRegistryModule->Get().OnFilesLoaded());
	}
}
inline void SDataAssetManagerWidget::HandleAssetDoubleClick(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OpenSelectedDataAssetInEditor();
	}
}

inline void SDataAssetManagerWidget::RegisterEditableText(TSharedPtr<FAssetData> AssetData, TSharedRef<SEditableText> EditableText)
{
	EditableTextWidgets.Add({ AssetData->PackagePath,AssetData->AssetName }, EditableText);
}

void SDataAssetManagerWidget::HandleAssetRename(TSharedPtr<FAssetData> AssetData, const FText& InText, ETextCommit::Type CommitMethod)
{
	if (!SelectedAsset.IsValid() || InText.IsEmpty())
	{
		bRenamedProgress = false;
		return;
	}
	UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s EditableTextWidgets counts %d"), *FString(__FUNCTION__), EditableTextWidgets.Num());

	if (CommitMethod == ETextCommit::OnEnter)
	{
		UObject* Asset = SelectedAsset->GetAsset();
		if (!IsValid(Asset))
		{
			UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("Asset is not valid"));
			return;
		}
		FString NewName = InText.ToString();
		FString PackagePath = Asset->GetPathName();
		PackagePath = FPaths::GetPath(PackagePath);
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>(DataAssetManager::ModuleName::AssetTools).Get();
		if (AssetTools.RenameAssets({ FAssetRenameData(Asset, PackagePath, NewName) }))
		{
			bRenamedProgress = false;
			UE_LOG(SDataAssetManagerWidgetLog, Log, TEXT("Asset renamed %s"), *Asset->GetName());
		}
	}
}

inline ESelectionMode::Type SDataAssetManagerWidget::GetAssetListSelectionMode() const
{
	return bRenamedProgress ? ESelectionMode::Single : ESelectionMode::Multi;
}

inline EVisibility SDataAssetManagerWidget::GetVisibilitySearchBox() const
{
	return SplitterValue.Get() < DataAssetManager::SearchBoxHideThreshold ? EVisibility::Hidden : EVisibility::Visible;
}

void SDataAssetManagerWidget::SubscribeToAssetRegistryEvent()
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(DataAssetManager::ModuleName::AssetRegistry);

	const auto SubscribeDelegates = [this, &AssetRegistryModule]()
	{
		AssetAddedDelegateHandle	= AssetRegistryModule.Get().OnAssetAdded().AddRaw(this, &SDataAssetManagerWidget::OnAssetAdded);
		AssetRemovedDelegateHandle	= AssetRegistryModule.Get().OnAssetRemoved().AddRaw(this, &SDataAssetManagerWidget::OnAssetRemoved);
		AssetRenamedDelegateHandle	= AssetRegistryModule.Get().OnAssetRenamed().AddRaw(this, &SDataAssetManagerWidget::OnAssetRenamed);
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

void SDataAssetManagerWidget::InitializeTextFontInfo()
{
	TextFontInfo = FCoreStyle::Get().GetFontStyle(FName("NormalText"));
	TextFontInfo.Size = DataAssetManager::DataAssetFontSize;
}

void SDataAssetManagerWidget::CreateDetailsView()
{
	const FDetailsViewArgs DetailsViewArgs = CreateDetailsViewArgs();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(DataAssetManager::ModuleName::PropertyEditor);
	DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);
}

FDetailsViewArgs SDataAssetManagerWidget::CreateDetailsViewArgs() const
{
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bShowObjectLabel = false;
	Args.bCustomNameAreaLocation = false;

	return Args;
}

TSharedPtr<SLayeredImage> SDataAssetManagerWidget::CreateFilterImage()
{
	return SNew(SLayeredImage)
		.Image(FAppStyle::Get().GetBrush("Icons.Filter"))
		.ColorAndOpacity(FSlateColor(FColor::White))
		.Visibility_Lambda([this]() { return SplitterValue.Get() < 0.05f ? EVisibility::Hidden : EVisibility::Visible; });
}


void SDataAssetManagerWidget::CreateContextMenuFromDataAsset(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FMenuBuilder MenuBuilder(true, nullptr);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("RenameMenuEntry", "Rename         "),
			LOCTEXT("RenameMenuTooltip", "Rename this item"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Rename"),
			FUIAction(FExecuteAction::CreateSP(this, &SDataAssetManagerWidget::FocusOnSelectedAsset),
				FCanExecuteAction::CreateLambda([this]() { return CanRename(); })));

		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteMenuEntry", "Delete         "),
			LOCTEXT("DeleteMenuTooltip", "Delete this item"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Delete"),
			FUIAction(FExecuteAction::CreateSP(this, &SDataAssetManagerWidget::DeleteDataAsset)));

		FSlateApplication::Get().PushMenu(
			AsShared(),
			FWidgetPath(),
			MenuBuilder.MakeWidget(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
	}
}

TSharedRef<SWidget> SDataAssetManagerWidget::CreateComboButtonContent()
{
    TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSpacer)
			.Size(FVector2D(2.0f, 2.0f))
		]

		+ SVerticalBox::Slot()
		[
			SNew(SListView<TSharedPtr<FString>>)
			.SelectionMode(ESelectionMode::None)
			.ListItemsSource(&ComboBoxAssetListItems)
			.OnGenerateRow_Lambda([this](TSharedPtr<FString> SourceItem, const TSharedRef<STableViewBase>& OwnerTable)
			{
				return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
					.Padding(7.f)
					[
						SNew(SOverlay)
						+SOverlay::Slot()
						[
							SNew(SCheckBox)
							.BorderBackgroundColor(FSlateColor::UseForeground())
							.ForegroundColor(FSlateColor::UseForeground())
							.IsChecked_Lambda([&, SourceItem]()
							{
								return ActiveFilters.Contains(*SourceItem) ? 
									ECheckBoxState::Checked : 
									ECheckBoxState::Unchecked;
							})
							.OnCheckStateChanged_Lambda([this, SourceItem](ECheckBoxState NewState)
							{
								(NewState == ECheckBoxState::Checked ? 
									static_cast<void>(ActiveFilters.Add(*SourceItem)) : 
									static_cast<void>(ActiveFilters.Remove(*SourceItem)));
								UpdateFilteredAssetList();
							})
							[
								SNew(STextBlock)
								.ColorAndOpacity(FSlateColor::UseForeground())
								.Text(FText::FromString(*SourceItem))
							]
						]
					];
			})
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSpacer)
			.Size(FVector2D(2.0f, 2.0f))
		];


	return SNew(SBox)
		.MaxDesiredHeight(400.f)
		.WidthOverride(250.f)
		[
			VerticalBox
		];
}

FReply SDataAssetManagerWidget::OnItemClicked(TSharedPtr<FString> SourceItem)
{
	SelectedAssetType = SourceItem;
	UpdateFilteredAssetList();
	if (ComboButton.IsValid())
	{
		ComboButton->SetIsOpen(false);
	}

	return FReply::Handled();
}

void SDataAssetManagerWidget::LoadDataAssets(const UDataAssetManagerSettings* PluginSettings)
{	
	if (!PluginSettings) return;

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(DataAssetManager::ModuleName::AssetRegistry);
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FString> AssetDirectories;
	AssetDirectories.Reserve(PluginSettings->ScannedAssetDirectories.Num());

	for (const FDirectoryPath& Dir : PluginSettings->ScannedAssetDirectories)
	{
		FString NormalizedPath = Dir.Path;
		FPaths::NormalizeDirectoryName(NormalizedPath);
		AssetDirectories.Add(MoveTemp(NormalizedPath));
	}

	TArray<FTopLevelAssetPath> IgnoredClassPaths;
	IgnoredClassPaths.Reserve(PluginSettings->ExcludedScanAssetTypes.Num());
	for (const TSubclassOf<UDataAsset>& IgnoredClass : PluginSettings->ExcludedScanAssetTypes)
	{
		if (IgnoredClass)
		{
			IgnoredClassPaths.Add(IgnoredClass->GetClassPathName());
		}
	}

	TArray<FAssetData> AssetDataArray;
	const FTopLevelAssetPath DataAssetPath = UDataAsset::StaticClass()->GetClassPathName();
	if (!AssetRegistry.GetAssetsByClass(DataAssetPath, AssetDataArray, true))
	{
		UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s Failed to get assets by class"), *FString(__FUNCTION__));
		return;
	}

	DataAssets.Reset(AssetDataArray.Num());
	for (const FAssetData& AssetData : AssetDataArray)
	{
		if (IgnoredClassPaths.Contains(AssetData.AssetClassPath))
		{
			continue;
		}

		FString NormalizedAssetPath = AssetData.PackagePath.ToString();
		FPaths::NormalizeDirectoryName(NormalizedAssetPath);

		// Check if asset is in any of our directories
		if (Algo::AnyOf(AssetDirectories, [&NormalizedAssetPath](const FString& Directory)
		{
		    return NormalizedAssetPath.StartsWith(Directory);
		}))
		{
		    DataAssets.Add(MakeShared<FAssetData>(AssetData));
		}
	}

	/**
	 * Sorts the found DataAssets alphabetically by asset name.
	 *
	 * Uses lexicographical comparison (LexicalLess) which:
	 * - Is case-sensitive
	 * - More efficient than string comparison as it works directly with FName
	 */
	DataAssets.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B) 
		{
			return A->AssetName.LexicalLess(B->AssetName);
		});
}

void SDataAssetManagerWidget::UpdateFilteredAssetList()
{
	FilteredDataAssets.Empty();
	const FString SearchString = SearchText.Get().ToString();
	for (const TSharedPtr<FAssetData>& AssetData : DataAssets)
	{
		if (!AssetData.IsValid())
		{
			continue;
		}

		const FString AssetClassName = AssetData->AssetClassPath.GetAssetName().ToString();
		const bool bMatchesType = ActiveFilters.Num() == 0 || ActiveFilters.Contains(AssetClassName);
		const bool bNameMatches = SearchString.IsEmpty() || AssetData->AssetName.ToString().Contains(SearchString);

		if (bMatchesType && bNameMatches)
		{
			FilteredDataAssets.Add(AssetData);
		}
	}

	if (AssetListView.IsValid())
	{
		AssetListView->RequestListRefresh();
	}
}

void SDataAssetManagerWidget::OnSearchTextChanged(const FText& InText)
{
	SearchText.Set(InText);
	UpdateFilteredAssetList();
}

TSharedRef<ITableRow> SDataAssetManagerWidget::GenerateAssetListRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerSTable)
{	
	return SNew(SDataAssetTableRow, OwnerSTable)
		.Item(Item)
		.OnAssetRenamed(this, &SDataAssetManagerWidget::HandleAssetRename)
		.OnCreateContextMenu(this, &SDataAssetManagerWidget::CreateContextMenuFromDataAsset)
		.OnAssetDoubleClicked(this, &SDataAssetManagerWidget::HandleAssetDoubleClick)
		.OnRegisterEditableText(this, &SDataAssetManagerWidget::RegisterEditableText)
		.OnMouseButtonDown(this, &SDataAssetManagerWidget::HandleRowMouseButtonDown);
}

FReply SDataAssetManagerWidget::HandleRowMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
    // ƒл€ левой кнопки разрешаем стандартную обработку
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }
    return FReply::Handled(); 
}

void SDataAssetManagerWidget::InitializeAssetTypeComboBox(TArray<TSharedPtr<FAssetData>> AssetDataList)
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
	// Debug !!!
	UE_LOG(LogTemp, Warning, TEXT("%s FilteredDataAssets: %i"), *FString(__FUNCTION__), FilteredDataAssets.Num());
}

void SDataAssetManagerWidget::FocusOnNewlyAddedAsset(const FAssetData& NewAssetData)
{
	TSharedPtr<FAssetData> NewAssetPtr = nullptr;
	for (const TSharedPtr<FAssetData>& Asset : FilteredDataAssets)
	{
		if (Asset->PackageName == NewAssetData.PackageName)
		{
			NewAssetPtr = Asset;
			break;
		}
	}

	if (!NewAssetPtr.IsValid())
	{
		UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s Newly added asset '%s' not found in filtered list"), 
			*FString(__FUNCTION__), *NewAssetData.PackageName.ToString());
		return;
	}

	if (UObject* AssetObject = NewAssetPtr->GetAsset())
	{
		if (AssetObject->HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad))
		{
			UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s: Asset '%s' is not fully loaded (flags: %X)"), 
				*FString(__FUNCTION__), *AssetObject->GetName(), AssetObject->GetFlags());
			return;
		}
	}
	else
	{
		UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s: Failed to load asset '%s'"), 
			*FString(__FUNCTION__), *NewAssetData.PackageName.ToString());
		return;
	}

	if (AssetListView.IsValid())
	{
		AssetListView->SetSelection(NewAssetPtr);
		OnAssetSelected(NewAssetPtr, ESelectInfo::Direct);
		AssetListView->RequestScrollIntoView(NewAssetPtr);
	}
}

FReply SDataAssetManagerWidget::ColumnButtonClicked(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FMenuBuilder MenuBuilder(true, nullptr);

		MenuBuilder.BeginSection("AdditionalActions", LOCTEXT("AdditionalActionsSection", "Additional Actions"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("ToggleAllColumns", "Hide All Columns"),
				LOCTEXT("ToggleAllColumnsTooltip", "Hide or show all columns at once"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([this]()
					{
						const bool bShouldHide = bShowDiskSizeColumn || bShowPathColumn 
							|| bShowTypeColumn || bShowRevisionColumn;

						bShowDiskSizeColumn = !bShouldHide;
						bShowPathColumn = !bShouldHide;
						bShowTypeColumn = !bShouldHide;
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


void SDataAssetManagerWidget::AddColumnToHeader(TSharedPtr<SHeaderRow> InHeaderRow, const FName& ColumnId, const FString& Label, const float FillWidth)
{
	InHeaderRow->AddColumn(SHeaderRow::FColumn::FArguments()
		.ColumnId(ColumnId)
		.DefaultLabel(FText::FromString(Label))
		.FillWidth(FillWidth)
		.HeaderContent()
		[
			SNew(SBorder)
			.BorderBackgroundColor(FSlateColor(FColor::Transparent))
			.OnMouseButtonDown(this, &SDataAssetManagerWidget::ColumnButtonClicked)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
			]
		]);
}


inline const FSlateBrush* SDataAssetManagerWidget::GetRevisionControlColumnIconBadge() const
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

void SDataAssetManagerWidget::InitializeColumnAdders() 
{
	ColumnAdders.Add(DataAssetListColumns::ColumnID_RC, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		if (bShowRevisionColumn)
		{
			HeaderRow->AddColumn(CreateRevisionControlColumn());
		}
	});

	ColumnAdders.Add(DataAssetListColumns::ColumnID_Name, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		AddColumnToHeader(HeaderRow, DataAssetListColumns::ColumnID_Name, TEXT("Name"), 0.4f);
	});

	ColumnAdders.Add(DataAssetListColumns::ColumnID_Type, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		if (bShowTypeColumn)
		{
			AddColumnToHeader(HeaderRow, DataAssetListColumns::ColumnID_Type, TEXT("Type"), 0.3f);
		}
	});

	ColumnAdders.Add(DataAssetListColumns::ColumnID_DiskSize, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		if (bShowDiskSizeColumn)
		{
			AddColumnToHeader(HeaderRow, DataAssetListColumns::ColumnID_DiskSize, TEXT("DiskSize"), 0.15f);
		}
	});

	ColumnAdders.Add(DataAssetListColumns::ColumnID_Path, [this](TSharedPtr<SHeaderRow> HeaderRow)
	{
		if (bShowPathColumn)
		{
			AddColumnToHeader(HeaderRow, DataAssetListColumns::ColumnID_Path, TEXT("Path"), 0.3f);
		}
	});
}


SHeaderRow::FColumn::FArguments SDataAssetManagerWidget::CreateRevisionControlColumn()
{
	TSharedRef<SLayeredImage> RevisionControlColumnIcon = SNew(SLayeredImage)
	.ColorAndOpacity(FSlateColor::UseForeground())
	.Image(FRevisionControlStyleManager::Get().GetBrush("RevisionControl.Icon"));

    RevisionControlColumnIcon->AddLayer(TAttribute<const FSlateBrush*>::CreateSP(this,
        &SDataAssetManagerWidget::GetRevisionControlColumnIconBadge));

    return SHeaderRow::Column(DataAssetListColumns::ColumnID_RC)
        .FixedWidth(StaticCast<TOptional<float>>(30.0f))
        .HAlignHeader(HAlign_Center)
        .VAlignHeader(VAlign_Center)
        .HAlignCell(HAlign_Center)
        .VAlignCell(VAlign_Center)
        .DefaultLabel(LOCTEXT("Column_RC", "Revision Control"))
        [
            RevisionControlColumnIcon
        ];
}


TSharedRef<SHeaderRow> SDataAssetManagerWidget::GenerateHeaderRow()
{
	TSharedRef<SHeaderRow> HeaderRow = SNew(SHeaderRow).Cursor(EMouseCursor::Hand);
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

void SDataAssetManagerWidget::UpdateColumnVisibility()
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

void SDataAssetManagerWidget::FocusOnSelectedAsset()
{
	if (!IsSelectedAssetValid()) return;

	UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s EditableTextWidgets counts %d"), *FString(__FUNCTION__), EditableTextWidgets.Num());

	TSharedPtr<FAssetData> FoundAsset = nullptr;
	for (const TSharedPtr<FAssetData>& DataAsset : FilteredDataAssets)
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
		UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("Widget not found in EditableTextMap"));
	}
}

void SDataAssetManagerWidget::OnAssetSelected(TSharedPtr<FAssetData> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		SelectedAsset = SelectedItem;
		if (!SelectedItem.IsValid())
		{
			return;
		}

		OpenDetailViewPanelForAsset(SelectedItem);
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

void SDataAssetManagerWidget::OpenDetailViewPanelForAsset(TSharedPtr<FAssetData> SelectedItem)
{
	if (!SelectedItem.IsValid())
	{
		UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("Selected Item is not valid"));
		return;
	}

	UDataAsset* const DataAsset = Cast<UDataAsset>(SelectedItem->GetAsset());
	if (!DataAsset)
	{
		UE_LOG(SDataAssetManagerWidgetLog, Error, TEXT("Failed to cast SelectedItem to UDataAsset. The asset might be of a different type or invalid."));
		return;
	}

	DetailsView->SetObject(DataAsset);
}

void SDataAssetManagerWidget::ProcessAssetData(const TArray<FAssetData>& RefAssetData, TFunction<void(const TArray<FAssetIdentifier>&)> ProcessFunction)
{
	TArray<FAssetIdentifier> AssetIdentifiers;
	IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList(RefAssetData, AssetIdentifiers);
	ProcessFunction(AssetIdentifiers);
}

void SDataAssetManagerWidget::OnAssetAdded(const FAssetData& NewAssetData)
{
	LoadDataAssets(DataAssetManager::Private::GetPluginSettings());
	UpdateFilteredAssetList();
	InitializeAssetTypeComboBox(DataAssets);

	FocusOnNewlyAddedAsset(NewAssetData);

	UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s Call Delegate"), *FString(__FUNCTION__));
}

void SDataAssetManagerWidget::OnAssetRemoved(const FAssetData& AssetToRemoved)
{
	LoadDataAssets(DataAssetManager::Private::GetPluginSettings());
	UpdateFilteredAssetList();
	InitializeAssetTypeComboBox(DataAssets);
	UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s Call Delegate"), *FString(__FUNCTION__));
}

void SDataAssetManagerWidget::OnAssetRenamed(const FAssetData& NewAssetData, const FString& Name)
{
	LoadDataAssets(DataAssetManager::Private::GetPluginSettings());
	UpdateFilteredAssetList();
	InitializeAssetTypeComboBox(DataAssets);

	FocusOnNewlyAddedAsset(NewAssetData);
	UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s Call Delegate"), *FString(__FUNCTION__));
}

void SDataAssetManagerWidget::DeleteDataAsset()
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
			LockedAssetsList += FString::Printf(TEXT("\nХ %s"), *Asset.AssetName.ToString());
	
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
		DataAssetManager::Private::DeleteMultiplyAsset(AssetsToDelete);
	}
}

// original code copy-paste from FAssetFileContextMenu class 
// AssetFileContextMenu::ExecuteShowAssetMetaData() 
// (*mini refactoring for original code *add const corection)
// 
void SDataAssetManagerWidget::ShowAssetMetaData()
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
					.MinWidth(StaticCast<TOptional<float>>(500.0f))
					.MinHeight(StaticCast<TOptional<float>>(250.0f))
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

bool SDataAssetManagerWidget::SaveAllDataAsset()
{
	constexpr bool bPromptUserToSave = false;
	constexpr bool bSaveMapPackages = true;
	constexpr bool bSaveContentPackages = true;
	constexpr bool bFastSave = false;
	constexpr bool bNotifyNoPackagesSaved = false;
	constexpr bool bCanBeDeclined = false;
	return FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave, bNotifyNoPackagesSaved, bCanBeDeclined);
}

void SDataAssetManagerWidget::UpdateComboButtonContent()
{
	if (ComboButton.IsValid())
	{
		ComboButton->SetMenuContent(CreateComboButtonContent());
	}
}

TArray<TSharedPtr<FAssetData>> SDataAssetManagerWidget::GetAssetListSelectedItem() const
{
	TArray<TSharedPtr<FAssetData>> SelectedItems;
	AssetListView->GetSelectedItems(SelectedItems);

	return SelectedItems;
}

bool SDataAssetManagerWidget::IsSelectedAssetValid(const FString& CustomMessage) const
{
	if (SelectedAsset.IsValid()) return true;

	const FString ErrorMsg = CustomMessage.IsEmpty() 
		? FString::Printf(TEXT("%s Selected Asset is not valid"), *FString(__FUNCTION__))
		: CustomMessage;

	UE_LOG(SDataAssetManagerWidgetLog, Warning, TEXT("%s"), *ErrorMsg);
	return false;
}


#undef LOCTEXT_NAMESPACE
