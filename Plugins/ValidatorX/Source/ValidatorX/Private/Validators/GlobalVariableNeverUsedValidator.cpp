// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/GlobalVariableNeverUsedValidator.h"
#include "K2Node_VariableSet.h"
#include "K2Node_VariableGet.h"
#include "Misc/DataValidation.h"
#include "BlueprintEditor.h"
#include "SMyBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"

UGlobalVariableNeverUsedValidator::UGlobalVariableNeverUsedValidator()
{
	SetValidationEnabled(true);
}

bool UGlobalVariableNeverUsedValidator::IsEnabled() const
{
	static const UGlobalVariableNeverUsedValidator* CDO = GetDefault<UGlobalVariableNeverUsedValidator>();
	return CDO->bIsEnabled && !bIsConfigDisabled;
}

bool UGlobalVariableNeverUsedValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UBlueprint>();
}

EDataValidationResult UGlobalVariableNeverUsedValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    bIsError = false;

    if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
    {
        const TArray<FBPVariableDescription>& Variables = Blueprint->NewVariables;

        TArray<UEdGraph*> AllGraphs;
        Blueprint->GetAllGraphs(AllGraphs);

        for(const FBPVariableDescription& VarDesc : Variables)
        {
            bool bUsed = false;

            for(UEdGraph* Graph : AllGraphs)
            {
                if(!Graph) continue;

                for(UEdGraphNode* Node : Graph->Nodes)
                {
                    if(const UK2Node_VariableGet* VarGet = Cast<UK2Node_VariableGet>(Node))
                    {
                        if(VarGet->GetVarName() == VarDesc.VarName)
                        {
                            bUsed = true;
                            break;
                        }
                    }
                    else if(const UK2Node_VariableSet* VarSet = Cast<UK2Node_VariableSet>(Node))
                    {
                        if(VarSet->GetVarName() == VarDesc.VarName)
                        {
                            bUsed = true;
                            break;
                        }
                    }
                }

                if(bUsed) break;
            }

            if(!bUsed)
            {
                const FText MessageText = FText::Format(
                    INVTEXT("Variable '{0}' in Blueprint '{1}' is never used."),
                    FText::FromName(VarDesc.VarName),
                    FText::FromString(Blueprint->GetName())
                );

                const TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);

                // Jump to variable
                Message->AddToken(FActionToken::Create(
                    FText::Format(INVTEXT("Jump to Variable - '{0}'"), FText::FromName(VarDesc.VarName)),
                    FText::GetEmpty(),
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
                                            MyBlueprintWidget->SelectItemByName(VarDesc.VarName, ESelectInfo::Direct, INDEX_NONE, false);
                                        }
                                    }
                                }
                            }
                        })));

                // Fix - delete variable
                Message->AddToken(FActionToken::Create(
                    FText::Format(INVTEXT("'Fix' - Delete Variable - '{0}'"), FText::FromName(VarDesc.VarName)),
                    FText::GetEmpty(),
                    FSimpleDelegate::CreateLambda([=]
                        {
                            if(Blueprint)
                            {
                                UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
                                AssetEditorSubsystem->OpenEditorForAsset(Blueprint);

                                FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=] (float DeltaTime)
                                    {
                                        if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
                                        {
                                            if(FBlueprintEditor* BlueprintEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
                                            {
                                                const FText ConfirmText = FText::Format(
                                                    INVTEXT("Are you sure you want to delete variable '{0}' from Blueprint '{1}'?"),
                                                    FText::FromName(VarDesc.VarName),
                                                    FText::FromString(Blueprint->GetName())
                                                );

                                                if(FMessageDialog::Open(EAppMsgType::YesNo, ConfirmText) == EAppReturnType::Yes)
                                                {
                                                    FBlueprintEditorUtils::RemoveMemberVariable(Blueprint, VarDesc.VarName);
                                                    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
                                                }
                                            }
                                        }

                                        return false;
                                    }));
                            }
                        })));

                bIsError = true;
            }
        }
    }

    return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
