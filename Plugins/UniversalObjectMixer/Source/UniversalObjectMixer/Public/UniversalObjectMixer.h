// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ObjectMixerEditorModule.h"

class FUniversalObjectMixerModule : public FObjectMixerEditorModule
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


#pragma region FObjectMixerEditorModule 
	virtual void Initialize() override;
	virtual FName GetModuleName() const override;
	virtual void SetupMenuItemVariables() override;
	virtual FName GetTabSpawnerId() override;
	virtual void RegisterSettings() const override;
	virtual void UnregisterSettings() const override;
#pragma endregion FObjectMixerEditorModule 
};
