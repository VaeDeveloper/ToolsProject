// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/UnusedNodeValidator.h"
#include "Engine/Blueprint.h"

#include "K2Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Variable.h"
#include "K2Node_Event.h"
#include "K2Node_MacroInstance.h"
#include "EdGraphNode_Comment.h"

#include "Misc/DataValidation.h"

#include "BlueprintEditorModule.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Library/BPUtilsNodeFunctionLibrary.h"

UUnusedNodeValidator::UUnusedNodeValidator()
{
	SetValidationEnabled(true);
}

bool UUnusedNodeValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) const
{
	return InAsset && InAsset->IsA<UBlueprint>();
}

bool UUnusedNodeValidator::IsEnabled() const
{
	static const UUnusedNodeValidator* CDO = GetDefault<UUnusedNodeValidator>();
	return CDO->bIsEnabled && !bIsConfigDisabled;
}

EDataValidationResult UUnusedNodeValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	bIsError = false;

	if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
	{
		TArray<UEdGraph*> AllGraphs;
		Blueprint->GetAllGraphs(AllGraphs);

		for(UEdGraph* Graph : AllGraphs)
		{
			if(!Graph) continue;

			TArray<UEdGraphNode_Comment*> CommentNodes;
			for(UEdGraphNode* Node : Graph->Nodes)
			{
				if(UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(Node))
				{
					CommentNodes.Add(Comment);
				}
			}

			for(UEdGraphNode* Node : Graph->Nodes)
			{
				if(!Node || Node->IsA<UEdGraphNode_Comment>()) continue;

				if(UBPUtilsNodeFunctionLibrary::IsNodeInsideComment(Node, CommentNodes))
				{
					continue;
				}

				bool bIsNodeUnused = false;

				if(UK2Node_Event* Event = Cast<UK2Node_Event>(Node))
				{
					bIsNodeUnused = UBPUtilsNodeFunctionLibrary::IsEmptyEvent(Event);
				}
				else if(UK2Node_CallFunction* Function = Cast<UK2Node_CallFunction>(Node))
				{
					bIsNodeUnused = Function->IsNodePure()
						? UBPUtilsNodeFunctionLibrary::IsEmptyPureFunction(Function)
						: UBPUtilsNodeFunctionLibrary::IsEmptyFunctions(Function);
				}
				else if(UK2Node_MacroInstance* Macro = Cast<UK2Node_MacroInstance>(Node))
				{
					bIsNodeUnused = UBPUtilsNodeFunctionLibrary::IsUnusedMacroInstance(Macro);
				}
				else
				{
					bIsNodeUnused = UBPUtilsNodeFunctionLibrary::IsUnusedVariableNode(Node);
				}

				if(bIsNodeUnused)
				{
					const FText MessageText = FText::Format(
						INVTEXT("Node '{0}' in Graph '{1}' appears to be unused."),
						FText::FromString(Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString()),
						FText::FromString(Graph->GetName())
					);

					TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Warning, MessageText);

					Message->AddToken(FActionToken::Create(FText::FromString("Jump to graph"), FText::FromString(""),
						FSimpleDelegate::CreateLambda([=]
							{
								if(Blueprint && Graph)
								{
									UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
									AssetEditorSubsystem->OpenEditorForAsset(Blueprint);
									if(IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false))
									{
										if(IBlueprintEditor* BlueprintEditor = StaticCast<IBlueprintEditor*>(EditorInstance))
										{
											TSet<UObject*> NodesToSelect;
											NodesToSelect.Add(Node);
											if(TSharedPtr<SGraphEditor> GraphEditor = BlueprintEditor->OpenGraphAndBringToFront(Graph, true))
											{
												GraphEditor->JumpToNode(Node, false);


												const bool bHasChain = UBPUtilsNodeFunctionLibrary::HasExecutionOutputConnections(Node);

												FString Comment = bHasChain ? TEXT("Unused node chain") : TEXT("Unused node");
												Node->NodeComment = Comment;
												Node->bCommentBubbleVisible = true;

												// TODO search Engine Implemetable functions create Comment Node !!!  
												// FVector2D Position(Node->NodePosX - 50, Node->NodePosY - 50);
												// FVector2D Size(Node->NodeWidth + 500, Node->NodeHeight + 500);
												// AddCommentNode(Graph, Position, Size, TEXT("Unused node detected"));
												// this idea create comment block for node width/height size 

												FNotificationInfo Info(FText::FromString(Comment));
												Info.ExpireDuration = 3.0f;
												Info.bUseThrobber = false;
												Info.bUseSuccessFailIcons = false;
												Info.bFireAndForget = true;
												GraphEditor->AddNotification(Info, true);
											}
										}
									}
								}
							})
					));
					bIsError = true;
				}
			}
		}
	}

	return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}



