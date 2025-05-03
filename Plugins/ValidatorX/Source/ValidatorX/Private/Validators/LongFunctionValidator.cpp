// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/LongFunctionValidator.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "BlueprintEditorModule.h"
#include "Misc/DataValidation.h"


/* clang-format off */

namespace ValidatorX
{
    FString GetGraphType(UBlueprint* Blueprint, UEdGraph* Graph)
    {
        if(Blueprint->FunctionGraphs.Contains(Graph)) return TEXT("Function");
        if(Blueprint->MacroGraphs.Contains(Graph)) return TEXT("Macro");
        if(Blueprint->UbergraphPages.Contains(Graph)) return TEXT("Event Graph");
        if(Blueprint->DelegateSignatureGraphs.Contains(Graph)) return TEXT("Delegate");
        if(Blueprint->IntermediateGeneratedGraphs.Contains(Graph)) return TEXT("Intermediate");

        return TEXT("Unknown");
    }
}

ULongFunctionValidator::ULongFunctionValidator()
{
    SetValidationEnabled(true);
}

bool ULongFunctionValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UBlueprint>();
}

bool ULongFunctionValidator::IsEnabled() const
{
    static const ULongFunctionValidator* CDO = GetDefault<ULongFunctionValidator>();
    return CDO->bIsEnabled && !bIsConfigDisabled;
}

EDataValidationResult ULongFunctionValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    constexpr int32 NodeLimit = 50;
    bIsError = false;

    if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
    {

        TArray<UEdGraph*> AllGraphs = Blueprint->UbergraphPages;
        AllGraphs.Append(Blueprint->FunctionGraphs);
        AllGraphs.Append(Blueprint->MacroGraphs);
        AllGraphs.Append(Blueprint->DelegateSignatureGraphs);
        AllGraphs.Append(Blueprint->IntermediateGeneratedGraphs);

        for(UEdGraph* Graph : AllGraphs)
        {
            if(!Graph) continue;

            int32 NodeCount = 0;
            for(UEdGraphNode* Node : Graph->Nodes)
            {
                if(Node && !Node->IsA<UK2Node_FunctionEntry>() && !Node->IsA<UK2Node_FunctionResult>())
                {
                    NodeCount++;
                }
            }

            

            if(NodeCount > NodeLimit)
            {
                const FString GraphType = ValidatorX::GetGraphType(Blueprint, Graph);
                const FText MessageText = FText::Format(
                    INVTEXT("'{0}' - '{1}' contains {2} nodes, which exceeds the recommended limit of {3}. Consider splitting it into smaller functions."),
                    FText::FromString(GraphType),
                    FText::FromString(Graph->GetName()),
                    FText::AsNumber(NodeCount),
                    FText::AsNumber(NodeLimit)
                );
                const FText JumpText = FText::Format(INVTEXT("Jump to '{0}' - {1}"), FText::FromString(Graph->GetName()), FText::FromString(GraphType));

                TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);
                Message->AddToken(FActionToken::Create(
                    JumpText,
                    FText::FromString(""),
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
                                            BlueprintEditor->OpenGraphAndBringToFront(Graph, true);
                                        }
                                    }
                                }
                            }
                        })));
                bIsError = true;
			}
		}
	}

    return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
