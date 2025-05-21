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
		static const FName ModulePjcName{ TEXT("ProjectCleaner") };
		static const FName ModulePjcStylesName{ TEXT("ProjectCleanerStyles") };
		static const FName ModulePjcTitle{ TEXT("Project Cleaner") };
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
FAssetRegistryModule& UAssetCleanerSubsystem::GetModuleAssetRegistry()
{
	return FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetCleaner::Constants::ModuleAssetRegistry);
}

FAssetToolsModule& UAssetCleanerSubsystem::GetModuleAssetTools()
{
	return FModuleManager::LoadModuleChecked<FAssetToolsModule>(AssetCleaner::Constants::ModuleAssetTools);
}

FContentBrowserModule& UAssetCleanerSubsystem::GetModuleContentBrowser()
{
	return FModuleManager::LoadModuleChecked<FContentBrowserModule>(AssetCleaner::Constants::ModuleContentBrowser);
}

FPropertyEditorModule& UAssetCleanerSubsystem::GetModulePropertyEditor()
{
	return FModuleManager::LoadModuleChecked<FPropertyEditorModule>(AssetCleaner::Constants::ModulePropertyEditor);
}
#endif