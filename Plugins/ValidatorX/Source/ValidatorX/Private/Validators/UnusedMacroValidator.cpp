// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/UnusedMacroValidator.h"
#include "K2Node_MacroInstance.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditor.h"
#include "Misc/DataValidation.h"
#include "SMyBlueprint.h"
UUnusedMacroValidator::UUnusedMacroValidator()
{
	SetValidationEnabled(true);
}

bool UUnusedMacroValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UBlueprint>();
}

bool UUnusedMacroValidator::IsEnabled() const
{
	static const UUnusedMacroValidator* CDO = GetDefault<UUnusedMacroValidator>();
	return CDO->bIsEnabled && !bIsConfigDisabled;
}

EDataValidationResult UUnusedMacroValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	bIsError = false;

	if (UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
	{
		TArray<UEdGraph*> AllGraphs;
		AllGraphs.Append(Blueprint->UbergraphPages);
		AllGraphs.Append(Blueprint->FunctionGraphs);
		AllGraphs.Append(Blueprint->MacroGraphs);
		AllGraphs.Append(Blueprint->DelegateSignatureGraphs);
		AllGraphs.Append(Blueprint->IntermediateGeneratedGraphs);

		for(UEdGraph* MacroGraph : Blueprint->MacroGraphs)
		{
			if(!MacroGraph) continue;

			const FName MacroName = MacroGraph->GetFName();
			bool bIsMacroUsed = false;

			for (UEdGraph* Graph : AllGraphs)
			{
				if (!Graph || Graph == MacroGraph) continue;

				for (UEdGraphNode* Node : Graph->Nodes)
				{
					if (const UK2Node_MacroInstance* MacroInstance = Cast<UK2Node_MacroInstance>(Node))
					{
						if (MacroInstance->GetMacroGraph() == MacroGraph)
						{
							bIsMacroUsed = true;
							break;
						}
					}
				}

				if (bIsMacroUsed)
				{
					break;
				}
			}

			if(!bIsMacroUsed)
			{
				const FText MessageText = FText::Format(
					INVTEXT("Macro '{0}' is never used."),
					FText::FromName(MacroName)
				);

				TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);
				Message->AddToken(FActionToken::Create(FText::FromString("Jump to macro"), FText::GetEmpty(),
					FSimpleDelegate::CreateLambda([=]
						{
							if(Blueprint && MacroGraph)
							{
								if(UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
								{
									AssetEditorSubsystem->OpenEditorForAsset(Blueprint);
									if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
									{
										if(FBlueprintEditor* BlueprintEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
										{
											BlueprintEditor->OpenGraphAndBringToFront(MacroGraph, true);
											if(TSharedPtr<SMyBlueprint> MyBlueprintWidget = BlueprintEditor->GetMyBlueprintWidget())
											{
												MyBlueprintWidget->SelectItemByName(MacroGraph->GetFName(),
													ESelectInfo::Direct,
													INDEX_NONE,
													false);
											}
										}
									}
								}
							}
						})
				));

				const FText DeleteMacroText = FText::Format(
					INVTEXT("'Fix' - Delete Macro - '{0}'"),
					FText::FromName(MacroName));

				Message->AddToken(FActionToken::Create(DeleteMacroText, FText::GetEmpty(),
					FSimpleDelegate::CreateLambda([=]
						{
							if(Blueprint && MacroGraph)
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
														INVTEXT("Are you sure you want to delete the unused Macro '{0}' from Blueprint '{1}'?"),
														FText::FromName(MacroName),
														FText::FromString(Blueprint->GetName())
													);

													if(FMessageDialog::Open(EAppMsgType::YesNo, ConfirmText) == EAppReturnType::Yes)
													{
														Blueprint->Modify();

														Blueprint->MacroGraphs.Remove(MacroGraph);
														MacroGraph->Modify();
														MacroGraph->MarkAsGarbage();

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
