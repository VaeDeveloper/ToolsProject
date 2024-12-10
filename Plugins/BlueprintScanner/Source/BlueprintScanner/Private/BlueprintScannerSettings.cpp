// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintScannerSettings.h"

#define LOCTEXT_NAMESPACE "BlueprintScanner"


UBlueprintScannerSettings::UBlueprintScannerSettings() 
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("Blueprint Scanner");

	bRefreshGameBlueprints = true;
	bCompileBlueprints = true;
	bShowDebugOnScreen = true;
}

#if WITH_EDITOR

FText UBlueprintScannerSettings::GetSectionText() const
{
	return LOCTEXT("SettingsDisplayName", "Blueprint Scanner");
}

#endif

#undef LOCTEXT_NAMESPACE
