// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/UnboundEventDispatcherValidator.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_RemoveDelegate.h"
#include "K2Node_CallDelegate.h"
#include "K2Node_AssignDelegate.h"
#include "AssetRegistry/AssetData.h"
#include "Editor.h"
#include "EdGraph/EdGraph.h"
#include "K2Node.h"
#include "EdGraphNode_Comment.h"
#include "AssetToolsModule.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Misc/DataValidation.h"
#include "BlueprintEditorModule.h"
#include "BlueprintEditor.h"
#include "SMyBlueprint.h"

UUnboundEventDispatcherValidator::UUnboundEventDispatcherValidator()
{
    SetValidationEnabled(true);
}

bool UUnboundEventDispatcherValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
    return InAsset && InAsset->IsA<UBlueprint>();
}

bool UUnboundEventDispatcherValidator::IsEnabled() const
{
    static const UUnboundEventDispatcherValidator* CDO = GetDefault<UUnboundEventDispatcherValidator>();
    return CDO->bIsEnabled && !bIsConfigDisabled;
}

EDataValidationResult UUnboundEventDispatcherValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    bIsError = false;

    if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
    {
        TSet<FName> AllDispatchers;

        for(const FBPVariableDescription& Variable : Blueprint->NewVariables)
        {
            if(Variable.VarType.PinCategory == UEdGraphSchema_K2::PC_MCDelegate || Variable.VarType.PinCategory == UEdGraphSchema_K2::PC_Delegate)
            {
                AllDispatchers.Add(Variable.VarName);
            }
        }

        if(AllDispatchers.Num() == 0)
        {
            return EDataValidationResult::Valid;
        }

        TSet<FName> UsedDispatchers;


        auto GatherUsed = [&UsedDispatchers] (const UEdGraph* Graph)
            {
                for(UEdGraphNode* Node : Graph->Nodes)
                {
                    if(UK2Node* K2Node = Cast<UK2Node>(Node))
                    {
                        if(UK2Node_AddDelegate* Add = Cast<UK2Node_AddDelegate>(K2Node))
                        {
                            UsedDispatchers.Add(Add->GetPropertyName());
                        }
                        else if(UK2Node_RemoveDelegate* Remove = Cast<UK2Node_RemoveDelegate>(K2Node))
                        {
                            UsedDispatchers.Add(Remove->GetPropertyName());
                        }
                        else if(UK2Node_CallDelegate* Call = Cast<UK2Node_CallDelegate>(K2Node))
                        {
                            UsedDispatchers.Add(Call->GetPropertyName());
                        }
                        else if(UK2Node_AssignDelegate* Assign = Cast<UK2Node_AssignDelegate>(K2Node))
                        {
                            UsedDispatchers.Add(Assign->GetPropertyName());
                        }
                    }
                }
            };

        TArray<UEdGraph*> AllGraphs;
        Blueprint->GetAllGraphs(AllGraphs);
        
        for(UEdGraph* Graph : AllGraphs)
        {
            GatherUsed(Graph);
        }

        for(const FName& Dispatcher : AllDispatchers)
        {
            if(!UsedDispatchers.Contains(Dispatcher))
            {
                const FText MessageText = FText::Format(
                    INVTEXT("Event Dispatcher '{0}' is never bound, assigned or called in Blueprint '{1}'."),
                    FText::FromName(Dispatcher),
                    FText::FromString(Blueprint->GetName())
                );

                TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);
                FText JumpToDispatcherText = FText::Format(INVTEXT("Jump to Dispatcher - '{0}'    "), FText::FromName(Dispatcher));
                Message->AddToken(FActionToken::Create(JumpToDispatcherText, FText::FromString(""),
                    FSimpleDelegate::CreateLambda([=]
                        {
                            if(Blueprint)
                            {
                                UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
                                AssetEditorSubsystem->OpenEditorForAsset(Blueprint);
                                if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
                                {
                                    if(FBlueprintEditor* BlueprintEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
                                    {
                                        if(TSharedPtr<SMyBlueprint> MyBlueprintWidget = BlueprintEditor->GetMyBlueprintWidget())
                                        {
                                            MyBlueprintWidget->SelectItemByName(Dispatcher, 
                                                ESelectInfo::Direct, 
                                                INDEX_NONE, 
                                                false);
                                        }
                                    }
                                }
                            }                        
                        })
                ));

                FText DeleteDispatcherText = FText::Format(INVTEXT("'Fix' - Delete Dispatcher - '{0}'"), FText::FromName(Dispatcher));
                Message->AddToken(FActionToken::Create(DeleteDispatcherText, FText::FromString(""),
                    FSimpleDelegate::CreateLambda([=]
                        {
                            if(Blueprint)
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
                                                    for(int32 i = 0; i < Blueprint->NewVariables.Num(); ++i)
                                                    {
                                                        if(Blueprint->NewVariables[i].VarName == Dispatcher)
                                                        {
                                                            IndexToRemove = i;
                                                            break;
                                                        }
                                                    }

                                                    if(IndexToRemove != INDEX_NONE)
                                                    {
                                                        const FText ConfirmText = FText::Format(
                                                            INVTEXT("Are you sure you want to delete the dispatcher '{0}' from Blueprint '{1}'?"),
                                                            FText::FromName(Dispatcher),
                                                            FText::FromString(Blueprint->GetName())
                                                        );

                                                        if(FMessageDialog::Open(EAppMsgType::YesNo, ConfirmText) == EAppReturnType::Yes)
                                                        {
                                                            Blueprint->Modify();
                                                            Blueprint->NewVariables.RemoveAt(IndexToRemove);
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
                        })
                ));
                bIsError = true;
            }
        }
    }

    return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
