// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/EmptyFunctionValidator.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditor.h"
#include "Misc/DataValidation.h"

UEmptyFunctionValidator::UEmptyFunctionValidator()
{
	SetValidationEnabled(true);
}

bool UEmptyFunctionValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UBlueprint>();
}

bool UEmptyFunctionValidator::IsEnabled() const
{
	static const UEmptyFunctionValidator* CDO = GetDefault<UEmptyFunctionValidator>();
	return CDO->bIsEnabled && !bIsConfigDisabled;
}

EDataValidationResult UEmptyFunctionValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	bIsError = false;

	if (UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
	{
		for (UEdGraph* FunctionGraph : Blueprint->FunctionGraphs)
		{
			if(!FunctionGraph) continue;

			if(FunctionGraph->GetFName() == UEdGraphSchema_K2::FN_UserConstructionScript) continue;

			int32 UsefulNodeCount = 0;
			for (UEdGraphNode* Node : FunctionGraph->Nodes)
			{
				if(!Node) continue;

				if (Node->IsA<UK2Node_FunctionEntry>() || Node->IsA<UK2Node_FunctionResult>())
				{
					continue;
				}

				UsefulNodeCount++;
			}

			if (UsefulNodeCount == 0)
			{
				const FText MessageText = FText::Format(
					INVTEXT("Function '{0}' in Blueprint '{1}' is empty."),
					FText::FromString(FunctionGraph->GetName()),
					FText::FromString(Blueprint->GetName()));

				const TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);

				const FText JumpToFunctionText = FText::Format(
					INVTEXT("Jump to Function - '{0}'"),
					FText::FromString(FunctionGraph->GetName()));

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
										}
									}
								}
							}
						})));

				const FText DeleteFunctionText = FText::Format(
					INVTEXT("'Fix' - Delete Function - '{0}'"),
					FText::FromString(FunctionGraph->GetName()));

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
											if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, /*bFocusIfOpen=*/false))
											{
												if(FBlueprintEditor* BlueprintEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
												{
													const FText ConfirmText = FText::Format(
														INVTEXT("Are you sure you want to delete the Function '{0}' from Blueprint '{1}'?"),
														FText::FromString(FunctionGraph->GetName()),
														FText::FromString(Blueprint->GetName())
													);

													if(FMessageDialog::Open(EAppMsgType::YesNo, ConfirmText) == EAppReturnType::Yes)
													{
														Blueprint->Modify();
														FunctionGraph->Modify();

														Blueprint->FunctionGraphs.Remove(FunctionGraph);
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


