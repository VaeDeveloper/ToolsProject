// Fill out your copyright notice in the Description page of Project Settings.


#include "Columns/OutlinerRTGroupIDColumn.h"
#include "ISceneOutlinerTreeItem.h"
#include "ComponentTreeItem.h"
#include "ActorTreeItem.h"
#include "SceneOutliner.h"
#include "Engine/StaticMeshActor.h"
#include "Components/TextRenderComponent.h"
#include "Components/BrushComponent.h"
#include "GameFramework/PlayerStart.h"

FOutlinerRTGroupIDColumn::FOutlinerRTGroupIDColumn(ISceneOutliner& SceneOutliner)
{
	this->SceneOutliner = &SceneOutliner;
	CurrentSortMode = EColumnSortMode::None;
}

SHeaderRow::FColumn::FArguments FOutlinerRTGroupIDColumn::ConstructHeaderRowColumn()
{
	return SHeaderRow::Column(GetColumnID())
		//.FixedWidth(50.0f)
		.HAlignHeader(HAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.VAlignHeader(VAlign_Center)
		.SortMode(TAttribute<EColumnSortMode::Type>::Create(TAttribute<EColumnSortMode::Type>::FGetter::CreateSP(this, &FOutlinerRTGroupIDColumn::GetColumnSortMode)))
		.OnSort(this, &FOutlinerRTGroupIDColumn::OnColumnSortModeChanged)
		.DefaultLabel(FText::FromString("RayTracingGroupId"))
		.HeaderContent()
		[
			SNew(STextBlock)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Text(FText::FromString("ID"))
		];
}

const TSharedRef<SWidget> FOutlinerRTGroupIDColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	return SNew(STextBlock)
		.Text_Lambda([WeakTreeItem = TWeakPtr<ISceneOutlinerTreeItem>(TreeItem), this] ()
			{
				if(TSharedPtr<ISceneOutlinerTreeItem> Pinned = WeakTreeItem.Pin())
				{
					return FText::FromString(GetRayTracingGroupIdString(Pinned.ToSharedRef()));
				}
				return FText::FromString(TEXT("-"));
			});
}

void FOutlinerRTGroupIDColumn::SortItems(TArray<FSceneOutlinerTreeItemPtr>& InOutItems, const EColumnSortMode::Type InSortMode) const
{
	if(InSortMode == EColumnSortMode::None)
	{
		return;
	}

	InOutItems.Sort([&, InSortMode] (const FSceneOutlinerTreeItemPtr& A, const FSceneOutlinerTreeItemPtr& B)
		{
			const FString AStr = GetRayTracingGroupIdString(A.ToSharedRef());
			const FString BStr = GetRayTracingGroupIdString(B.ToSharedRef());

			const int32 AInt = FCString::Atoi(*AStr);
			const int32 BInt = FCString::Atoi(*BStr);

			return (InSortMode == EColumnSortMode::Ascending) ? (AInt < BInt) : (AInt > BInt);
		});
}

FString FOutlinerRTGroupIDColumn::GetRayTracingGroupIdString(FSceneOutlinerTreeItemRef TreeItem) const
{
	if(!TreeItem->IsValid())
	{
		return TEXT("");
	}

	if(FActorTreeItem* ActorItem = TreeItem->CastTo<FActorTreeItem>())
	{
		if(AActor* Actor = ActorItem->Actor.Get())
		{

			if(Actor->IsA<APlayerStart>())
			{
				return TEXT("");
			}

			USceneComponent* RootComp = Actor->GetRootComponent();

			if(RootComp && !RootComp->IsA<UBrushComponent>() && !RootComp->IsA<UTextRenderComponent>())
			{
				if(UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(RootComp))
				{
					return FString::FromInt(PrimComp->GetRayTracingGroupId());
				}
			}
		}
	}

	return TEXT("");
}

EColumnSortMode::Type FOutlinerRTGroupIDColumn::GetColumnSortMode() const
{
	return CurrentSortMode;
}

void FOutlinerRTGroupIDColumn::OnColumnSortModeChanged(EColumnSortPriority::Type PriorityType, const FName& Name, EColumnSortMode::Type SortMode)
{
	CurrentSortMode = SortMode;

	if(SceneOutliner)
	{
		SceneOutliner->RequestSort();
		SceneOutliner->FullRefresh();
	}
}
