// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IUNotepadModule : public IModuleInterface
{
public:
	virtual void OpenManagerTab() = 0;
};

class FUNotepadModule : public IUNotepadModule
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void OpenManagerTab() override;

	static const FName UNotepadTabName;


private:
	TSharedRef<SDockTab> CreateNotepadManagerTab(const FSpawnTabArgs& Args);
	ETabSpawnerMenuType::Type GetVisibleModule() const;

};
