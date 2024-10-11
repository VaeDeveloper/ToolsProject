// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomTools.h"
#include "CustomToolsStyle.h"
#include "CustomToolsCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "EditorAssetLibrary.h"
#include "SlateWidget/SFolderCleaningWidget.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "AssetViewUtils.h"


static const FName CustomToolsTabName("CustomTools");

#define LOCTEXT_NAMESPACE "FCustomToolsModule"

void FCustomToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	/* folder cleaner initialization and registration */
	InitializeMenuExtention();
	RegisterFolderCleanerTabs();

	FCustomToolsStyle::Initialize();
	FCustomToolsStyle::ReloadTextures();

	FCustomToolsCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FCustomToolsCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FCustomToolsModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCustomToolsModule::RegisterMenus));
}

void FCustomToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FCustomToolsStyle::Shutdown();

	FCustomToolsCommands::Unregister();
}

void FCustomToolsModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
		LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
		FText::FromString(TEXT("FCustomToolsModule::PluginButtonClicked()")),
		FText::FromString(TEXT("CustomTools.cpp"))
	);
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FCustomToolsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCustomToolsCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FCustomToolsCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FCustomToolsModule::InitializeMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FCustomToolsModule::CustomMenuExtender));
}

void FCustomToolsModule::RegisterFolderCleanerTabs()
{
	/* clang-format off */
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("FolderCleaner"),			//
		FOnSpawnTab::CreateRaw(this, &FCustomToolsModule::OnSpawnFolderCleanerTab))		//
		.SetDisplayName(FText::FromString(TEXT("FolderCleaner")));						//
}

TSharedRef<SDockTab> FCustomToolsModule::OnSpawnFolderCleanerTab(const FSpawnTabArgs& TabArgs)
{
	return TSharedRef<SDockTab>();
}

/**
 * @brief Extends the custom menu based on the selected paths in the Content Browser.
 *
 * @param SelectedPaths An array of strings representing the paths selected by the user in the
 * Content Browser. This function adds a custom "Delete" menu option for paths that have been selected.
 * @return A shared reference to an FExtender object that contains the extended menu options.
 */
TSharedRef<FExtender> FCustomToolsModule::CustomMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FCustomToolsModule::AddMenuEntry));

		FolderPathsSelected = SelectedPaths;
	}
	return TSharedRef<FExtender>::TSharedRef();
}

void FCustomToolsModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("FolderCleaner")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
		FExecuteAction::CreateRaw(this, &FCustomToolsModule::OnFolderCleanerButtonClicked));
}

void FCustomToolsModule::OnFolderCleanerButtonClicked()
{
	// FixupRedirectors();
	FGlobalTabmanager::Get()->TryInvokeTab(FName("FolderCleaner"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCustomToolsModule, CustomTools)