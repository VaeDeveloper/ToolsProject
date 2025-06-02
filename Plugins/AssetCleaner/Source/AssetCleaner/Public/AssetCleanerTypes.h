// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "RevisionControlStyle/RevisionControlStyle.h"

namespace AssetCleaner
{
	namespace ModuleName
	{
		static const FName AssetTools = TEXT("AssetTools");
		static const FName AssetRegistry = TEXT("AssetRegistry");
		static const FName ContentBrowser = TEXT("ContentBrowser");
		static const FName DataValidation = TEXT("DataValidation");
		static const FName MessageLog = TEXT("MessageLog");
		static const FName OutputLog = TEXT("OutputLog");
		static const FName Settings = TEXT("Settings");
		static const FName PropertyEditor = TEXT("PropertyEditor");
		static const FName AssetCleaner = TEXT("AssetCleaner");
	}
	
	namespace IconStyle
	{
		static const FName AppStyle = FAppStyle::GetAppStyleSetName();
		static const FName RevisionControlStyle = FRevisionControlStyleManager::GetStyleSetName();
	}

	namespace Icons
	{
		// File Menu
		const FSlateIcon AddNewAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.AssetActions.ReimportAsset");
		const FSlateIcon SaveAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SaveAllCurrentFolder");
		const FSlateIcon SaveAll = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SaveAllCurrentFolder");
		const FSlateIcon Validate = FSlateIcon(IconStyle::AppStyle, "Icons.Adjust");
		// Assets Menu
		const FSlateIcon OpenAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ShowInExplorer");
		const FSlateIcon FindInCB = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ShowInExplorer");
		const FSlateIcon Copy = FSlateIcon(IconStyle::AppStyle, "GenericCommands.Copy");
		const FSlateIcon ReferenceViewer = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ReferenceViewer");
		const FSlateIcon SizeMap = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SizeMap");
		const FSlateIcon Audit = FSlateIcon(IconStyle::AppStyle, "Icons.Audit");
		const FSlateIcon RevisionControl = FSlateIcon(IconStyle::RevisionControlStyle, "RevisionControl.Actions.Diff");
		const FSlateIcon Duplicate = FSlateIcon(IconStyle::AppStyle, "Icons.Duplicate");
		const FSlateIcon Edit = FSlateIcon(IconStyle::AppStyle, "Icons.Edit");
		// Settings Menu
		const FSlateIcon MessageLog = FSlateIcon(IconStyle::AppStyle, "MessageLog.TabIcon");
		const FSlateIcon Visibility = FSlateIcon(IconStyle::AppStyle, "Icons.Visibility");
		const FSlateIcon Settings = FSlateIcon(IconStyle::AppStyle, "Icons.Settings");
		const FSlateIcon Refresh = FSlateIcon(IconStyle::AppStyle, "Icons.Refresh");
		const FSlateIcon OutputLog = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Log.TabIcon");

		// Help Menu
		const FSlateIcon Documentation = FSlateIcon(IconStyle::AppStyle, "GraphEditor.GoToDocumentation");
	}



	static FName PathRoot{ TEXT("/Game") };
}



struct FStatItem
{
	FText Name;
	FText Num;
	FText Size;
	FText TooltipName;
	FText ToolTipNum;
	FText ToolTipSize;
	FLinearColor TextColor{ FLinearColor::White };
	FMargin NamePadding{ FMargin{0.0f} };
};

struct FAssetTreeFolderNode
{
	FString FolderPath;
	FString FolderName;
	TSharedPtr<FAssetTreeFolderNode> Parent;
	TArray<TSharedPtr<FAssetTreeFolderNode>> SubItems;

	int32 NumAssetsTotal = 0;
	int32 NumAssetsUsed = 0;
	int32 NumAssetsUnused = 0;
	float SizeAssetsUnused = 0.0f;
	float PercentageUnused = 0.0f;
	float PercentageUnusedNormalized = 0.0f;

	bool bIsDev : 1;
	bool bIsRoot : 1;
	bool bIsEmpty : 1;
	bool bIsExcluded : 1;
	bool bIsExpanded : 1;
	bool bIsVisible : 1;

	bool operator==(const FAssetTreeFolderNode& Other) const
	{
		return FolderPath.Equals(Other.FolderPath);
	}

	bool operator!=(const FAssetTreeFolderNode& Other) const
	{
		return !(*this == Other);
	}
};

