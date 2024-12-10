// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintScanner.h"
#include "BlueprintScannerStyle.h"
#include "BlueprintScannerCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Framework/Commands/Commands.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "SlateBasics.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet/KismetStringLibrary.h"
#include "PackageTools.h"
#include "Engine/Blueprint.h"
#include "Engine/LevelScriptBlueprint.h"
#include "FileHelpers.h"
#include "Subsystems/AssetEditorSubsystem.h"

#include "Dialog/SCustomDialog.h"
#include "Widgets/Input/SHyperlink.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "BlueprintScannerSettings.h"

static const FName BlueprintScannerTabName("BlueprintScanner");

#define LOCTEXT_NAMESPACE "FBlueprintScannerModule"

DEFINE_LOG_CATEGORY(BlueprintScannerLog);

// Formats text, accounts for proper grammar with plurals
#define BLUEPRINTS_TEXT(x) FText::Format(FText::FromString("{0} blueprint{1}"), x, (x == 1) ? FText::GetEmpty() : FText::FromString("s"))


namespace ScanerBlueprints
{
	constexpr float ExpireDuration = 15.0f;
	constexpr float InfoNotificationExpireDuration = 5.0f;
}

/**
 * Opens the editor for the specified Blueprint.
 *
 * This function attempts to open the Blueprint editor for a given `UBlueprint` object.
 * It first checks if the Blueprint is valid and if the `GEditor` instance is available.
 * If both conditions are met, it opens the editor using the `UAssetEditorSubsystem`.
 *
 * @param InBlueprint A weak pointer to the `UBlueprint` object to open in the editor.
 */
static void OpenBlueprintEditor(TWeakObjectPtr<UBlueprint> InBlueprint)
{
	if (UBlueprint* BlueprintToEdit = InBlueprint.Get())
	{
		if (! GEditor) return;

		if (UAssetEditorSubsystem* EditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			EditorSubsystem->OpenEditorForAsset(BlueprintToEdit);
		}
	}
}

/**
 * Creates a hyperlink widget that, when clicked, opens the specified Blueprint editor.
 *
 * This function creates a clickable hyperlink UI element using `SHyperlink`. When the user clicks the hyperlink,
 * it opens the editor for the specified `UBlueprint` and brings the dialog containing the hyperlink to the front
 * if the dialog is valid.
 *
 * @param InBlueprint A weak pointer to the `UBlueprint` to be opened in the editor.
 * @param InDialog A shared pointer to the dialog window containing the hyperlink, used to bring the dialog to the front.
 * @return A shared reference to the created `SHyperlink` widget.
 */
static TSharedRef<SWidget> CreateBlueprintHyperlink(TWeakObjectPtr<UBlueprint> InBlueprint, TSharedPtr<SCustomDialog> InDialog)
{
	return SNew(SHyperlink)
		.Style(FAppStyle::Get(), "Common.GotoBlueprintHyperlink")
		.OnNavigate(FSimpleDelegate::CreateLambda(
			[InBlueprint, InDialog]()
			{
				OpenBlueprintEditor(InBlueprint);

				if (InDialog.IsValid())
				{
					InDialog->BringToFront(true);
				}
			}))
		.Text(FText::FromString(InBlueprint->GetName()))
		.ToolTipText(NSLOCTEXT("SourceHyperlink", "EditBlueprint_ToolTip", "Click to edit the blueprint"));
}

/**
 * Displays a dialog showing a list of blueprints that failed to compile, with clickable hyperlinks
 * that open the corresponding blueprint editors once the dialog is closed.
 *
 * This function creates and shows a modal dialog containing a list of errored `UBlueprint` objects.
 * Each blueprint is represented by a clickable hyperlink, which opens the blueprint editor when clicked.
 * The dialog provides a "Dismiss" button to close the window.
 *
 * @param ErroredBlueprints An array of `UBlueprint` objects that failed to compile.
 */
static void ShowProblemBlueprintsDialog(TArray<UBlueprint*> ErroredBlueprints)
{
	TSharedRef<SVerticalBox> DialogContents = SNew(SVerticalBox);

	DialogContents->AddSlot()
					.Padding(0, 0, 0, 16)
					[
						SNew(STextBlock)
							.Text(FText::FromString("The following blueprints failed to compile:"))
					];

	TSharedPtr<SCustomDialog> CustomDialog;

	for (UBlueprint* Blueprint : ErroredBlueprints)
	{
		DialogContents->AddSlot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						[
							CreateBlueprintHyperlink(Blueprint, CustomDialog)
						];
	}

	DialogContents->AddSlot()
					.Padding(0, 16, 0, 0)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Clicked blueprints will open once this dialog is closed."))
					];

	CustomDialog = SNew(SCustomDialog)
						.Title(FText::FromString("Blueprint Compilation Errors"))
						.Icon(FAppStyle::Get().GetBrush("NotificationList.DefaultMessage"))
						.Content()[DialogContents]
						.Buttons({SCustomDialog::FButton(FText::FromString("Dismiss"))});

	CustomDialog->ShowModal();
}




void FBlueprintScannerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FBlueprintScannerStyle::Initialize();
	FBlueprintScannerStyle::ReloadTextures();

	FBlueprintScannerCommands::Register();
	
	RegisterLevelEditorButton();
	RegisterPathViewContextMenuButton();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FBlueprintScannerCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FBlueprintScannerModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBlueprintScannerModule::RegisterMenus));
}

void FBlueprintScannerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FBlueprintScannerStyle::Shutdown();

	FBlueprintScannerCommands::Unregister();

	LevelEditorExtender->RemoveExtension(LevelEditorExtension.ToSharedRef());
	LevelEditorExtension.Reset();
	ContentBrowserExtension.Reset();
	LevelEditorExtender.Reset();
}

void FBlueprintScannerModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FBlueprintScannerModule::PluginButtonClicked()")),
							FText::FromString(TEXT("BlueprintScanner.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FBlueprintScannerModule::RegisterLevelEditorButton()
{
	const TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList());

	CommandList->MapAction(FBlueprintScannerCommands::Get().ScanAndRefreshButton, FExecuteAction::CreateRaw(this, &FBlueprintScannerModule::RefreshAllButton_Clicked), FCanExecuteAction());

	LevelEditorExtender = MakeShareable(new FExtender());
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	LevelEditorExtension = LevelEditorExtender->AddMenuExtension("WorldSettingsClasses", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FBlueprintScannerModule::AddLevelEditorMenuEntry));

	auto& MenuExtenders = LevelEditorModule.GetAllLevelEditorToolbarBlueprintsMenuExtenders();
	MenuExtenders.Add(LevelEditorExtender);

	LevelEditorModule.GetGlobalLevelEditorActions()->Append(CommandList.ToSharedRef());
}

void FBlueprintScannerModule::RegisterPathViewContextMenuButton()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	ContentBrowserModule.GetAllPathViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FBlueprintScannerModule::CreateContentBrowserExtender));
}

TSharedRef<FExtender> FBlueprintScannerModule::CreateContentBrowserExtender(const TArray<FString>& SelectedPaths)
{
	SelectedFolders = SelectedPaths;

	TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList());
	CommandList->MapAction(FBlueprintScannerCommands::Get().ScanAndRefreshPathButton, FExecuteAction::CreateRaw(this, &FBlueprintScannerModule::RefreshPathButton_Clicked), FCanExecuteAction());

	TSharedPtr<FExtender> ContentBrowserExtender = MakeShareable(new FExtender());

	ContentBrowserExtension = ContentBrowserExtender->AddMenuExtension("PathViewFolderOptions", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateRaw(this, &FBlueprintScannerModule::AddPathViewContextMenuEntry));
	return ContentBrowserExtender.ToSharedRef();
}

void FBlueprintScannerModule::FindAndRefreshBlueprints(const FARFilter& Filter, bool bShouldExclude)
{
	ResetBlueprintsState();
	const UBlueprintScannerSettings* Settings = GetDefault<UBlueprintScannerSettings>();

	int32 NumAssets;
	TArray<FAssetData> AssetDatas = FindAssets(Filter, NumAssets);

	if (AssetDatas.IsEmpty())
	{
		ShowNotification(FText::FromString("No blueprints were refreshed"), 1.5f);
		return;
	}

	TSharedPtr<SNotificationItem> RefreshingNotification = ShowProgressNotification(AssetDatas.Num());

	TArray<UPackage*> PackagesToSave = ProcessAssets(AssetDatas, Settings, bShouldExclude);

	DisplayDebugMessage(Settings);
	HandleSavePackages(PackagesToSave);
	UpdateProgressNotification(RefreshingNotification, NumAssets);
	DisplayCompilationNotification();
}

void FBlueprintScannerModule::AddLevelEditorMenuEntry(FMenuBuilder& Builder)
{
	FSlateIcon IconBrush = FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorViewport.RotateMode");

	Builder.BeginSection("Scanning Blueprints ", FText::FromString("Refresh All Nodes"));
	Builder.AddMenuEntry(FBlueprintScannerCommands::Get().ScanAndRefreshButton, FName(""), FText::FromString("Refresh All Blueprint Nodes"), FText::FromString("Refresh all nodes in every blueprint"), IconBrush);
	Builder.EndSection();
}

