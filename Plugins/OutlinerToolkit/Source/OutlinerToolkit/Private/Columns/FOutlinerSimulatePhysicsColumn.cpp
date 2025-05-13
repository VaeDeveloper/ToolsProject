// Fill out your copyright notice in the Description page of Project Settings.


#include "Columns/FOutlinerSimulatePhysicsColumn.h"
#include "ComponentTreeItem.h"
#include "ActorTreeItem.h"
#include "SceneOutliner.h"

FOutlinerSimulatePhysicsColumn::FOutlinerSimulatePhysicsColumn(ISceneOutliner& SceneOutliner)
{
	this->SceneOutliner = &SceneOutliner;
}

SHeaderRow::FColumn::FArguments FOutlinerSimulatePhysicsColumn::ConstructHeaderRowColumn()
{
	return SHeaderRow::Column(GetColumnID())
		//.FixedWidth(50.0f)
		.HAlignHeader(HAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.VAlignHeader(VAlign_Center)
		.DefaultLabel(FText::FromString("Physics"))
		.HeaderContent()
		[
			SNew(STextBlock)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Text(FText::FromString("Physics"))
		];
}

const TSharedRef<SWidget> FOutlinerSimulatePhysicsColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	//FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();
	//if(!ActorTreeItem || !ActorTreeItem->IsValid())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Invalid ActorTreeItem"));
	//	return SNullWidget::NullWidget;
	//}

	//AActor* Actor = ActorTreeItem->Actor.Get();
	//if(!Actor)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Actor is null"));
	//	return SNullWidget::NullWidget;
	//}

	//UPrimitiveComponent* Primitive = Actor->FindComponentByClass<UPrimitiveComponent>();
	//if(!Primitive)
	//{
	//	return SNullWidget::NullWidget;
	//}

	//TSharedRef<SCheckBox> CheckBox = SNew(SCheckBox)
	//	.IsChecked_Lambda([Primitive] ()
	//		{
	//			return Primitive->IsSimulatingPhysics() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	//		})
	//	.OnCheckStateChanged_Lambda([Primitive, this] (ECheckBoxState NewState)
	//		{
	//			if(!Primitive)
	//			{
	//				UE_LOG(LogTemp, Warning, TEXT("Primitive component is null in OnCheckStateChanged"));
	//				return;
	//			}

	//			Primitive->Modify();
	//			Primitive->SetSimulatePhysics(NewState == ECheckBoxState::Checked);

	//			if(SceneOutliner)
	//			{
	//				SceneOutliner->FullRefresh();
	//			}
	//		});

	return SNullWidget::NullWidget;
}
