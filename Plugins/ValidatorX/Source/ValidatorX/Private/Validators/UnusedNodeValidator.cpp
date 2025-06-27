// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/UnusedNodeValidator.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphSchema_K2.h"
#include "EdGraphNode_Comment.h"
#include "UObject/ConstructorHelpers.h"
#include "K2Node.h"
#include "Misc/DataValidation.h"
#include "K2Node_Variable.h"
#include "Engine/Blueprint.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealTypePrivate.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/UnrealType.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_MacroInstance.h"

#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_VariableSetRef.h"
#include "Engine/MemberReference.h"
#include "BlueprintEditorModule.h"
#include "Widgets/Notifications/SNotificationList.h"


void AddCommentNode(UEdGraph* Graph, const FVector2D& Position, const FVector2D& Size, const FString& CommentText)
{
	if(!Graph) return;

	
	UEdGraphNode_Comment* CommentNode = NewObject<UEdGraphNode_Comment>(Graph);
	Graph->AddNode(CommentNode, true, false);

	CommentNode->NodePosX = Position.X;
	CommentNode->NodePosY = Position.Y;
	CommentNode->NodeWidth = Size.X;
	CommentNode->NodeHeight = Size.Y;
	CommentNode->NodeComment = CommentText;
	CommentNode->CommentDepth = 0;
	

	CommentNode->AllocateDefaultPins();
	CommentNode->PostPlacedNewNode();
	CommentNode->SnapToGrid(16);
	CommentNode->ReconstructNode();

	Graph->NotifyGraphChanged();
}

namespace ValidatorX
{
	void GetDerivedBlueprintClasses(const UClass* ParentClass, TArray<UClass*>& OutDerived)
	{
		for(TObjectIterator<UBlueprintGeneratedClass> It; It; ++It)
		{
			if(It->IsChildOf(ParentClass) && *It != ParentClass)
			{
				OutDerived.Add(*It);
			}
		}
	}

	bool IsEmptyEvent(UK2Node_Event* EventNode)
	{
		if(!EventNode || EventNode->IsAutomaticallyPlacedGhostNode() /*|| EventNode->bOverrideFunction*/) return false;

		UEdGraphPin* ExecThenPin = EventNode->FindPin(UEdGraphSchema_K2::PN_Then);
		if(!ExecThenPin || !ExecThenPin->LinkedTo.IsEmpty()) return false;

		const UBlueprint* Blueprint = EventNode->GetBlueprint();
		if(!Blueprint || !Blueprint->GeneratedClass)  return true;

		const FName EventName = EventNode->GetFunctionName();
		const UClass* ThisClass = Blueprint->GeneratedClass;

		TArray<UClass*> DerivedClasses;
		GetDerivedBlueprintClasses(ThisClass, DerivedClasses);

		for(UClass* DerivedClass : DerivedClasses)
		{
			if(!DerivedClass) continue;

			UBlueprint* DerivedBP = Cast<UBlueprint>(DerivedClass->ClassGeneratedBy);
			if(!DerivedBP) continue;

			for(UEdGraph* Graph : DerivedBP->UbergraphPages)
			{
				for(UEdGraphNode* Node : Graph->Nodes)
				{
					UK2Node_Event* ChildEvent = Cast<UK2Node_Event>(Node);
					if(ChildEvent && ChildEvent->GetFunctionName() == EventName)
					{
						UEdGraphPin* ChildThen = ChildEvent->FindPin(UEdGraphSchema_K2::PN_Then);
						if(ChildThen && !ChildThen->LinkedTo.IsEmpty())
						{
							return false;
						}
					}
				}
			}
		}

		return true;
	}

