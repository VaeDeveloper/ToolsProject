// Fill out your copyright notice in the Description page of Project Settings.


#include "DeveloperSettings/DataAssetManagerSettings.h"

#define LOCTEXT_NAMESPACE "DataAssetManager"

UDataAssetManagerSettings::UDataAssetManagerSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("DataAssetManager");
}

FText UDataAssetManagerSettings::GetSectionText() const
{
	return LOCTEXT("SettingsDisplayName", "DataAssetManager");
}

#undef LOCTEXT_NAMESPACE