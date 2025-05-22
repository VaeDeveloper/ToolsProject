// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/AssetCleanerSubsystem.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace AssetCleaner
{
	namespace Constants
	{
		// modules
		static const FName ModuleAssetRegistry{ TEXT("AssetRegistry") };
		static const FName ModuleAssetTools{ TEXT("AssetTools") };
		static const FName ModuleContentBrowser{ TEXT("ContentBrowser") };
		static const FName ModulePropertyEditor{ TEXT("PropertyEditor") };
		static const FName ModuleMegascans{ TEXT("MegascansPlugin") };
	}
}


#if WITH_EDITOR
void UAssetCleanerSubsystem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SaveConfig();
}
FAssetRegistryModule& UAssetCleanerSubsystem::GetAssetRegistryModule()
{
	return FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetCleaner::Constants::ModuleAssetRegistry);
}

FAssetToolsModule& UAssetCleanerSubsystem::GetAssetToolsModule()
{
	return FModuleManager::LoadModuleChecked<FAssetToolsModule>(AssetCleaner::Constants::ModuleAssetTools);
}

FContentBrowserModule& UAssetCleanerSubsystem::GetContentBrowserModule()
{
	return FModuleManager::LoadModuleChecked<FContentBrowserModule>(AssetCleaner::Constants::ModuleContentBrowser);
}

FPropertyEditorModule& UAssetCleanerSubsystem::GetPropertyEditorModule()
{
	return FModuleManager::LoadModuleChecked<FPropertyEditorModule>(AssetCleaner::Constants::ModulePropertyEditor);
}
#endif