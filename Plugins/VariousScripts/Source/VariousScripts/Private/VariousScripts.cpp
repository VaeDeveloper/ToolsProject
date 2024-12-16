// Copyright Epic Games, Inc. All Rights Reserved.

#include "VariousScripts.h"
#include "VariousScriptsStyle.h"
#include "VariousScriptsCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName VariousScriptsTabName("VariousScripts");

#define LOCTEXT_NAMESPACE "FVariousScriptsModule"

void FVariousScriptsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FVariousScriptsStyle::Initialize();
	FVariousScriptsStyle::ReloadTextures();

	FVariousScriptsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVariousScriptsCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FVariousScriptsModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVariousScriptsModule::RegisterMenus));
}

void FVariousScriptsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FVariousScriptsStyle::Shutdown();

	FVariousScriptsCommands::Unregister();
}

void FVariousScriptsModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FVariousScriptsModule::PluginButtonClicked()")),
							FText::FromString(TEXT("VariousScripts.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FVariousScriptsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FVariousScriptsCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVariousScriptsCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVariousScriptsModule, VariousScripts)