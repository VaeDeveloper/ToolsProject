// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintScanner.h"
#include "BlueprintScannerStyle.h"
#include "BlueprintScannerCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName BlueprintScannerTabName("BlueprintScanner");

#define LOCTEXT_NAMESPACE "FBlueprintScannerModule"

void FBlueprintScannerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FBlueprintScannerStyle::Initialize();
	FBlueprintScannerStyle::ReloadTextures();

	FBlueprintScannerCommands::Register();
	
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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlueprintScannerModule, BlueprintScanner)