// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISceneOutlinerColumn.h"


/**
 * Custom Scene Outliner column for displaying and toggling "Simulate Physics" property on actors.
 * This column adds a checkbox next to each actor in the Outliner, allowing users to enable/disable physics simulation.
 */
class OUTLINERTOOLKIT_API FOutlinerSimulatePhysicsColumn : public ISceneOutlinerColumn
{
public:
	/** Constructor. Initializes the column with a reference to the parent Scene Outliner. */
	FOutlinerSimulatePhysicsColumn(ISceneOutliner& SceneOutliner);

	//~ Begin ISceneOutlinerColumn Interface

	/**
	 * Returns the unique identifier for this column.
	 * @return Column ID as an FName ("SimulatePhysics").
	 */
	virtual FName GetColumnID() override { return FName("SimulatePhysics"); }

	/**
	 * Static accessor for the column's ID. Used during column registration.
	 * @return Column ID as an FName ("SimulatePhysics").
	 */
	static FName GetID() { return FName("SimulatePhysics"); }

	/**
	 * Configures the header row entry for this column.
	 * @return Slate arguments defining the header appearance (e.g., tooltip, visibility).
	 */
	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;

	/**
	 * Determines if this column supports sorting (this column does not).
	 * @return Always false, as physics simulation state is not sortable.
	 */
	virtual bool SupportsSorting() const override { return false; }

	/**
	 * Generates the widget for a specific Outliner tree item in this column.
	 * @param TreeItem  The item (actor, folder, etc.) to create a widget for.
	 * @param Row       The parent table row containing the item.
	 * @return A checkbox widget bound to the actor's "Simulate Physics" property.
	 */
	virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;

	//~ End ISceneOutlinerColumn Interface

private:
	/**
	 * reference to the parent Scene Outliner.
	 * Used to interact with the Outliner's data and refresh state when needed.
	 */
	ISceneOutliner* SceneOutliner;
};