// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISceneOutlinerColumn.h"


class OUTLINERTOOLKIT_API FOutlinerSimulatePhysicsColumn : public ISceneOutlinerColumn
{
public:
	FOutlinerSimulatePhysicsColumn(ISceneOutliner& SceneOutliner);

	/** Returns the column ID name for this column. */
	virtual FName GetColumnID() { return FName("SimulatePhysics"); }

	/** Returns the static column ID used for registration. */
	static FName GetID() { return FName("SimulatePhysics"); }

	/** Constructs the header row configuration for this column. */
	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;

	/** Returns whether this column supports sorting. */
	virtual bool SupportsSorting() const override { return false; }

	/** Constructs the widget to display for a specific tree item in this column. */
	virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;


private:
	/** Pointer to the owning Scene Outliner instance. */
	class ISceneOutliner* SceneOutliner = nullptr;
};
