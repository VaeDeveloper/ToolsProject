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
#define LOCTEXT_NAMESPACE "FOutlinerToolkitModule"

void FOutlinerToolkitModule::StartupModule()
{
    FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([this] (UObject* Object, FPropertyChangedEvent& Event)
        {
            const FName PropertyName = Event.GetPropertyName();
            UE_LOG(LogTemp, Warning, TEXT("[OnPropertyChanged] Object: %s | Property: %s"),
                *GetNameSafe(Object), *PropertyName.ToString());

            //if(AActor* Actor = Cast<AActor>(Object))
            //{
            //
            //    UClass* ActorClass = Actor->GetClass();
            //    FProperty* Property = ActorClass->FindPropertyByName(TEXT("RayTracingGroupId"));
            //
            //    if(Property)
            //    {
            //        if(FIntProperty* IntProperty = CastField<FIntProperty>(Property))
            //        {
            //            int32 RayTracingGroupIdValue = 0;
            //            IntProperty->GetValue_InContainer(Actor, &RayTracingGroupIdValue);
            //
            //            UE_LOG(LogTemp, Warning, TEXT("Detected RayTracingGroupId change on Actor: %s, New Value: %d"),
            //                *Actor->GetName(), RayTracingGroupIdValue);
            //            RefreshSceneOutliner();
            //        }
            //    }
            //}


            if(UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Object))
            {
                if(PropertyName == GET_MEMBER_NAME_CHECKED(UPrimitiveComponent, RayTracingGroupId))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Detected RayTracingGroupId change on PrimitiveComponent: %s (Owner: %s)"),
                        *PrimComp->GetName(), *GetNameSafe(PrimComp->GetOwner()));
                    // Refresh Outliner
                    return;
                }
            }
        });

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOutlinerToolkitModule::RegisterMenus));
}

void FOutlinerToolkitModule::RegisterMenus()
{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
    FToolMenuSection& Section = Menu->FindOrAddSection("Toolset");
    Section.AddMenuEntry(
        "CreateBlueprintEntry", 
        LOCTEXT("CreateBlueprintLabel", "Create Blueprint"),  
        LOCTEXT("CreateBlueprintTooltip", "Create a Blueprint from the selected Actor"),  
        FSlateIcon(),
        FUIAction(FSimpleDelegate::CreateRaw(this, &FOutlinerToolkitModule::EntryFunctionWithContext))
    );
}

void FOutlinerToolkitModule::EntryFunctionWithContext()
{
    USelection* EngineSelection = GEditor->GetSelectedActors();
    if(!EngineSelection || EngineSelection->Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No actors selected."));
        return;
    }

    FString PackageName = TEXT("/Game/GeneratedBP");
    FString AssetName = TEXT("MyGeneratedBlueprint");

    FString FinalPackageName;
    FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
    AssetToolsModule.Get().CreateUniqueAssetName(PackageName, TEXT(""), FinalPackageName, AssetName);

    UPackage* Package = CreatePackage(*FinalPackageName);

    
    UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(AActor::StaticClass(), Package, FName(*AssetName), BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), FName("CreateBlueprintFromActors"));

    if(!Blueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Blueprint."));
        return;
    }

    for(FSelectionIterator It(*EngineSelection); It; ++It)
    {
        if(AActor* Actor = Cast<AActor>(*It))
        {
            TArray<UActorComponent*> Components = Actor->GetComponents().Array();
            for(UActorComponent* Component : Components)
            {
                if(USceneComponent* SceneComp = Cast<USceneComponent>(Component))
                {
                    FKismetEditorUtilities::AddComponentsToBlueprint(Blueprint, { SceneComp });
                }
            }
        }
    }

    FAssetRegistryModule::AssetCreated(Blueprint);
    Blueprint->MarkPackageDirty();

    FString PackageFileName = FPackageName::LongPackageNameToFilename(FinalPackageName, FPackageName::GetAssetPackageExtension());
    UPackage::SavePackage(Package, Blueprint, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);

    GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);
    UE_LOG(LogTemp, Log, TEXT("Blueprint '%s' created successfully."), *AssetName);

}

void FOutlinerToolkitModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOutlinerToolkitModule, OutlinerToolkit)