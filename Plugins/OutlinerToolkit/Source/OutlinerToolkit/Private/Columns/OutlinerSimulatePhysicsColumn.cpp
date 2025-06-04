// Fill out your copyright notice in the Description page of Project Settings.


#include "Columns/OutlinerSimulatePhysicsColumn.h"
#include "ComponentTreeItem.h"
#include "ActorTreeItem.h"
#include "SceneOutliner.h"
#include "Engine/StaticMeshActor.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/CoreStyle.h"

DEFINE_LOG_CATEGORY_STATIC(OutlinerSimulatePhysicsColumnLog, All, All);

FOutlinerSimulatePhysicsColumn::FOutlinerSimulatePhysicsColumn(ISceneOutliner& SceneOutliner)
{
	this->SceneOutliner = &SceneOutliner;
}

SHeaderRow::FColumn::FArguments FOutlinerSimulatePhysicsColumn::ConstructHeaderRowColumn()
{
	return SHeaderRow::Column(GetColumnID())
		.FixedWidth(30.0f)
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultLabel(FText::FromString("Physics"))
		.HeaderContent()
		[
			//TODO !!! use SImage in future 
			SNew(STextBlock)
				//.Image(FSlateBrush())
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Text(FText::FromString("PS"))
		];
}

const TSharedRef<SWidget> FOutlinerSimulatePhysicsColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	const FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();
	if(!ActorTreeItem || !ActorTreeItem->IsValid())
	{
		return SNullWidget::NullWidget;
	}

	AActor* Actor = ActorTreeItem->Actor.Get();
	if(!Actor)
	{
		return SNullWidget::NullWidget;
	}

	UPrimitiveComponent* Primitive = Actor->FindComponentByClass<UPrimitiveComponent>();
	if(!Primitive)
	{
		return SNullWidget::NullWidget;
	}

	bool bIsStaticMeshActor = Actor->IsA<AStaticMeshActor>();

	bool bIsSkySphere = false;
	if(const UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Primitive))
	{
		const UStaticMesh* Mesh = StaticMeshComp->GetStaticMesh();
		if(Mesh && Mesh->GetName().Contains(TEXT("SM_SkySphere")))
		{
			bIsSkySphere = true;
		}
	}

	const bool bIsEnabled = bIsStaticMeshActor && !bIsSkySphere;
	TWeakObjectPtr<UPrimitiveComponent> WeakPrimitive = Primitive;

	TSharedRef<SCheckBox> CheckBox = 
		SNew(SCheckBox)
		.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("PinnedCommandList.CheckBox"))
		.IsChecked_Lambda([WeakPrimitive, bIsEnabled] 
			{
				if(!bIsEnabled)
				{
					return ECheckBoxState::Undetermined;
				}

				if(WeakPrimitive.IsValid())
				{
					return WeakPrimitive->IsSimulatingPhysics() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				}

				return ECheckBoxState::Unchecked;
			})
		.OnCheckStateChanged_Lambda([WeakPrimitive, bIsEnabled, this] (ECheckBoxState NewState)
			{
				if(!bIsEnabled)
				{
					return;
				}

				if(!WeakPrimitive.IsValid())
				{
					UE_LOG(OutlinerSimulatePhysicsColumnLog, Warning, TEXT("Primitive component is invalid in OnCheckStateChanged"));
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
			})
		.IsEnabled(bIsEnabled)
		.ToolTipText_Lambda([bIsStaticMeshActor, bIsSkySphere] 
			{
				if(!bIsStaticMeshActor)
				{
					return FText::FromString("Only StaticMeshActor supports physics simulation.");
				}
				if(bIsSkySphere)
				{
					return FText::FromString("SkySphere actors cannot simulate physics.");
				}
				return FText::FromString("Toggle physics simulation.");
			});

	return CheckBox;
}

