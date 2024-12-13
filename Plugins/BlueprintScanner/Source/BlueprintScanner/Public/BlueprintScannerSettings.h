// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/DeveloperSettings.h"
#include "BlueprintScannerSettings.generated.h"

/**
 *
 */
UCLASS(config = Engine, defaultconfig)
class BLUEPRINTSCANNER_API UBlueprintScannerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBlueprintScannerSettings();


#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif

	UPROPERTY(config, EditAnywhere, Category = "Behavior")
	bool bCompileBlueprints;

	UPROPERTY(config, EditAnywhere, Category = "Search")
	bool bRefreshLevelBlueprints;

	UPROPERTY(config, EditAnywhere, Category = "Search")
	bool bRefreshGameBlueprints;

	UPROPERTY(config, EditAnywhere, Category = "Search")
	bool bRefreshEngineBlueprints;

	UPROPERTY(config, EditAnywhere, Category = "Search")
	TArray<FName> AdditionalBlueprintPaths;

	UPROPERTY(config, EditAnywhere, Category = "Search")
	TArray<FName> ExcludeBlueprintPaths;

	UPROPERTY(config, EditAnywhere, Category = "Debug")
	bool bShowDebugOnScreen;

	UPROPERTY(config, EditAnywhere, Category = "Debug")
	bool bConsoleMessageLog;

	UPROPERTY(config, EditAnywhere, Category = "Debug")
	int32 TimeToDisplayForScreenMessage = 5.0f;


};
