// Copyright Epic Games, Inc. All Rights Reserved.

#include "UNotepad.h"

#define LOCTEXT_NAMESPACE "FUNotepadModule"


namespace UNotepad
{
	namespace ModuleName
	{
		const FName ToolProjectEditor(TEXT("ToolProjectEditor"));
	}
}

const FName FUNotepadModule::UNotepadTabName = FName("UNotepad");





void FUNotepadModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		UNotepadTabName,
		FOnSpawnTab::CreateRaw(this, &FUNotepadModule::CreateNotepadManagerTab))
		.SetDisplayName(LOCTEXT("FDataAssetManagerModule", "Data Asset Manager"))
		.SetMenuType(GetVisibleModule());
}

void FUNotepadModule::ShutdownModule()
{

}

void FUNotepadModule::OpenManagerTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UNotepadTabName);
}

TSharedRef<SDockTab> FUNotepadModule::CreateNotepadManagerTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> UNotepadTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);

	return UNotepadTab;
}

ETabSpawnerMenuType::Type FUNotepadModule::GetVisibleModule() const
{
	if(FModuleManager::Get().IsModuleLoaded(UNotepad::ModuleName::ToolProjectEditor))
	{
		return ETabSpawnerMenuType::Enabled;
	}
	return ETabSpawnerMenuType::Hidden;
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUNotepadModule, UNotepad)