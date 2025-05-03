// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DataAssetManagerSettings.generated.h"


/**
 * 
 */
UCLASS(Config = Engine, defaultconfig)
class DATAASSETMANAGER_API UDataAssetManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UDataAssetManagerSettings();

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif


	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (RelativePath, LongPackageName))
	FDirectoryPath DefaultAssetCreationDirectory = { TEXT("/Game") };

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (RelativePath, LongPackageName))
	TArray<FDirectoryPath> ScannedAssetDirectories = { { TEXT("/Game") } };

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (AllowedClasses = "/Script/Engine.DataAsset"))
	TArray<TSubclassOf<UDataAsset>> ExcludedScanAssetTypes;

	//UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (AllowedClasses = "/Script/Engine.DataAsset"))
	//TArray<TSubclassOf<UDataAsset>> ExcludedValidationAssetTypes;
};


