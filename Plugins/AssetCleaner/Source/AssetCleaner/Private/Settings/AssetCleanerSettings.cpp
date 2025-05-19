// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/AssetCleanerSettings.h"


#define LOCTEXT_NAMESPACE "AssetCleaner"


UAssetCleanerSettings::UAssetCleanerSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("AssetCleaner");
}

#if WITH_EDITOR
FText UAssetCleanerSettings::GetSectionText() const
{
	return LOCTEXT("SettingsDisplayName", "AssetCleaner");
}
#endif

#undef LOCTEXT_NAMESPACE