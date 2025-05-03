// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataAssetManager.h"
#include "Modules/ModuleManager.h"
#include "Editor/EditorEngine.h"
#include "StatusBarSubsystem.h"
#include "TimerManager.h"
#include "DeveloperSettings/DataAssetManagerSettings.h"
#include "UI/SDataAssetManagerWidget.h"
#include "UI/SDeveloperSettingsWidget.h"

const FName FDataAssetManagerModule::DataAssetManagerTabName = FName("DataAssetManager");

#define LOCTEXT_NAMESPACE "FDataAssetManagerModule"

namespace DataAssetManager
{
	namespace ModuleName
	{
		const FName ToolProjectEditor(TEXT("ToolProjectEditor"));
	}

	constexpr float TabReopenDelaySeconds = 1.0f;
	const FName StatusBarName(TEXT("DataAssetManagerStatusBar"));
}

void FDataAssetManagerModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(DataAssetManager::ModuleName::PropertyEditor);
	PropertyEditorModule.RegisterCustomClassLayout(UDataAssetManagerSettings::StaticClass()->GetFName(),
	FOnGetDetailCustomizationInstance::CreateStatic(&SDeveloperSettingsWidget::MakeInstance));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
	DataAssetManagerTabName,
	FOnSpawnTab::CreateRaw(this, &FDataAssetManagerModule::CreateDataAssetManagerTab))
	.SetDisplayName(LOCTEXT("FDataAssetManagerModule", "Data Asset Manager"))
	.SetMenuType(GetVisibleModule());
}

void FDataAssetManagerModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded(DataAssetManager::ModuleName::PropertyEditor))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(DataAssetManager::ModuleName::PropertyEditor);
		PropertyEditorModule.UnregisterCustomClassLayout(UDataAssetManagerSettings::StaticClass()->GetFName());
	}
}

ETabSpawnerMenuType::Type FDataAssetManagerModule::GetVisibleModule() const
{
	if (FModuleManager::Get().IsModuleLoaded(DataAssetManager::ModuleName::ToolProjectEditor))
	{
		return ETabSpawnerMenuType::Enabled;
	}
	return ETabSpawnerMenuType::Hidden;
}

TSharedRef<SDockTab> FDataAssetManagerModule::CreateDataAssetManagerTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> DataAssetManagerTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	UStatusBarSubsystem* StatusBarSubsystem = GEditor->GetEditorSubsystem<UStatusBarSubsystem>();
	if (StatusBarSubsystem)
	{
		TSharedRef<SWidget> StatusBarWidget = StatusBarSubsystem->MakeStatusBarWidget(DataAssetManager::StatusBarName, DataAssetManagerTab);

		DataAssetManagerTab->SetContent(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f) 
			[
				SNew(SDataAssetManagerWidget)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				StatusBarWidget
			]
		);
	}
	else
	{
		DataAssetManagerTab->SetContent(SNew(SDataAssetManagerWidget));
	}

	return DataAssetManagerTab;
}

void FDataAssetManagerModule::RestartWidget()
{
	TSharedPtr<SDockTab> DataAssetManagerTab = FGlobalTabmanager::Get()->FindExistingLiveTab(DataAssetManagerTabName);
	if (DataAssetManagerTab.IsValid())
	{
		DataAssetManagerTab->RequestCloseTab();

		// Set timer to reopen the tab after a short delay.
		FTimerHandle TimerHandle;
		GEditor->GetTimerManager().Get().SetTimer(TimerHandle, []()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(DataAssetManagerTabName);
			}, DataAssetManager::TabReopenDelaySeconds, false);
	}
}

void FDataAssetManagerModule::OpenDataAssetManagerTab() 
{
    FGlobalTabmanager::Get()->TryInvokeTab(DataAssetManagerTabName);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDataAssetManagerModule, DataAssetManager)