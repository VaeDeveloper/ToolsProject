// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"


// TODO !!! move in function library in future
namespace AssetCleaner::Private
{
	FORCEINLINE FString GetAssetDiskSize(const FAssetData& AssetData)
	{
		FString PackageFileName;
		if(FPackageName::DoesPackageExist(AssetData.PackageName.ToString(), &PackageFileName))
		{
			const int64 FileSize = IFileManager::Get().FileSize(*PackageFileName);

			if(FileSize == INDEX_NONE)
			{
				return TEXT("Unknown");
			}

			const double SizeInKb = static_cast<double>(FileSize) / 1024.0;
			if(SizeInKb >= 1024)
			{
				const double SizeInMb = SizeInKb / 1024.0;
				return FString::Printf(TEXT("%.1f Mb"), SizeInMb);
			}

			return FString::Printf(TEXT("%.1f Kb"), SizeInKb);
		}
		return TEXT("Unknown");
	}
}




namespace AssetCleanerListColumns
{
	/** IDs for list columns */
	static const FName ColumnID_RC("RevisionControl");
	static const FName ColumnID_Name("Name");
	static const FName ColumnID_Type("Type");
	static const FName ColumnID_DiskSize("DiskSize");
	static const FName ColumnID_Path("Path");
}

/**
 * SAssetCleanerTableRow
 *
 * Represents a single row in the asset cleaner table view.
 * Displays information about a specific asset and supports interactions such as renaming, context menu, and double-click.
 */
class SAssetCleanerTableRow final : public SMultiColumnTableRow<TSharedPtr<FAssetData>>
{
public:
	/** Delegate triggered when an asset is renamed. */
	DECLARE_DELEGATE_ThreeParams(FOnAssetRenamed, TSharedPtr<FAssetData>, const FText&, ETextCommit::Type);

	/** Delegate triggered when a context menu is requested. */
	DECLARE_DELEGATE_TwoParams(FOnCreateContextMenu, const FGeometry&, const FPointerEvent&);

	/** Delegate triggered when an asset row is double-clicked. */
	DECLARE_DELEGATE_TwoParams(FOnAssetDoubleClicked, const FGeometry&, const FPointerEvent&);

	/** Delegate used to register editable text widgets (used during renaming). */
	DECLARE_DELEGATE_TwoParams(FOnRegisterEditableText, TSharedPtr<FAssetData>, TSharedRef<SEditableText>);

	/** Delegate triggered when a mouse button is pressed on the asset row. */
DECLARE_DELEGATE_RetVal_TwoParams(FReply, FOnAssetMouseButtonDown, const FGeometry&, const FPointerEvent&); 

public:
	SLATE_BEGIN_ARGS(SAssetCleanerTableRow) {}
		/** The asset item represented by this row. */
		SLATE_ARGUMENT(TSharedPtr<FAssetData>, Item)

		/** Called when the asset is renamed. */
		SLATE_EVENT(FOnAssetRenamed, OnAssetRenamed)

		/** Called when a context menu should be created. */
		SLATE_EVENT(FOnCreateContextMenu, OnCreateContextMenu)

		/** Called when the row is double-clicked. */
		SLATE_EVENT(FOnAssetDoubleClicked, OnAssetDoubleClicked)

		/** Called to register editable text widgets. */
		SLATE_EVENT(FOnRegisterEditableText, OnRegisterEditableText)

		/** Called when the row receives a mouse down event. */
		SLATE_EVENT(FOnAssetMouseButtonDown, OnMouseButtonDown)
	SLATE_END_ARGS()

	/**
	 * @param InArgs Arguments passed via Slate syntax.
	 * @param InOwnerTable The parent table view.
	 */
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable);

	/** Destructor. */
	virtual ~SAssetCleanerTableRow();

	/**
	 * Generates the widget for a specific column in this row.
	 *
	 * @param ColumnId The name of the column.
	 * @return The widget to display in the specified column.
	 */
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnId) override;

	/** 
	 * @param AllottedGeometry The geometry assigned to this widget.
	 * @param InCurrentTime The current time.
	 * @param InDeltaTime The time since the last frame.
	 */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/**
	 * Registers a callback to update the "dirty" state when a package changes.
	 *
	 * @param PackageName The name of the package being tracked.
	 */
	void AddDirtyEventHandler(const FString& PackageName);

	/**
	 * Handles mouse button down events on the border area.
	 */
	FReply BorderMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);

	/**
	 * Handles double-click events on the border area.
	 */
	FReply BorderMouseDoubleClicked(const FGeometry& InGeometry, const FPointerEvent& MouseEvent);

private:
	/** Whether the asset is currently marked as dirty (modified). */
	bool bIsDirty = false;

	/** The asset data this row represents. */
	TSharedPtr<FAssetData> Item = nullptr;

	/** The widget used to display a "dirty" indicator icon. */
	TSharedPtr<SImage> DirtyBrushWidget = nullptr;

	/** Delegate called when the asset is renamed. */
	FOnAssetRenamed OnAssetRenamed{};

	/** Delegate called when a context menu is requested. */
	FOnCreateContextMenu OnCreateContextMenu{};

	/** Delegate called when the row is double-clicked. */
	FOnAssetDoubleClicked OnAssetDoubleClicked{};

	/** Delegate used to register editable text widgets. */
	FOnRegisterEditableText OnRegisterEditableText{};

	/** Delegate triggered on mouse button down. */
	FOnAssetMouseButtonDown MouseButtonDown{};

	/** Handle to the delegate binding for dirty state changes. */
	FDelegateHandle OnPackageDirtyStateChangedHandle{};
};