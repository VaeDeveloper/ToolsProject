// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/UnusedFunctionValidator.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditor.h"
#include "Misc/DataValidation.h"
#include "SMyBlueprint.h"
UUnusedFunctionValidator::UUnusedFunctionValidator()
{
    SetValidationEnabled(true);
}

bool UUnusedFunctionValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UBlueprint>();
}

bool UUnusedFunctionValidator::IsEnabled() const
{
    static const UUnusedFunctionValidator* CDO = GetDefault<UUnusedFunctionValidator>();
    return CDO->bIsEnabled && !bIsConfigDisabled;
}

EDataValidationResult UUnusedFunctionValidator::ValidateLoadedAsset_Implementation(
    const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    bIsError = false;

    if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
    {
        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(Blueprint->UbergraphPages);
        AllGraphs.Append(Blueprint->FunctionGraphs);
        AllGraphs.Append(Blueprint->MacroGraphs);
        AllGraphs.Append(Blueprint->DelegateSignatureGraphs);
        AllGraphs.Append(Blueprint->IntermediateGeneratedGraphs);

        for(UEdGraph* FunctionGraph : Blueprint->FunctionGraphs)
        {
            
            if(!FunctionGraph) continue;


            const FName FunctionName = FunctionGraph->GetFName();
            bool bIsFunctionUsed = false;

            if(FunctionName == UEdGraphSchema_K2::FN_UserConstructionScript)
            {
                continue;
            }

            for(UEdGraph* Graph : AllGraphs)
            {
                if(!Graph || Graph == FunctionGraph) continue;

                for(UEdGraphNode* Node : Graph->Nodes)
                {
                    if(const UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(Node))
                    {
                        if(CallFunctionNode->FunctionReference.GetMemberName() == FunctionName)
                        {
                            bIsFunctionUsed = true;
                            break;
                        }
                    }
                }

                if(bIsFunctionUsed)
                {
                    break;
                }
            }

            if(!bIsFunctionUsed)
            {
                const FText MessageText = FText::Format(
                    INVTEXT("Function '{0}' in Blueprint '{1}' is never used."),
                    FText::FromName(FunctionName),
                    FText::FromString(Blueprint->GetName())
                );

                const TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);

                const FText JumpToFunctionText = FText::Format(
                    INVTEXT("Jump to Function - '{0}'"),
                    FText::FromName(FunctionName));

                Message->AddToken(FActionToken::Create(JumpToFunctionText, FText::GetEmpty(),
                    FSimpleDelegate::CreateLambda([=]
                        {
                            if(Blueprint && FunctionGraph)
                            {
                                if(UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                                {
                                    AssetEditorSubsystem->OpenEditorForAsset(Blueprint);
                                    if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
                                    {
                                        if(FBlueprintEditor* BlueprintEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
                                        {
                                            BlueprintEditor->OpenGraphAndBringToFront(FunctionGraph, true);

                                            if(TSharedPtr<SMyBlueprint> MyBlueprintWidget = BlueprintEditor->GetMyBlueprintWidget())
                                            {
                                                MyBlueprintWidget->SelectItemByName(FunctionGraph->GetFName(),
                                                    ESelectInfo::Direct,
                                                    INDEX_NONE,
                                                    false);
                                            }
                                        }
                                    }
                                }
                            }
                        })));

                const FText DeleteFunctionText = FText::Format(
                    INVTEXT("'Fix' - Delete Function - '{0}'"),
                    FText::FromName(FunctionName));

                Message->AddToken(FActionToken::Create(DeleteFunctionText, FText::GetEmpty(),
                    FSimpleDelegate::CreateLambda([=]
                        {
                            if(Blueprint && FunctionGraph)
                            {
                                if(UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                                {
                                    AssetEditorSubsystem->OpenEditorForAsset(Blueprint);

                                    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=] (float DeltaTime)
                                        {
                                            if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
                                            {
                                                if(FBlueprintEditor* BlueprintEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
                                                {
                                                    const FText ConfirmText = FText::Format(
                                                        INVTEXT("Are you sure you want to delete the unused Function '{0}' from Blueprint '{1}'?"),
                                                        FText::FromName(FunctionName),
                                                        FText::FromString(Blueprint->GetName())
                                                    );

                                                    if(FMessageDialog::Open(EAppMsgType::YesNo, ConfirmText) == EAppReturnType::Yes)
                                                    {
                                                        Blueprint->Modify();

                                                        Blueprint->FunctionGraphs.Remove(FunctionGraph);
                                                        FunctionGraph->Modify();
                                                        FunctionGraph->MarkAsGarbage();

                                                        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
                                                    }
                                                }
                                            }
                                            return false;
                                        }));
                                }
                            }
                        })));

                bIsError = true;
            }
        }
    }

    return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
