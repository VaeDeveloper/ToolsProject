// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/BPUtilsNodeFunctionLibrary.h"
#include "EdGraphNode_Comment.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_MacroInstance.h"
#include "EdGraphNode_Comment.h"
#include "K2Node_IfThenElse.h"
#include "AssetRegistry/AssetRegistryModule.h"

DEFINE_LOG_CATEGORY_STATIC(NodeFunctionLibraryLog, All, All);

void UBPUtilsNodeFunctionLibrary::GetDerivedRegistryBlueprintClasses(const UClass* ParentClass, TArray<UClass*>& OutDerived)
{
	if(!ParentClass)
	{
		UE_LOG(NodeFunctionLibraryLog, Warning, TEXT("[GetDerivedBlueprintClasses] ParentClass is nullptr."));
		return;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> BlueprintAssets;
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	AssetRegistryModule.Get().GetAssets(Filter, BlueprintAssets);

	int32 FoundCount = 0;

	for(const FAssetData& Asset : BlueprintAssets)
	{
		UBlueprint* LoadedBP = Cast<UBlueprint>(Asset.GetAsset());
		if(!LoadedBP || !LoadedBP->GeneratedClass) continue;

		if(LoadedBP->GeneratedClass->IsChildOf(ParentClass) && LoadedBP->GeneratedClass != ParentClass)
		{
			OutDerived.Add(LoadedBP->GeneratedClass);
			UE_LOG(NodeFunctionLibraryLog, Display, TEXT("[DerivedBP] %s"), *LoadedBP->GetName());
			++FoundCount;
		}
	}

	UE_LOG(NodeFunctionLibraryLog, Display, TEXT("Found %d derived blueprint classes (via AssetRegistry) of %s"), FoundCount, *ParentClass->GetName());
}

void UBPUtilsNodeFunctionLibrary::GetAllDerivedBlueprintClasses(const UClass* ParentClass, TArray<UClass*>& OutDerived, bool bSearchAssetRegistry)
{
	// 1. In-memory first
	UBPUtilsNodeFunctionLibrary::GetDerivedBlueprintClasses(ParentClass, OutDerived);

	// 2. If none found, fallback to AssetRegistry
	if(OutDerived.Num() == 0 && bSearchAssetRegistry)
	{
		UBPUtilsNodeFunctionLibrary::GetDerivedRegistryBlueprintClasses(ParentClass, OutDerived);
	}
}

void UBPUtilsNodeFunctionLibrary::GetDerivedBlueprintClasses(const UClass* ParentClass, TArray<UClass*>& OutDerived)
{
	if(!ParentClass)
	{
		UE_LOG(NodeFunctionLibraryLog, Warning, TEXT("[GetDerivedBlueprintClasses] ParentClass is nullptr."));
		return;
	}

	int32 FoundCount = 0;

	for(TObjectIterator<UBlueprintGeneratedClass> It; It; ++It)
	{
		UBlueprintGeneratedClass* Candidate = *It;

		if(Candidate && Candidate->IsChildOf(ParentClass) && Candidate != ParentClass)
		{
			OutDerived.Add(Candidate);
			UE_LOG(NodeFunctionLibraryLog, Display, TEXT("  [Derived] %s"), *Candidate->GetName());
			++FoundCount;
		}
	}

	UE_LOG(NodeFunctionLibraryLog, Display, TEXT("Found %d derived classes of %s"), FoundCount, *ParentClass->GetName());
}


bool UBPUtilsNodeFunctionLibrary::IsEmptyEvent(const UK2Node_Event* EventNode)
{
	/** Placed Ghost Node */
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

bool UBPUtilsNodeFunctionLibrary::IsEmptyFunctions(const UK2Node_CallFunction* EventNode)
{
	if(!EventNode) return false;

	UEdGraphPin* ExecThenPin = EventNode->FindPin(UEdGraphSchema_K2::PN_Execute);
	UEdGraphPin* RetPin = EventNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);

	if(ExecThenPin && !ExecThenPin->LinkedTo.IsEmpty())  return false;
	if(RetPin && !RetPin->LinkedTo.IsEmpty()) return false;

	return true;
}

bool UBPUtilsNodeFunctionLibrary::IsUnusedVariableGet(const UK2Node_VariableGet* Node)
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

bool UBPUtilsNodeFunctionLibrary::IsUnusedVariableSet(const UK2Node_VariableSet* Node)
{
	if(!Node) return false;

	UEdGraphPin* ExecPin = Node->FindPin(UEdGraphSchema_K2::PN_Execute);
	UEdGraphPin* ThenPin = Node->FindPin(UEdGraphSchema_K2::PN_Then);

	bool bNoExec = !ExecPin || ExecPin->LinkedTo.IsEmpty();
	bool bNoThen = !ThenPin || ThenPin->LinkedTo.IsEmpty();

	// TODO && or || retrun ? 
	return bNoExec && bNoThen;
}

bool UBPUtilsNodeFunctionLibrary::IsUnusedMacroInstance(const UK2Node_MacroInstance* Node)
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


bool UBPUtilsNodeFunctionLibrary::IsEmptyPureFunction(const UK2Node_CallFunction* Node)
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

bool UBPUtilsNodeFunctionLibrary::IsUnusedVariableNode(UEdGraphNode* Node)
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

bool UBPUtilsNodeFunctionLibrary::IsNodeInsideComment(UEdGraphNode* Node, const TArray<UEdGraphNode_Comment*>& CommentNodes)
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

bool UBPUtilsNodeFunctionLibrary::HasExecutionOutputConnections(const UEdGraphNode* Node)
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

FString UBPUtilsNodeFunctionLibrary::GetGraphType(UBlueprint* Blueprint, UEdGraph* Graph)
{
	if(Blueprint->FunctionGraphs.Contains(Graph)) return TEXT("Function");
	if(Blueprint->MacroGraphs.Contains(Graph)) return TEXT("Macro");
	if(Blueprint->UbergraphPages.Contains(Graph)) return TEXT("Event Graph");
	if(Blueprint->DelegateSignatureGraphs.Contains(Graph)) return TEXT("Delegate");
	if(Blueprint->IntermediateGeneratedGraphs.Contains(Graph)) return TEXT("Intermediate");

	return TEXT("Unknown");
}

bool UBPUtilsNodeFunctionLibrary::IsBoolVariableSetInThisOrParentBPs(UBlueprint* Blueprint, FName VarName, FString* OutSourceInfo)
{
	if(!Blueprint)
	{
		UE_LOG(NodeFunctionLibraryLog, Warning, TEXT("[IsBoolVariableSet] Blueprint is nullptr."));
		return false;
	}

	UBlueprint* CurrentBP = Blueprint;
	while(CurrentBP)
	{
		UE_LOG(NodeFunctionLibraryLog, Display, TEXT("[IsBoolVariableSet] Checking Blueprint: %s"), *CurrentBP->GetName());

		TArray<UEdGraph*> Graphs;
		CurrentBP->GetAllGraphs(Graphs);

		for(UEdGraph* Graph : Graphs)
		{
			if(!Graph) continue;

			UE_LOG(NodeFunctionLibraryLog, VeryVerbose, TEXT("  [Graph] %s"), *Graph->GetName());

			for(UEdGraphNode* Node : Graph->Nodes)
			{
				if(UK2Node_VariableSet* SetNode = Cast<UK2Node_VariableSet>(Node))
				{
					const FName SetVarName = SetNode->GetVarName();
					UE_LOG(NodeFunctionLibraryLog, Verbose, TEXT("[SetNode] Variable: %s"), *SetVarName.ToString());

					if(SetVarName == VarName)
					{
						UE_LOG(NodeFunctionLibraryLog, Display, TEXT("Found SET for '%s' in BP '%s', Graph '%s'"),
							*VarName.ToString(),
							*CurrentBP->GetName(),
							*Graph->GetName());

						if(OutSourceInfo)
						{
							*OutSourceInfo = FString::Printf(TEXT("Set found in Blueprint: %s, Graph: %s"),
								*CurrentBP->GetName(),
								*Graph->GetName());
						}
						return true;
					}
				}
			}
		}

		UClass* ParentClass = CurrentBP->ParentClass;
		if(!ParentClass)
		{
			UE_LOG(NodeFunctionLibraryLog, Display, TEXT("[IsBoolVariableSet] Reached top base class from %s"), *CurrentBP->GetName());
			break;
		}

		CurrentBP = UBlueprint::GetBlueprintFromClass(ParentClass);
		if(!CurrentBP)
		{
			UE_LOG(NodeFunctionLibraryLog, Display, TEXT("[IsBoolVariableSet] No Blueprint found for ParentClass: %s"), *ParentClass->GetName());
		}
	}

	if(OutSourceInfo)
	{
		*OutSourceInfo = TEXT("No Set found in this Blueprint or any parent.");
	}

	UE_LOG(NodeFunctionLibraryLog, Display, TEXT("No SET found for variable '%s' in %s or parents."),
		*VarName.ToString(),
		*Blueprint->GetName());

	return false;
}

bool UBPUtilsNodeFunctionLibrary::AreAllBranchExecsDisconnected(const UK2Node_IfThenElse* Branch)
{
	TArray<UEdGraphPin*> ExecPins;

	for(UEdGraphPin* Pin : Branch->Pins)
	{
		if(Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
		{
			if(Pin->PinName == "Then" || Pin->PinName == "Else")
			{
				ExecPins.Add(Pin);
			}
		}
	}

	for(UEdGraphPin* ExecPin : ExecPins)
	{
		if(ExecPin && ExecPin->LinkedTo.Num() > 0)
		{
			return false;
		}
	}

	return true;
}





