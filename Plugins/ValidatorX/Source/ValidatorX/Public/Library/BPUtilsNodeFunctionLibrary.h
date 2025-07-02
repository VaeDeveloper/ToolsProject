// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPUtilsNodeFunctionLibrary.generated.h"

class UK2Node_Event;
class UK2Node_IfThenElse;
class UK2Node_VariableGet;
class UK2Node_VariableSet;
class UEdGraphNode_Comment;
class UK2Node_CallFunction;
class UK2Node_MacroInstance;
/**
 * 
 */
UCLASS()
class VALIDATORX_API UBPUtilsNodeFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Populates an array with all Blueprint-generated classes that are derived from the specified parent class.
	 *
	 * This function iterates over all loaded UBlueprintGeneratedClass objects in memory
	 * and checks whether each one is a subclass of the given ParentClass (excluding the ParentClass itself).
	 * Only classes that are already loaded into memory will be returned.
	 *
	 * @param ParentClass   The base class to search derived Blueprint classes from. Must not be null.
	 * @param OutDerived    The output array that will be filled with all matching derived Blueprint-generated classes.
	 */
	static void GetDerivedBlueprintClasses(const UClass* ParentClass, TArray<UClass*>& OutDerived);
	
	static void GetDerivedRegistryBlueprintClasses(const UClass* ParentClass, TArray<UClass*>& OutDerived);
	
	static void GetAllDerivedBlueprintClasses(const UClass* ParentClass, TArray<UClass*>& OutDerived, bool bSearchAssetRegistry = true);


	static bool IsEmptyEvent(const UK2Node_Event* EventNode);
	static bool IsEmptyFunctions(const UK2Node_CallFunction* EventNode);
	static bool IsUnusedVariableGet(const UK2Node_VariableGet* Node);
	static bool IsUnusedVariableSet(const UK2Node_VariableSet* Node);
	static bool IsUnusedMacroInstance(const UK2Node_MacroInstance* Node);
	static bool IsEmptyPureFunction(const UK2Node_CallFunction* Node);
	static bool IsUnusedVariableNode(UEdGraphNode* Node);

	static bool IsNodeInsideComment(UEdGraphNode* Node, const TArray<UEdGraphNode_Comment*>& CommentNodes);
	static bool HasExecutionOutputConnections(const UEdGraphNode* Node);

	static FString GetGraphType(UBlueprint* Blueprint, UEdGraph* Graph);

	static bool IsBoolVariableSetInThisOrParentBPs(UBlueprint* Blueprint, FName VarName, FString* OutSourceInfo = nullptr);
	static bool AreAllBranchExecsDisconnected(const UK2Node_IfThenElse* Branch);

};
