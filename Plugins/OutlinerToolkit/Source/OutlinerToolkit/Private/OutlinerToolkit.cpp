// Copyright Epic Games, Inc. All Rights Reserved.

#include "OutlinerToolkit.h"
#include "LevelEditor.h"
#include "Selection.h"
#include "Kismet2/KismetEditorUtilities.h"  
#include "Engine/Blueprint.h"                   
#include "Kismet/BlueprintFunctionLibrary.h"   
#include "AssetToolsModule.h"
#include "GameFramework/Actor.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SimpleConstructionScript.h"
#include "SceneOutlinerModule.h"
#include "Columns/OutlinerSimulatePhysicsColumn.h"
#include "Columns/OutlinerMobilityColumn.h"
#include "Columns/OutlinerHiddenInGameColumn.h"
#include "EditorActorFolders.h"
#include "ActorGroupingUtils.h"
#include "Editor/GroupActor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FOutlinerToolkitModule"

void FOutlinerToolkitModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOutlinerToolkitModule::RegisterMenus));
	InitCustomSceneOutlinerColumn();
}

void FOutlinerToolkitModule::RegisterMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
	FToolMenuSection& Section = Menu->FindOrAddSection("Toolset");

	Section.AddSubMenu(
		"MySubMenu",
		LOCTEXT("OutlinerToolkitLabel", "Outliner Toolkit"),
		LOCTEXT("OutlinerToolkitTooltip", "Custom tools for selected actors"),
		FNewToolMenuDelegate::CreateLambda([this] (UToolMenu* SubMenu)
			{
				FToolMenuSection& SubSection = SubMenu->AddSection("MyToolsSection");

				SubSection.AddMenuEntry(
					"MySubAction",
					LOCTEXT("CreateBlueprintLabel", "Group Actors"),
					LOCTEXT("CreateBlueprintTooltip", ""),
					FSlateIcon(),
					FUIAction(FSimpleDelegate::CreateRaw(this, &FOutlinerToolkitModule::EntryFunctionWithContext))
				);
			})
	);
}
// TODO !!! Test (create blueprint for outliner actors 
void FOutlinerToolkitModule::EntryFunctionWithContext()
{
	TArray<AActor*> SelectedActors;
	for(FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
	{
		if(AActor* Actor = Cast<AActor>(*It))
		{
			SelectedActors.Add(Actor);
		}
	}

	if(SelectedActors.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Need at least 2 actors to group."));
		return;
	}

	UActorGroupingUtils* GroupingUtils = NewObject<UActorGroupingUtils>();
	AGroupActor* GroupActor = GroupingUtils->GroupActors(SelectedActors);

	if(!GroupActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to group actors."));
		return;
	}

	UWorld* World = GroupActor->GetWorld();
	if(!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid world for grouped actor."));
		return;
	}

	FName BaseFolderName = TEXT("Group");
	FName FinalFolderName = BaseFolderName;
	int32 Index = 1;

	while(FActorFolders::Get().ContainsFolder(*World, FFolder(FFolder::GetInvalidRootObject(), FinalFolderName)))
	{
		FinalFolderName = FName(*FString::Printf(TEXT("%s_%d"), *BaseFolderName.ToString(), Index++));
	}

	const FFolder NewFolder(FFolder::GetInvalidRootObject(), FinalFolderName);
	FActorFolders::Get().CreateFolder(*World, NewFolder);

	GroupActor->Modify();
	GroupActor->SetFolderPath(FinalFolderName);

	TArray<AActor*> GroupedActors;
	GroupActor->GetGroupActors(GroupedActors);

	for(AActor* ActorInGroup : GroupedActors)
	{
		if(IsValid(ActorInGroup))
		{
			ActorInGroup->Modify();
			ActorInGroup->SetFolderPath(FinalFolderName);
		}
	}

	FActorFolders::Get().SetSelectedFolderPath(NewFolder);
	GEditor->NoteSelectionChange();
	UE_LOG(LogTemp, Log, TEXT("Group and its members moved to folder '%s'."), *FinalFolderName.ToString());

	FNotificationInfo Info(FText::Format(
		NSLOCTEXT("OutlinerToolkit", "FolderCreated", "Folder '{0}' created. Press F2 to rename."),
		FText::FromName(FinalFolderName)
	));
	Info.ExpireDuration = 5.0f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.DefaultMessage"));

	FSlateNotificationManager::Get().AddNotification(Info);
}

void FOutlinerToolkitModule::InitCustomSceneOutlinerColumn()
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");

	// HiddenInGame Column
	FSceneOutlinerColumnInfo HiddenInGameColumnInfo
	(
		ESceneOutlinerColumnVisibility::Visible,
		2,
		FCreateSceneOutlinerColumn::CreateRaw(this, &FOutlinerToolkitModule::OnCreateHiddenInGame)
	);
	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerHiddenInGameColumn>(HiddenInGameColumnInfo);
	
	// Physics Column
	FSceneOutlinerColumnInfo SimulatePhysicsColumnInfo
	(
		ESceneOutlinerColumnVisibility::Visible,
		3,
		FCreateSceneOutlinerColumn::CreateRaw(this, &FOutlinerToolkitModule::OnCreateSimulatePhysics)
	);
	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerSimulatePhysicsColumn>(SimulatePhysicsColumnInfo);

	// Mobility Column
	FSceneOutlinerColumnInfo MobilityColumnInfo
	(
		ESceneOutlinerColumnVisibility::Visible,
		4,
		FCreateSceneOutlinerColumn::CreateRaw(this, &FOutlinerToolkitModule::OnCreateMobility)
	);
	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerMobilityColumn>(MobilityColumnInfo);

	//TODO more column
}

TSharedRef<ISceneOutlinerColumn> FOutlinerToolkitModule::OnCreateSimulatePhysics(ISceneOutliner& SceneOutliner)
{
	return MakeShareable(new FOutlinerSimulatePhysicsColumn(SceneOutliner));
}

TSharedRef<ISceneOutlinerColumn> FOutlinerToolkitModule::OnCreateMobility(ISceneOutliner& SceneOutliner)
{
	return MakeShareable(new FOutlinerMobilityColumn(SceneOutliner));
}

TSharedRef<ISceneOutlinerColumn> FOutlinerToolkitModule::OnCreateHiddenInGame(ISceneOutliner& SceneOutliner)
{
	return MakeShareable(new FOutlinerHiddenInGameColumn(SceneOutliner));
}

void FOutlinerToolkitModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOutlinerToolkitModule, OutlinerToolkit)