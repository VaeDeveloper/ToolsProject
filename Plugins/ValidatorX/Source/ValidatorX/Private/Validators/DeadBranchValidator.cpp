// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/DeadBranchValidator.h"

#include "K2Node_IfThenElse.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "EdGraph/EdGraph.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/DataValidation.h"
#include "SMyBlueprint.h"
#include "Library/BPUtilsNodeFunctionLibrary.h"
#define LOCTEXT_NAMESPACE "ValidatorX"



UDeadBranchValidator::UDeadBranchValidator()
{
	SetValidationEnabled(true);
}

bool UDeadBranchValidator::IsEnabled() const
{
	static const UDeadBranchValidator* CDO = GetDefault<UDeadBranchValidator>();
	return CDO->bIsEnabled && !bIsConfigDisabled;

}

bool UDeadBranchValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UBlueprint>();
}

EDataValidationResult UDeadBranchValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	bIsError = false;

	if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
	{
		TArray<UEdGraph*> Graphs;
		Blueprint->GetAllGraphs(Graphs);

		for(UEdGraph* Graph : Graphs)
		{
			if(!Graph) continue;

			for(UEdGraphNode* Node : Graph->Nodes)
			{
				if(UK2Node_IfThenElse* Branch = Cast<UK2Node_IfThenElse>(Node))
				{
					UEdGraphPin* Cond = Branch->GetConditionPin();
					if(!Cond) continue;

					bool bIsDead = false;
					FString Info;

					// Case 1: Literal condition
					if(Cond->LinkedTo.Num() == 0)
					{
						if(Cond->DefaultValue == "true" || Cond->DefaultValue == "false")
						{
							bIsDead = true;
							Info = FString::Printf(TEXT("Branch with literal condition '%s'"), *Cond->DefaultValue);
						}
					}

					// Case 2: Variable condition (unused)
					else if(Cond->LinkedTo.Num() == 1)
					{
						if(UK2Node_VariableGet* GetNode = Cast<UK2Node_VariableGet>(Cond->LinkedTo[0]->GetOwningNode()))
						{
							FName VarName = GetNode->GetVarName();
							FString SourceInfo;
							bool bFoundSet = UBPUtilsNodeFunctionLibrary::IsBoolVariableSetInThisOrParentBPs(Blueprint, VarName, &SourceInfo);

							// Not found in this BP
							if(!bFoundSet)
							{
								bool bCDOValue = false;

								if(UClass* GeneratedClass = Blueprint->GeneratedClass)
								{
									if(UObject* CDO = GeneratedClass->GetDefaultObject())
									{
										if(FProperty* Property = GeneratedClass->FindPropertyByName(VarName))
										{
											if(FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
											{
												bCDOValue = BoolProp->GetPropertyValue_InContainer(CDO);
											}
										}
									}
								}

								bIsDead = true;
								Info = FString::Printf(TEXT("Branch with variable '%s' that is never modified in this Blueprint (checked parents). %s"),
									*VarName.ToString(),
									SourceInfo.IsEmpty() ? TEXT("No Set found.") : *SourceInfo);
							}
						}
					}

					// Case 3: No logic on Then/Else
					auto AreAllBranchExecsDisconnected = [] (UK2Node_IfThenElse* BranchNode)
						{
							for(UEdGraphPin* Pin : BranchNode->Pins)
							{
								if(Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
								{
									if((Pin->PinName == "Then" || Pin->PinName == "Else") && Pin->LinkedTo.Num() > 0)
									{
										return false;
									}
								}
							}
							return true;
						};

					if(!bIsDead && AreAllBranchExecsDisconnected(Branch))
					{
						bIsDead = true;
						Info = TEXT("Branch has no execution logic on either output (Then/Else not connected)");
					}

					if(bIsDead)
					{
						const FText Msg = FText::Format(
							LOCTEXT("DeadBranch", "Dead branch detected in Graph '{0}': {1}"),
							FText::FromString(Graph->GetName()),
							FText::FromString(Info)
						);

						const TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, Msg);

						// Action: Jump to node
						Message->AddToken(FActionToken::Create(
							INVTEXT("Jump to Branch"),
							FText::GetEmpty(),
							FSimpleDelegate::CreateLambda([=]
								{
									if(Blueprint && Graph && Node)
									{
										UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
										AssetEditorSubsystem->OpenEditorForAsset(Blueprint);

										if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
										{
											if(FBlueprintEditor* BPEditor = StaticCast<FBlueprintEditor*>(EditorInstance))
											{
												if(TSharedPtr<SGraphEditor> GraphEditor = BPEditor->OpenGraphAndBringToFront(Graph, true))
												{
													GraphEditor->JumpToNode(Node, false);
												}
											}
										}
									}
								})
						));

						// Action: Delete branch node
						Message->AddToken(FActionToken::Create(
							FText::FromString(FString::Printf(TEXT("Fix: Delete Branch node in '%s'"), *Graph->GetName())),
							FText::GetEmpty(),
							FSimpleDelegate::CreateLambda([=]
								{
									if(Blueprint && Graph && Node)
									{
										const FText ConfirmText = FText::Format(
											INVTEXT("Are you sure you want to delete this Branch node from Graph '{0}'?"),
											FText::FromString(Graph->GetName())
										);

										if(FMessageDialog::Open(EAppMsgType::YesNo, ConfirmText) == EAppReturnType::Yes)
										{
											Graph->RemoveNode(Node);
											FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
										}
									}
								})
						));

						Node->NodeComment = TEXT("Dead branch detected");
						Node->bCommentBubbleVisible = true;
						bIsError = true;
					}
				}
			}
		}
	}

	return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}


#undef LOCTEXT_NAMESPACE