	bool IsEmptyFunctions(UK2Node_CallFunction* EventNode)
	{
		if(!EventNode) return false;

		UEdGraphPin* ExecThenPin = EventNode->FindPin(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* RetPin = EventNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
		if(ExecThenPin && !ExecThenPin->LinkedTo.IsEmpty())  return false;
		if(RetPin && !RetPin->LinkedTo.IsEmpty()) return false;
		return true;
	}

	bool IsUnusedVariableGet(UK2Node_VariableGet* Node)
	{
		if(!Node) return false;

		for(UEdGraphPin* Pin : Node->Pins)
		{
			if(Pin->Direction == EGPD_Output && !Pin->LinkedTo.IsEmpty())
			{
				return false;
			}
		}

		return true;
	}

	bool IsUnusedMacroInstance(const UK2Node_MacroInstance* Node)
	{
		if(!Node) return false;

		for(UEdGraphPin* Pin : Node->Pins)
		{
			if(!Pin->LinkedTo.IsEmpty())
			{
				return false;
			}
		}

		return true;
	}

	bool IsUnusedVariableSet(UK2Node_VariableSet* Node)
	{
		if(!Node) return false;

		UEdGraphPin* ExecPin = Node->FindPin(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* ThenPin = Node->FindPin(UEdGraphSchema_K2::PN_Then);

		bool bNoExec = !ExecPin || ExecPin->LinkedTo.IsEmpty();
		bool bNoThen = !ThenPin || ThenPin->LinkedTo.IsEmpty();

		return bNoExec && bNoThen;
	}

	bool IsEmptyPureFunction(UK2Node_CallFunction* Node)
	{
		if(!Node || !Node->IsNodePure()) return false;

		for(UEdGraphPin* Pin : Node->Pins)
		{
			if(Pin->Direction == EGPD_Output && !Pin->LinkedTo.IsEmpty())
			{
				return false;
			}
		}

		return true;
	}

	bool IsUnusedVariableNode(UEdGraphNode* Node)
	{
		if(UK2Node_VariableGet* VarGet = Cast<UK2Node_VariableGet>(Node))
		{
			return IsUnusedVariableGet(VarGet);
		}
		if(UK2Node_VariableSet* VarSet = Cast<UK2Node_VariableSet>(Node))
		{
			return IsUnusedVariableSet(VarSet);
		}
		return false;
	}

	bool IsNodeInsideComment(UEdGraphNode* Node, const TArray<UEdGraphNode_Comment*>& CommentNodes)
	{
		if(!Node) return false;

		const FVector2D NodePos(Node->NodePosX, Node->NodePosY);

		for(UEdGraphNode_Comment* Comment : CommentNodes)
		{
			const FVector2D CommentPos(Comment->NodePosX, Comment->NodePosY);
			const FVector2D CommentSize(Comment->NodeWidth, Comment->NodeHeight);

			const bool bIsInsideBounds =
				NodePos.X >= CommentPos.X && NodePos.X <= CommentPos.X + CommentSize.X &&
				NodePos.Y >= CommentPos.Y && NodePos.Y <= CommentPos.Y + CommentSize.Y;

			const FString CommentText = Comment->NodeComment.ToLower();

			if(bIsInsideBounds /*&& CommentText.Contains(TEXT("todo"))*/)
			{
				return true;
			}
		}

		return false;
	}

	bool HasExecutionOutputConnections(const UEdGraphNode* Node)
	{
		for(UEdGraphPin* Pin : Node->Pins)
		{
			if(Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
			{
				if(Pin->LinkedTo.Num() > 0)
				{
					return true;
				}
			}
		}
		return false;
	}
};

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
		TArray<UEdGraph*> AllGraphs = Blueprint->UbergraphPages;
		AllGraphs.Append(Blueprint->FunctionGraphs);
		AllGraphs.Append(Blueprint->MacroGraphs);
		AllGraphs.Append(Blueprint->DelegateSignatureGraphs);
		AllGraphs.Append(Blueprint->IntermediateGeneratedGraphs);
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

				if(ValidatorX::IsNodeInsideComment(Node, CommentNodes))
				{
					continue;
				}

				bool bIsNodeUnused = false;

				if(UK2Node_Event* Event = Cast<UK2Node_Event>(Node))
				{
					bIsNodeUnused = ValidatorX::IsEmptyEvent(Event);
				}
				else if(UK2Node_CallFunction* Function = Cast<UK2Node_CallFunction>(Node))
				{
					bIsNodeUnused = Function->IsNodePure()
						? ValidatorX::IsEmptyPureFunction(Function)
						: ValidatorX::IsEmptyFunctions(Function);
				}
				else if(UK2Node_MacroInstance* Macro = Cast<UK2Node_MacroInstance>(Node))
				{
					bIsNodeUnused = ValidatorX::IsUnusedMacroInstance(Macro);
				}
				else
				{
					bIsNodeUnused = ValidatorX::IsUnusedVariableNode(Node);
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


												const bool bHasChain = ValidatorX::HasExecutionOutputConnections(Node);

												FString Comment = bHasChain ? TEXT("Unused node chain") : TEXT("Unused node");
												Node->NodeComment = Comment;
												Node->bCommentBubbleVisible = true;

												 // FVector2D Position(Node->NodePosX - 50, Node->NodePosY - 50);
												 // FVector2D Size(Node->NodeWidth + 500, Node->NodeHeight + 500);
												 // AddCommentNode(Graph, Position, Size, TEXT("Unused node detected"));

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