void FBlueprintScannerModule::AddPathViewContextMenuEntry(FMenuBuilder& Builder)
{
	FSlateIcon IconBrush = FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorViewport.RotateMode");

	Builder.AddMenuEntry(FBlueprintScannerCommands::Get().ScanAndRefreshPathButton, FName(""), FText::FromString("Scanning Blueprints"), FText::FromString("Refresh all nodes in blueprints under this folder"), IconBrush);
}

void FBlueprintScannerModule::RefreshPathButton_Clicked()
{
	// This function is called when the button in the Content Browser right-click context menu is pressed
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	const UBlueprintScannerSettings* Settings = GetDefault<UBlueprintScannerSettings>();

	if (Settings->bRefreshLevelBlueprints)
	{
		// Search for UWorld objects if we're searching for level blueprints
		Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	}

	for (const FString& FolderPath : SelectedFolders)
	{
		Filter.PackagePaths.Add(*FolderPath);
	}

	FindAndRefreshBlueprints(Filter, false);
}

void FBlueprintScannerModule::RefreshAllButton_Clicked()
{
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	const UBlueprintScannerSettings* Settings = GetDefault<UBlueprintScannerSettings>();

	for (const FName Path : Settings->AdditionalBlueprintPaths)
	{
		Filter.PackagePaths.Add(UKismetStringLibrary::Conv_StringToName("/" + Path.ToString()));
	}

	if (Settings->bRefreshLevelBlueprints)
	{
		// Search for UWorld objects if we're searching for level blueprints
		Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	}
	if (Settings->bRefreshGameBlueprints)
	{
		Filter.PackagePaths.Add("/Game");
	}
	if (Settings->bRefreshEngineBlueprints)
	{
		Filter.PackagePaths.Add("/Engine");
	}

	FindAndRefreshBlueprints(Filter);
}


void FBlueprintScannerModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FBlueprintScannerCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FBlueprintScannerCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}


TArray<UPackage*> FBlueprintScannerModule::ProcessAssets(const TArray<FAssetData>& AssetDatas, const UBlueprintScannerSettings* Settings, bool bShouldExclude)
{
	TArray<UPackage*> PackagesToSave;

	for (const FAssetData& Data : AssetDatas)
	{
		if (ShouldSkipAsset(Data, Settings, bShouldExclude)) continue;

		TWeakObjectPtr<UBlueprint> Blueprint = GetBlueprintFromAsset(Data);

		if (! Blueprint.IsValid()) continue;

		FString AssetPathString = Data.GetObjectPathString();
		UE_LOG(BlueprintScannerLog, Warning, TEXT("Refreshing Blueprint: %s"), *AssetPathString);

		FBlueprintEditorUtils::RefreshAllNodes(Blueprint.Get());

		if (Settings->bCompileBlueprints)
		{
			CompileBlueprint(Blueprint, AssetPathString);
		}

		PackagesToSave.Add(Data.GetPackage());
	}

	return PackagesToSave;
}

bool FBlueprintScannerModule::ShouldSkipAsset(const FAssetData& Data, const UBlueprintScannerSettings* Settings, bool bShouldExclude)
{
	if (! bShouldExclude) return false;

	FString AssetPathString = Data.GetObjectPathString();
	for (const FName& Path : Settings->ExcludeBlueprintPaths)
	{
		if (AssetPathString.StartsWith(Path.ToString(), ESearchCase::CaseSensitive)) return true;
	}

	return false;
}

void FBlueprintScannerModule::DisplayCompilationNotification()
{
	NotificationInfoCompilationMessage(ErrorBlueprints, "{0} failed to compile", "Icons.WarningWithColor");
	NotificationInfoCompilationMessage(WarningBlueprints, "{0} warning", "Icons.Warning");
}

TArray<FAssetData> FBlueprintScannerModule::FindAssets(const FARFilter& Filter, int32& NumAssets)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	NumAssets = AssetData.Num();

	return AssetData;
}

TWeakObjectPtr<UBlueprint> FBlueprintScannerModule::GetBlueprintFromAsset(const FAssetData& Data)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Data.FastGetAsset())) return Blueprint;

	if (UWorld* World = Cast<UWorld>(Data.GetAsset()))
	{
		if (ULevel* Level = World->GetCurrentLevel()) return Level->GetLevelScriptBlueprint(true);
	}

	return nullptr;
}

void FBlueprintScannerModule::UpdateProgressNotification(TSharedPtr<SNotificationItem> RefreshingNotification, int32 NumAssets)
{
	if (RefreshingNotification.IsValid())
	{
		RefreshingNotification->SetText(FText::Format(FText::FromString("Refreshed {0}"), BLUEPRINTS_TEXT(NumAssets)));
		RefreshingNotification->SetCompletionState(SNotificationItem::CS_Success);
		RefreshingNotification->ExpireAndFadeout();
	}
}

