// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AssetCleanerSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Engine, defaultconfig)
class ASSETCLEANER_API UAssetCleanerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UAssetCleanerSettings();

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
};
