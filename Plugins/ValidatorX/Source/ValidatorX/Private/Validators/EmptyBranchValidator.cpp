// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/EmptyBranchValidator.h"
#include "K2Node_IfThenElse.h"
#include "Misc/DataValidation.h"
#include "BlueprintEditorModule.h"

UEmptyBranchValidator::UEmptyBranchValidator()
{
    SetValidationEnabled(true);
}

bool UEmptyBranchValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
    return InAsset && InAsset->IsA<UBlueprint>();
}

EDataValidationResult UEmptyBranchValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    bIsError = false;

    if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
    {
        TArray<UEdGraph*> AllGraphs;
        Blueprint->GetAllGraphs(AllGraphs);

        for(UEdGraph* Graph : AllGraphs)
        {
            for(UEdGraphNode* Node : Graph->Nodes)
            {
                if(UK2Node_IfThenElse* Branch = Cast<UK2Node_IfThenElse>(Node))
                {
                    const UEdGraphPin* ThenPin = Branch->GetThenPin();
                    const UEdGraphPin* ElsePin = Branch->GetElsePin();

                    const bool bThenUnconnected = ThenPin && ThenPin->LinkedTo.Num() == 0;
                    const bool bElseUnconnected = ElsePin && ElsePin->LinkedTo.Num() == 0;

                    // Only if BOTH branches are not connected
                    if(bThenUnconnected && bElseUnconnected)
                    {
                        const FText MessageText = FText::Format(
                            INVTEXT("Branch node in graph '{0}' has both 'Then' and 'Else' execution pins unconnected."),
                            FText::FromString(Graph->GetName())
                        );

                        TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);
                        Message->AddToken(FActionToken::Create(FText::FromString("Jump to Branch"), FText::GetEmpty(),
                            FSimpleDelegate::CreateLambda([=]
                                {
                                    if(Blueprint && Graph)
                                    {
                                        if(UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                                        {
                                            AssetEditorSubsystem->OpenEditorForAsset(Blueprint);
                                            if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
                                            {
                                                if(IBlueprintEditor* BlueprintEditor = StaticCast<IBlueprintEditor*>(EditorInstance))
                                                {
                                                    if(TSharedPtr<SGraphEditor> GraphEditor = BlueprintEditor->OpenGraphAndBringToFront(Graph, true))
                                                    {
                                                        GraphEditor->JumpToNode(Branch, false);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }))
                        );

                        bIsError = true;
                    }
                }
            }
        }
    }

    return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

bool UEmptyBranchValidator::IsEnabled() const
{
    static const UEmptyBranchValidator* CDO = GetDefault<UEmptyBranchValidator>();
    return CDO->bIsEnabled && !bIsConfigDisabled;
}