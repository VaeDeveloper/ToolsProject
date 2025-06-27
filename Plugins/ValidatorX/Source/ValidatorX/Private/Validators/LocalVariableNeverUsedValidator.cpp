// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/LocalVariableNeverUsedValidator.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_VariableGet.h"
#include "K2Node_LocalVariable.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphSchema_K2.h"
#include "Misc/DataValidation.h"
#include "BlueprintEditorModule.h"
#include "GraphEditor.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"
#include "BlueprintEditor.h"
#include "SMyBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"


ULocalVariableNeverUsedValidator::ULocalVariableNeverUsedValidator()
{
    SetValidationEnabled(true);
}

bool ULocalVariableNeverUsedValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) const
{
    return InAsset && InAsset->IsA<UBlueprint>();
}

bool ULocalVariableNeverUsedValidator::IsEnabled() const
{
    static const ULocalVariableNeverUsedValidator* CDO = GetDefault<ULocalVariableNeverUsedValidator>();
    return CDO->bIsEnabled && !bIsConfigDisabled;
}

EDataValidationResult ULocalVariableNeverUsedValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    bIsError = false;
  
    if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
    {
        const auto AllGraphs = Blueprint->FunctionGraphs;
    
        for(UEdGraph* Graph : AllGraphs)
        {
            UK2Node_FunctionEntry* EntryNode = nullptr;

            for(UEdGraphNode* Node : Graph->Nodes)
            {
                if(UK2Node_FunctionEntry* Entry = Cast<UK2Node_FunctionEntry>(Node))
                {
                    EntryNode = Entry;
                    break;
                }
            }
   
             if(!EntryNode)  continue;
     
             for(const FBPVariableDescription& LocalVar : EntryNode->LocalVariables)
             {
                 bool bUsed = false;

                 for(UEdGraphNode* Node : Graph->Nodes)
                 {
                     if(const UK2Node_VariableGet* VarGet = Cast<UK2Node_VariableGet>(Node))
                     {
                         if(VarGet->GetVarName() == LocalVar.VarName)
                         {
                             
                             bUsed = true;
                             break;
                         }
                     }
                     else if(const UK2Node_VariableSet* VarSet = Cast<UK2Node_VariableSet>(Node))
                     {
                         if(VarSet->GetVarName() == LocalVar.VarName)
                         {
                             
                             bUsed = true;
                             break;
                         }
                     }
                 }
     
                 if(!bUsed)
                 {
                     const FText MessageText = FText::Format(
                         INVTEXT("Local variable '{0}' in function '{1}' is never used."),
                         FText::FromName(LocalVar.VarName),
                         FText::FromString(Graph->GetName()));

                     const TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);
                     const FText JumpToVariableText = FText::Format(INVTEXT("Jump to variable  - '{0}'"), FText::FromName(LocalVar.VarName));
                     Message->AddToken(FActionToken::Create(JumpToVariableText, FText::FromString(""), FSimpleDelegate::CreateLambda([=]
                         {
                             if(Blueprint && EntryNode)
                             {
                                 UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
                                 AssetEditorSubsystem->OpenEditorForAsset(Blueprint);
                                 if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
                                 {
                                     if(FBlueprintEditor* BlueprintEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
                                     {
                                         if (TSharedPtr<SGraphEditor> GraphEditor = BlueprintEditor->OpenGraphAndBringToFront(Graph, true))
                                         {
                                             if(TSharedPtr<SMyBlueprint> MyBlueprintWidget = BlueprintEditor->GetMyBlueprintWidget())
                                             {
                                                 MyBlueprintWidget->SelectItemByName(LocalVar.VarName,
                                                     ESelectInfo::Direct,
                                                     INDEX_NONE,
                                                     false);
                                             }
                                         }
                                     }
                                 }
                             }
                         })));

                     const FText DeleteVariableText = FText::Format(INVTEXT("'Fix' - Delete Local Variable - '{0}'"), FText::FromName(LocalVar.VarName));
                     Message->AddToken(FActionToken::Create(DeleteVariableText, FText::FromString(""),
                         FSimpleDelegate::CreateLambda([=]
                             {
                                 if (Blueprint && EntryNode)
                                 {
                                     UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
                                     AssetEditorSubsystem->OpenEditorForAsset(Blueprint);
                                     FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=] (float DeltaTime)
                                         {
                                             if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, /*bFocusIfOpen=*/false))
                                             {
                                                 if(FBlueprintEditor* BlueprintEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
                                                 {
                                                     if(BlueprintEditor->GetMyBlueprintWidget().IsValid())
                                                     {
                                                         int32 IndexToRemove = INDEX_NONE;
                                                         for(int32 i = 0; i < EntryNode->LocalVariables.Num(); ++i)
                                                         {
                                                             if(EntryNode->LocalVariables[i].VarName == LocalVar.VarName)
                                                             {
                                                                 IndexToRemove = i;
                                                                 break;
                                                             }
                                                         }

                                                         if(IndexToRemove != INDEX_NONE)
                                                         {
                                                             const FText ConfirmText = FText::Format(
                                                                 INVTEXT("Are you sure you want to delete the dispatcher '{0}' from Blueprint '{1}'?"),
                                                                 FText::FromName(LocalVar.VarName),
                                                                 FText::FromString(Blueprint->GetName())
                                                             );

                                                             if(FMessageDialog::Open(EAppMsgType::YesNo, ConfirmText) == EAppReturnType::Yes)
                                                             {
                                                                 EntryNode->Modify();
                                                                 EntryNode->LocalVariables.RemoveAt(IndexToRemove);
                                                                 FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
                                                             }
                                                         }

                                                         return false;
                                                     }
                                                 }
                                             }

                                             return true;
                                         }));
                                 }
                             })));


                     bIsError = true;
                 }
             }
        }
    }
   
    return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
