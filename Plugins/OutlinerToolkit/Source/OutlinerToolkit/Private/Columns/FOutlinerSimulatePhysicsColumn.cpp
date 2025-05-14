// Fill out your copyright notice in the Description page of Project Settings.


#include "Columns/FOutlinerSimulatePhysicsColumn.h"
#include "ComponentTreeItem.h"
#include "ActorTreeItem.h"
#include "SceneOutliner.h"
#include "Engine/StaticMeshActor.h"

FOutlinerSimulatePhysicsColumn::FOutlinerSimulatePhysicsColumn(ISceneOutliner& SceneOutliner)
{
	this->SceneOutliner = &SceneOutliner;
}

SHeaderRow::FColumn::FArguments FOutlinerSimulatePhysicsColumn::ConstructHeaderRowColumn()
{
	return SHeaderRow::Column(GetColumnID())
		.FixedWidth(50.0f)
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
	const FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();
	if(!ActorTreeItem || !ActorTreeItem->IsValid())
	{
		return SNullWidget::NullWidget;
	}

	const AActor* Actor = ActorTreeItem->Actor.Get();
	if(!Actor)
	{
		return SNullWidget::NullWidget;
	}

	if(!Actor->IsA<AStaticMeshActor>())
	{
		return SNullWidget::NullWidget;
	}

	UPrimitiveComponent* Primitive = Actor->FindComponentByClass<UPrimitiveComponent>();

	if(!Primitive)
	{
		return SNullWidget::NullWidget;
	}

	if(const UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Primitive))
	{
		const UStaticMesh* Mesh = StaticMeshComp->GetStaticMesh();
		if(Mesh && Mesh->GetName().Contains(TEXT("SM_SkySphere")))
		{
			return SNullWidget::NullWidget;
		}
	}

	TWeakObjectPtr<UPrimitiveComponent> WeakPrimitive = Primitive;

	TSharedRef<SCheckBox> CheckBox = SNew(SCheckBox)
		.IsChecked_Lambda([WeakPrimitive] ()
			{
				if(WeakPrimitive.IsValid())
				{
					return WeakPrimitive->IsSimulatingPhysics() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				}
				return ECheckBoxState::Unchecked;
			})
		.OnCheckStateChanged_Lambda([WeakPrimitive, this] (ECheckBoxState NewState)
			{
				if(!WeakPrimitive.IsValid())
				{
					UE_LOG(LogTemp, Warning, TEXT("Primitive component is invalid in OnCheckStateChanged"));
					return;
				}

				UPrimitiveComponent* Primitive = WeakPrimitive.Get();

				Primitive->SetMobility(EComponentMobility::Movable);
				Primitive->Modify();
				Primitive->SetSimulatePhysics(NewState == ECheckBoxState::Checked);

				if(SceneOutliner)
				{
					SceneOutliner->FullRefresh();
				}
			});

	return CheckBox;
}


