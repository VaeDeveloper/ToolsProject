// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DataAssetManagerSettings.generated.h"


/**
 * Settings for the Data Asset Manager.
 *
 * Settings are saved in the configuration file, and any changes require the module or editor to be reloaded.
 */
UCLASS(Config = Engine, defaultconfig)
class DATAASSETMANAGER_API UDataAssetManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UDataAssetManagerSettings();

#if WITH_EDITOR
	/**
	 * @brief Override method to get the section text for settings in the editor.
	 *
	 * This method allows setting a custom text for the settings section in the editor UI.
	 *
	 * @return The text that will be displayed in the UI for the settings section.
	 */
	virtual FText GetSectionText() const override;
#endif

	/** Default directory path for asset creation. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (RelativePath, LongPackageName))
	FDirectoryPath DefaultAssetCreationDirectory = { TEXT("/Game") };

	/** List of directories to scan for assets. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (RelativePath, LongPackageName))
	TArray<FDirectoryPath> ScannedAssetDirectories = { { TEXT("/Game") } };

	/** List of asset types to exclude from scanning. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (AllowedClasses = "/Script/Engine.DataAsset"))
	TArray<TSubclassOf<UDataAsset>> ExcludedScanAssetTypes;

	/** URL to the online documentation. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FString DocumentationURL; 
};


