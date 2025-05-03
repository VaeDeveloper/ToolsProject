// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UBlueprintValidatorBase;

class FValidatorXModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	void HandlePostEngineInit();

	TSharedRef<SDockTab> OnSpawnValidatorXTab(const FSpawnTabArgs& Args);

	/** Validators */
	TArray <TSharedPtr<UBlueprintValidatorBase>> Validators;
};
