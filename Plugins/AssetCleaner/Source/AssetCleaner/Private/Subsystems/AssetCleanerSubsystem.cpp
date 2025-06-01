// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/AssetCleanerSubsystem.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "AssetManagerEditorModule.h"
#include "AssetCleaner.h"
#include "ObjectTools.h"


DEFINE_LOG_CATEGORY_STATIC(AssetCleanerSubsystemLog, All, All);

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
		static const FName ModuleAssetCleaner{ TEXT("AssetCleaner") };
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
FAssetCleanerModule& UAssetCleanerSubsystem::GetAssetCleanerModule()
{
	return FModuleManager::LoadModuleChecked<FAssetCleanerModule>(AssetCleaner::Constants::ModuleAssetCleaner);
}
void UAssetCleanerSubsystem::ProcessAssetData(const TArray<FAssetData>& RefAssetData, TFunction<void(const TArray<FAssetIdentifier>&)> ProcessFunction)
{

	TArray<FAssetIdentifier> AssetIdentifiers;
	IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList(RefAssetData, AssetIdentifiers);
	ProcessFunction(AssetIdentifiers);

}
bool UAssetCleanerSubsystem::IsExcludedFolder(const FString& FolderPath)
{
	return FolderPath.Contains(TEXT("Developers"))
		|| FolderPath.Contains(TEXT("Collections"))
		|| FolderPath.Contains(TEXT("__ExternalActors__"))
		|| FolderPath.Contains(TEXT("__ExternalObjects__"));
	
}
bool UAssetCleanerSubsystem::DeleteMultiplyAsset(const TArray<FAssetData>& Assets)
{
	if(Assets.Num() == 0)
	{
		UE_LOG(AssetCleanerSubsystemLog, Warning, TEXT("%s No assets to delete!"), *FString(__FUNCTION__));
		return false;
	}

	int32 DeletedCount = ObjectTools::DeleteAssets(Assets);
	UE_LOG(AssetCleanerSubsystemLog, Log, TEXT("%s Deleted %d assets"), *FString(__FUNCTION__), DeletedCount);

	return DeletedCount > 0;
}
#endif