void FBlueprintScannerModule::PrintDebugMessageForAssetData(const TArray<FAssetData>& AssetDatas)
{
	for (const auto AssetData : AssetDatas)
	{
		UE_LOG(BlueprintScannerLog, Warning, TEXT("%s"), *AssetData.GetObjectPathString());
		FString Msg = FString::Printf(TEXT("Blueprint Refresh : %s"), *AssetData.GetObjectPathString());
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, *Msg);
	}
}

void FBlueprintScannerModule::NotificationInfoCompilationMessage(TArray<UBlueprint*>& Blueprints, const FString& Mesasge, const FName& PropertyName)
{
	if (! Blueprints.IsEmpty())
	{
		FNotificationInfo Info(FText::Format(FText::FromString(Mesasge), BLUEPRINTS_TEXT(Blueprints.Num())));
		Info.ExpireDuration = ScanerBlueprints::ExpireDuration;
		Info.Image = FAppStyle::GetBrush(PropertyName);

		TSharedPtr<SNotificationItem> ProblemNotification;
		ProblemNotification = FSlateNotificationManager::Get().AddNotification(Info);
		ProblemNotification->SetHyperlink(FSimpleDelegate::CreateLambda([&]() { ShowProblemBlueprintsDialog(Blueprints); }), FText::FromString("Show blueprints"));
	}
}

void FBlueprintScannerModule::HandleSavePackages(const TArray<UPackage*>& PackagesToSave)
{
	bool bSuccess = UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);

	if (! bSuccess)
	{
		UE_LOG(BlueprintScannerLog, Error, TEXT("Failed to save packages"));
		FNotificationInfo Info(FText::FromString("Failed to save packages"));
		Info.ExpireDuration = ScanerBlueprints::ExpireDuration;

		FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Fail);
	}
}

void FBlueprintScannerModule::ResetBlueprintsState()
{
	ErrorBlueprints.Reset();
	WarningBlueprints.Reset();
}

TSharedPtr<SNotificationItem> FBlueprintScannerModule::ShowProgressNotification(int32 NumAssets)
{
	FNotificationInfo Info(FText::Format(FText::FromString("Refreshing {0}..."), BLUEPRINTS_TEXT(NumAssets)));
	Info.ExpireDuration = ScanerBlueprints::InfoNotificationExpireDuration;
	Info.bFireAndForget = false;

	return FSlateNotificationManager::Get().AddNotification(Info);
}

void FBlueprintScannerModule::ShowNotification(const FText& Message, float Duration)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = Duration;
	FSlateNotificationManager::Get().AddNotification(Info);
}

void FBlueprintScannerModule::CompileBlueprint(TWeakObjectPtr<UBlueprint>& Blueprint, const FString& AssetPathString)
{
	// Compile blueprint
	UE_LOG(BlueprintScannerLog, Warning, TEXT("Compiling Blueprint: %s"), *AssetPathString);
	FKismetEditorUtilities::CompileBlueprint(Blueprint.Get(), EBlueprintCompileOptions::BatchCompile | EBlueprintCompileOptions::SkipSave);

	// Check if the blueprint failed to compile
	if (! Blueprint->IsUpToDate() && Blueprint->Status != BS_Unknown)
	{
		UE_LOG(BlueprintScannerLog, Error, TEXT("Failed to compile %s"), *AssetPathString);
		ErrorBlueprints.Add(Blueprint.Get());
	}

	if (Blueprint->IsUpToDate() && Blueprint->Status == BS_UpToDateWithWarnings)
	{
		UE_LOG(BlueprintScannerLog, Error, TEXT("Warning to compile %s"), *AssetPathString);
		WarningBlueprints.Add(Blueprint.Get());
	}
}

void FBlueprintScannerModule::DisplayDebugMessage(const UBlueprintScannerSettings* Settings)
{
	if (Settings->bShowDebugOnScreen)
	{
		for (const auto WarningBlueprint : WarningBlueprints)
		{
			FString Msg = FString::Printf(TEXT("Warning in Blueprint : %s"), *WarningBlueprint->GetFullName());
			GEngine->AddOnScreenDebugMessage(-1, Settings->TimeToDisplayForScreenMessage, FColor::Yellow, *Msg);
		}

		for (const auto PBlueprint : ErrorBlueprints)
		{
			FString Msg = FString::Printf(TEXT("Error in Blueprint : %s"), *PBlueprint->GetFullName());
			GEngine->AddOnScreenDebugMessage(-1, Settings->TimeToDisplayForScreenMessage, FColor::Red, *Msg);
		}
	}
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlueprintScannerModule, BlueprintScanner)