// Copyright Epic Games, Inc. All Rights Reserved.

#include "OutlinerToolkit.h"
#include "LevelEditor.h"
#include "Selection.h"
#include "Elements/Framework/TypedElementRegistry.h"
#include "Elements/Actor/ActorElementData.h"
#include "ToolMenus.h"
#include "Elements/Actor/ActorElementEditorSelectionInterface.h"
#include "Elements/Framework/TypedElementSelectionSet.h"
#include "Editor.h"
#define LOCTEXT_NAMESPACE "FOutlinerToolkitModule"

void FOutlinerToolkitModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOutlinerToolkitModule::RegisterMenus));
}

void FOutlinerToolkitModule::RegisterMenus()
{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
    FToolMenuSection& Section = Menu->FindOrAddSection("YourCustomSection");
    Section.AddMenuEntry(
        "CreateBlueprintEntry", 
        LOCTEXT("CreateBlueprintLabel", "Create Blueprint"),  
        LOCTEXT("CreateBlueprintTooltip", "Create a Blueprint from the selected Actor"),  
        FSlateIcon(),
        FToolMenuExecuteAction::CreateRaw(this, &FOutlinerToolkitModule::EntryFunctionWithContext)
    );
}

void FOutlinerToolkitModule::EntryFunctionWithContext(const FToolMenuContext& MenuContext)
{
   {
       // Альтернативно — через GEditor
       USelection* EngineSelection = GEditor->GetSelectedActors();
       for(FSelectionIterator It(*EngineSelection); It; ++It)
       {
           if(AActor* Actor = Cast<AActor>(*It))
           {
               UE_LOG(LogTemp, Log, TEXT("Selected Actor (Fallback): %s"), *Actor->GetName());
           }
       }
   }
}

void FOutlinerToolkitModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOutlinerToolkitModule, OutlinerToolkit)