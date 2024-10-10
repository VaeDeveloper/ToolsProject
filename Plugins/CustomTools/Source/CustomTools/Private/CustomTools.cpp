// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomTools.h"
#include "CustomToolsStyle.h"
#include "CustomToolsCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName CustomToolsTabName("CustomTools");

#define LOCTEXT_NAMESPACE "FCustomToolsModule"

void FCustomToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomToolsModule, CustomTools)