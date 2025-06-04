// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISceneOutlinerColumn.h"

/**
 * 
 */
class OUTLINERTOOLKIT_API FOutlinerHiddenInGameColumn : public ISceneOutlinerColumn
{
public:
	FOutlinerHiddenInGameColumn(ISceneOutliner& SceneOutliner);

	virtual FName GetColumnID() override { return FName("HiddenInGame"); }
	static FName GetID() { return FName("HiddenInGame"); }

	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;
	virtual bool SupportsSorting() const override { return false; }
	virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;

private:
	ISceneOutliner* SceneOutliner;
};
