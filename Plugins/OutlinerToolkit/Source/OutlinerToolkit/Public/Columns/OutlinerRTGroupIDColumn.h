// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISceneOutlinerColumn.h"

/**
 * FOutlinerRTGroupIDColumn
 *
 * Custom column for the Scene Outliner that displays Ray Tracing Group ID
 * for actors and primitive components.
 *
 * Supports sorting by Group ID and shows fallback text when data is invalid.
 */
class OUTLINERTOOLKIT_API FOutlinerRTGroupIDColumn : public ISceneOutlinerColumn
{
public:
	/** Constructor that initializes the column with the given Scene Outliner reference. */
	FOutlinerRTGroupIDColumn(ISceneOutliner& SceneOutliner);

	/** Returns the column ID name for this column. */
	virtual FName GetColumnID() { return FName("RayTracingGroupID"); }

	/** Returns the static column ID used for registration. */
	static FName GetID() { return FName("RayTracingGroupID"); }

	/** Constructs the header row configuration for this column. */
	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;
	
	/** Returns whether this column supports sorting. */
	virtual bool SupportsSorting() const override { return true; }

	/** Constructs the widget to display for a specific tree item in this column. */
	virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;

	/** Sorts the given array of tree items based on the current sort mode. */
	virtual void SortItems(TArray<FSceneOutlinerTreeItemPtr>& InOutItems, const EColumnSortMode::Type InSortMode) const override;

private:
	/** Returns a string representation of the Ray Tracing Group ID for the given tree item. */
	FString GetRayTracingGroupIdString(FSceneOutlinerTreeItemRef TreeItem) const;

	/** Returns the current sort mode of this column. */
	EColumnSortMode::Type GetColumnSortMode() const;

	/** Called when the column sort mode is changed by the user. */
	void OnColumnSortModeChanged(EColumnSortPriority::Type PriorityType, const FName& Name, EColumnSortMode::Type SortMode);

	/** The current sort mode of the column (ascending, descending, or none). */
	EColumnSortMode::Type CurrentSortMode = EColumnSortMode::None;

	/** Pointer to the owning Scene Outliner instance. */
	ISceneOutliner* SceneOutliner = nullptr;
};
