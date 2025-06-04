// Fill out your copyright notice in the Description page of Project Settings.


#include "Columns/OutlinerHiddenInGameColumn.h"
#include "ActorTreeItem.h"
#include "SceneOutliner.h"
#include "Components/SceneComponent.h"
#include "Widgets/Input/SCheckBox.h"
#include "Styling/CoreStyle.h"

FOutlinerHiddenInGameColumn::FOutlinerHiddenInGameColumn(ISceneOutliner& InOutliner)
{
	SceneOutliner = &InOutliner;
}

SHeaderRow::FColumn::FArguments FOutlinerHiddenInGameColumn::ConstructHeaderRowColumn()
{
	return SHeaderRow::Column(GetColumnID())
		.FixedWidth(30.0f)
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultLabel(FText::FromString("Hidden In Game"))
		[
			//TODO !!! use SImage in future 
			SNew(STextBlock)
				//.Image(FSlateBrush())
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Text(FText::FromString("HiG"))
		];
}

const TSharedRef<SWidget> FOutlinerHiddenInGameColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	const FActorTreeItem* ActorItem = TreeItem->CastTo<FActorTreeItem>();
	if(!ActorItem || !ActorItem->IsValid())
	{
		return SNullWidget::NullWidget;
	}

	AActor* Actor = ActorItem->Actor.Get();
	if(!Actor)
	{
		return SNullWidget::NullWidget;
	}

	USceneComponent* RootComponent = Actor->GetRootComponent();
	if(!RootComponent)
	{
		return SNullWidget::NullWidget;
	}

	TWeakObjectPtr<USceneComponent> WeakComponent = RootComponent;

	return SNew(SCheckBox)
		.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("PinnedCommandList.CheckBox"))
		.IsChecked_Lambda([WeakComponent] ()
			{
				if(!WeakComponent.IsValid()) return ECheckBoxState::Undetermined;

				return WeakComponent->bHiddenInGame ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
		.OnCheckStateChanged_Lambda([WeakComponent, this] (ECheckBoxState NewState)
			{
				if(!WeakComponent.IsValid()) return;

				USceneComponent* Comp = WeakComponent.Get();
				Comp->Modify();
				Comp->SetHiddenInGame(NewState == ECheckBoxState::Checked);

				if(SceneOutliner)
				{
					SceneOutliner->FullRefresh();
				}
			})
		.ToolTipText(FText::FromString("Toggles whether the actor is hidden during gameplay."));
}

