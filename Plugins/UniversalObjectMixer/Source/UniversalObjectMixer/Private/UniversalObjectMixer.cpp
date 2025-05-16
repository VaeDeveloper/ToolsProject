// Copyright Epic Games, Inc. All Rights Reserved.

#include "UniversalObjectMixer.h"
#include "Filters/RayTracingObjectFilter.h"

#define LOCTEXT_NAMESPACE "FUniversalObjectMixerModule"

void FUniversalObjectMixerModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUniversalObjectMixerModule::Initialize);

}

void FUniversalObjectMixerModule::ShutdownModule()
{
	Teardown();
}

void FUniversalObjectMixerModule::Initialize()
{
	FObjectMixerEditorModule::Initialize();
	// DefaultFilterClass = URayTracingObjectFilter::StaticClass();
}

FName FUniversalObjectMixerModule::GetModuleName() const
{
	return FName("UniversalObjectMixer");
}

void FUniversalObjectMixerModule::SetupMenuItemVariables()
{
	TabLabel = LOCTEXT("UniversalObjectMixerTabLabel", "Universal Object Mixer");

	MenuItemName = LOCTEXT("UniversalObjectMixerEditorMenuItem", "Object Mixer");
	MenuItemIcon = FSlateIcon(FAppStyle::Get().GetStyleSetName(), "LevelEditor.GameSettings");
	MenuItemTooltip = LOCTEXT("OpenUniversalObjectMixerEditorTooltip", "Open Universal Object Mixer");
}

FName FUniversalObjectMixerModule::GetTabSpawnerId()
{
	return FName("UniversalObjectMixer");
}

void FUniversalObjectMixerModule::RegisterSettings() const
{
}

void FUniversalObjectMixerModule::UnregisterSettings() const
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUniversalObjectMixerModule, UniversalObjectMixer)