// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SStaticMeshOutlinerPanel.h"
#include "SceneOutlinerModule.h"
#include "SceneOutlinerPublicTypes.h"
#include "Widgets/Layout/SBox.h"
#include "Engine/StaticMeshActor.h"
#include "ISceneOutliner.h"
#include "ISceneOutlinerColumn.h"
#include "SceneOutlinerFilters.h"
#include "SSceneOutliner.h"
#include "ActorMode.h"
#include "SceneOutlinerFilters.h"
#include "ActorTreeItem.h"
#include "ISceneOutlinerTreeItem.h"

struct FActorFilter
{
	using FFilterPredicate = TDelegate<bool(const AActor*)>;
	//using FInteractivePredicate = TDelegate<bool(const AActor*)>;
};

void SStaticMeshOutlinerPanel::Construct(const FArguments& InArgs)
{
	FSceneOutlinerInitializationOptions InitOptions;
	InitOptions.bShowHeaderRow = true;
	InitOptions.bShowSearchBox = true;
	InitOptions.Filters = MakeShared<FSceneOutlinerFilters>();


	//InitOptions.Filters->AddFilterPredicate<FActorFilter>(
	//	FActorFilter::FFilterPredicate::CreateLambda([] (const AActor* InActor)
	//		{
	//			return InActor && InActor->IsA<AStaticMeshActor>();
	//		})
	//);

	//// 3. Оставляем ModeFactory стандартным
	//InitOptions.ModeFactory = FCreateSceneOutlinerMode::CreateLambda([] (ISceneOutliner& Outliner) {
	//	return MakeShared<FActorMode>(&Outliner); // или nullptr
	//	});

	//TSharedRef<SSceneOutliner> SceneOutliner = SNew(SSceneOutliner, InitOptions)
	//	.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute());

	//// 2. Подписываемся на выбор
	//SceneOutliner->GetTreeView()->SetOnSelectionChanged(
	//	SSceneOutliner::GetOnItemSelectionChanged::CreateLambda(
	//		[] (ISceneOutlinerTreeItemPtr PickedItem, ESelectInfo::Type)
	//		{
	//			if(FActorTreeItem* ActorItem = static_cast<FActorTreeItem*>(PickedItem.Get()))
	//			{
	//				if(ActorItem->IsValid())
	//				{
	//					if(AActor* Actor = ActorItem->Actor.Get())
	//					{
	//						UE_LOG(LogTemp, Log, TEXT("Selected: %s"), *Actor->GetName());
	//					}
	//				}
	//			}
	//		})
	//);

	//SceneOutlinerWidget = SceneOutliner;

	//ChildSlot
	//	[
	//		SceneOutlinerWidget.ToSharedRef()
	//	];
}
