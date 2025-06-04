// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class ISceneOutlinerColumn;
class ISceneOutliner;

class FOutlinerToolkitModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RegisterMenus();
	void EntryFunctionWithContext();

private:
	void InitCustomSceneOutlinerColumn();
	TSharedRef<ISceneOutlinerColumn> OnCreateSimulatePhysics(ISceneOutliner& SceneOutliner);
	TSharedRef<ISceneOutlinerColumn> OnCreateMobility(ISceneOutliner& SceneOutliner);
	TSharedRef<ISceneOutlinerColumn> OnCreateHiddenInGame(ISceneOutliner& SceneOutliner);
};
