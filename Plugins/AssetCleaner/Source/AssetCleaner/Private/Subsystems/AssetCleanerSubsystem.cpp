// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/AssetCleanerSubsystem.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "AssetManagerEditorModule.h"
#include "AssetCleaner.h"
#include "AssetCleanerTypes.h"

#include "ObjectTools.h"


DEFINE_LOG_CATEGORY_STATIC(AssetCleanerSubsystemLog, All, All);


#if WITH_EDITOR
void UAssetCleanerSubsystem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SaveConfig();
}
#endif

FAssetRegistryModule& UAssetCleanerSubsystem::GetAssetRegistryModule()
{
	return FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetCleaner::ModuleName::AssetRegistry);
}

FAssetToolsModule& UAssetCleanerSubsystem::GetAssetToolsModule()
{
	return FModuleManager::LoadModuleChecked<FAssetToolsModule>(AssetCleaner::ModuleName::AssetTools);
}

FContentBrowserModule& UAssetCleanerSubsystem::GetContentBrowserModule()
{
	return FModuleManager::LoadModuleChecked<FContentBrowserModule>(AssetCleaner::ModuleName::ContentBrowser);
}

FPropertyEditorModule& UAssetCleanerSubsystem::GetPropertyEditorModule()
{
	return FModuleManager::LoadModuleChecked<FPropertyEditorModule>(AssetCleaner::ModuleName::PropertyEditor);
}

FAssetCleanerModule& UAssetCleanerSubsystem::GetAssetCleanerModule()
{
	return FModuleManager::LoadModuleChecked<FAssetCleanerModule>(AssetCleaner::ModuleName::AssetCleaner);
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
	UE_LOG(AssetCleanerSubsystemLog, Log, TEXT("%s Deleted %d assets"), ANSI_TO_TCHAR(__FUNCTION__), DeletedCount);

	return DeletedCount > 0;
}

int64 UAssetCleanerSubsystem::ParseSizeString(const FString& SizeString)
{
	FString NumberPart, UnitPart;
	SizeString.TrimStartAndEnd().Split(TEXT(" "), &NumberPart, &UnitPart);

	double Number = FCString::Atod(*NumberPart);
	int64 Multiplier = 1;

	if(UnitPart.Equals(TEXT("KB"), ESearchCase::IgnoreCase))      Multiplier = 1024;
	else if(UnitPart.Equals(TEXT("MB"), ESearchCase::IgnoreCase)) Multiplier = 1024 * 1024;
	else if(UnitPart.Equals(TEXT("GB"), ESearchCase::IgnoreCase)) Multiplier = 1024LL * 1024 * 1024;

	return static_cast<int64>(Number * Multiplier);
}

FString UAssetCleanerSubsystem::PathNormalize(const FString& InPath)
{
	if(InPath.IsEmpty()) return {};

	// Ensure the path dont starts with a slash or a disk drive letter
	if(!(InPath.StartsWith(TEXT("/")) || InPath.StartsWith(TEXT("\\")) || (InPath.Len() > 2 && InPath[1] == ':')))
	{
		return {};
	}

	FString Path = FPaths::ConvertRelativePathToFull(InPath).TrimStartAndEnd();
	FPaths::RemoveDuplicateSlashes(Path);

	// Collapse any ".." or "." references in the path
	FPaths::CollapseRelativeDirectories(Path);

	if(FPaths::GetExtension(Path).IsEmpty())
	{
		FPaths::NormalizeDirectoryName(Path);
	}
	else
	{
		FPaths::NormalizeFilename(Path);
	}

	// Ensure the path does not end with a trailing slash
	if(Path.EndsWith(TEXT("/")) || Path.EndsWith(TEXT("\\")))
	{
		Path = Path.LeftChop(1);
	}

	return Path;
}

FString UAssetCleanerSubsystem::PathConvertToRelative(const FString& InPath)
{
	const FString PathNormalized = PathNormalize(InPath);
	const FString PathProjectContent = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()).LeftChop(1);

	if(PathNormalized.IsEmpty()) return {};
	if(PathNormalized.StartsWith(AssetCleaner::PathRoot.ToString())) return PathNormalized;
	if(PathNormalized.StartsWith(PathProjectContent))
	{
		FString Path = PathNormalized;
		Path.RemoveFromStart(PathProjectContent);

		return Path.IsEmpty() ? AssetCleaner::PathRoot.ToString() : AssetCleaner::PathRoot.ToString() / Path;
	}

	return {};
}

FString UAssetCleanerSubsystem::GetPathExternalActors()
{
	return FString::Printf(TEXT("/Game/%s"), FPackagePath::GetExternalActorsFolderName());
}

FString UAssetCleanerSubsystem::GetPathExternalObjects()
{
	return FString::Printf(TEXT("/Game/%s"), FPackagePath::GetExternalObjectsFolderName());
}

bool UAssetCleanerSubsystem::FolderIsExternal(const FString& InPath)
{
	return InPath.StartsWith(GetPathExternalActors()) || InPath.StartsWith(GetPathExternalObjects());
}

FString UAssetCleanerSubsystem::PathConvertToAbsolute(const FString& InPath)
{
	const FString PathNormalized = PathNormalize(InPath);
	const FString PathProjectContent = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()).LeftChop(1);

	if(PathNormalized.IsEmpty()) return {};
	if(PathNormalized.StartsWith(PathProjectContent)) return PathNormalized;
	if(PathNormalized.StartsWith(AssetCleaner::PathRoot.ToString()))
	{
		FString Path = PathNormalized;
		Path.RemoveFromStart(AssetCleaner::PathRoot.ToString());

		return Path.IsEmpty() ? PathProjectContent : PathProjectContent / Path;
	}

	return {};
}

bool UAssetCleanerSubsystem::FolderIsEmpty(const FString& InPath)
{
	if(InPath.IsEmpty()) return false;

	const FName PathRel = FName(*PathConvertToRelative(InPath));
	if(const bool HasAsset = UAssetCleanerSubsystem::GetAssetRegistryModule().Get().HasAssets(PathRel, true))
	{
		return false;
	}

	const FString PathAbs = PathConvertToAbsolute(InPath);
	if(PathAbs.IsEmpty()) return false;

	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *PathAbs, TEXT("*"), true, false);

	return Files.Num() == 0;
}
