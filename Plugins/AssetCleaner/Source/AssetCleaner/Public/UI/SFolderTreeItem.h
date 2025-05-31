// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetCleanerTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Notifications/SProgressBar.h"



namespace FolderItemTreeID
{
	/** IDs for list columns */
	static const FName ColumnID_Path("Path");
	static const FName ColumnID_NumAssetsTotal("NumAssetsTotal");
	static const FName ColumnID_NumAssetsUsed("NumAssetsUsed");
	static const FName ColumnID_NumAssetsUnused("NumAssetsUnused");
	static const FName ColumnID_UnusedPercent("UnusedPercent");
	static const FName ColumnID_UnusedSize("UnusedSize");

}

/**
 * A single row widget used to represent a folder item in an asset tree view.
 * Implements multi-column support for folder display within an STreeView.
 */
class SFolderItemTree final : public SMultiColumnTableRow<TSharedPtr<SFolderItemTree>>
{
public:
	SLATE_BEGIN_ARGS(SFolderItemTree) {}
		/** The data model representing this folder node. */
		SLATE_ARGUMENT(TSharedPtr<FAssetTreeFolderNode>, Item)

		/** The text to highlight within the folder name, typically used for search filtering. */
		SLATE_ARGUMENT(FText, HightlightText)
	SLATE_END_ARGS()

	/**
	 * Constructs the folder row widget using the specified arguments and owning table.
	 *
	 * @param InArgs Construction arguments passed via SLATE macros.
	 * @param InTable Reference to the parent table view widget.
	 */
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InTable);

	/**
	 * Generates the widget to use for the specified column name.
	 * Used by STreeView to populate each column in the row.
	 *
	 * @param InColumnName The name of the column for which to generate content.
	 * @return The widget representing the column's content.
	 */
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;

private:
	/**
	 * Gets the appropriate icon to use for this folder item.
	 *
	 * @return A pointer to the Slate brush representing the folder icon.
	 */
	const FSlateBrush* GetFolderIcon() const;

	/**
	 * Gets the color used to tint this folder item.
	 * This can be used to visually distinguish different folder states.
	 *
	 * @return A Slate color representing the folder's display tint.
	 */
	FSlateColor GetFolderColor() const;

	/** The text to be highlighted within the folder label. */
	FText HighlightText;

	/** The underlying data representing this folder item. */
	TSharedPtr<FAssetTreeFolderNode> Item;
};