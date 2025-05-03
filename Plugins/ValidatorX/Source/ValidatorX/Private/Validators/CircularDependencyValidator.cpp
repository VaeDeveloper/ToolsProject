// Fill out your copyright notice in the Description page of Project Settings.


#include "Validators/CircularDependencyValidator.h"
#include "EdGraph/EdGraph.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MacroInstance.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Editor/EditorEngine.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "IAssetTools.h"
#include "EdGraphSchema_K2.h"
#include "Misc/DataValidation.h"
#include "BlueprintEditor.h"

UEdGraph* UCircularDependencyValidator::FindGraphByName(UBlueprint* Blueprint, const FName& GraphName)
{
	auto FindInArray = [&GraphName] (const TArray<UEdGraph*>& Graphs) -> UEdGraph*
		{
			for(UEdGraph* Graph : Graphs)
			{
				if(Graph && Graph->GetFName() == GraphName)
				{
					return Graph;
				}
			}
			return nullptr;
		};

	if(UEdGraph* Found = FindInArray(Blueprint->FunctionGraphs)) return Found;
	if(UEdGraph* Found = FindInArray(Blueprint->MacroGraphs)) return Found;

	return nullptr;
}

UCircularDependencyValidator::UCircularDependencyValidator()
{
	SetValidationEnabled(true);
}

void UCircularDependencyValidator::SetValidationEnabled(bool bEnabled)
{
	static UCircularDependencyValidator* CDO = GetMutableDefault<UCircularDependencyValidator>();
	if(bIsConfigDisabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("Validator is disabled by config!"));
		return;
	}

	CDO->bIsEnabled = bEnabled;
	SaveConfig();
}

bool UCircularDependencyValidator::IsEnabled() const
{
	static const UCircularDependencyValidator* CDO = GetDefault<UCircularDependencyValidator>();
	return CDO->bIsEnabled && !bIsConfigDisabled;

}

bool UCircularDependencyValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UBlueprint>();
}

EDataValidationResult UCircularDependencyValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	bIsError = false;

	if(UBlueprint* Blueprint = Cast<UBlueprint>(InAsset))
	{
		if(HasCircularDependency(Blueprint, Context))
		{
			bIsError = true;
		}
	}

	return bIsError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

bool UCircularDependencyValidator::HasCircularDependency(UBlueprint* Blueprint, FDataValidationContext& Context)
{
	TMap<FName, TArray<FName>> CallGraph;

	auto CollectGraphCalls = [&] (const TArray<UEdGraph*>& Graphs)
		{
			for(UEdGraph* Graph : Graphs)
			{
				if(!Graph) continue;

				FName ThisGraphName = Graph->GetFName();
				TArray<FName>& Called = CallGraph.FindOrAdd(ThisGraphName);

				for(UEdGraphNode* Node : Graph->Nodes)
				{
					if(UK2Node_CallFunction* CallFunction = Cast<UK2Node_CallFunction>(Node))
					{
						Called.Add(CallFunction->FunctionReference.GetMemberName());
					}
					else if(UK2Node_MacroInstance* Macro = Cast<UK2Node_MacroInstance>(Node))
					{
						Called.Add(Macro->GetMacroGraph()->GetFName());
					}
				}
			}
		};

	CollectGraphCalls(Blueprint->FunctionGraphs);
	CollectGraphCalls(Blueprint->MacroGraphs);

	for(const auto& Pair : CallGraph)
	{
		const FName& Start = Pair.Key;
		TSet<FName> Visited;
		TSet<FName> Stack;
		TArray<FName> CyclePath;

		if(DetectCycle(Start, CallGraph, Visited, Stack, CyclePath))
		{
			FString CycleStr = FString::JoinBy(CyclePath, TEXT(" - "), [] (const FName& Name) { return Name.ToString(); });
			const FText MessageText = FText::FromString(FString::Printf(TEXT("Circular call detected: %s"), *CycleStr));

			TSharedRef<FTokenizedMessage> Message = Context.AddMessage(EMessageSeverity::Error, MessageText);
			if(UEdGraph* TargetGraph = FindGraphByName(Blueprint, CyclePath[0]))
			{
				Message->AddToken(FActionToken::Create(
					FText::FromString("Jump to graph"),
					FText::FromString("Opens the first function or macro involved in the circular call"),
					FSimpleDelegate::CreateLambda([Blueprint, TargetGraph] ()
						{
							if(UAssetEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
							{
								Subsystem->OpenEditorForAsset(Blueprint);
								if(IAssetEditorInstance* EditorInstance = Subsystem->FindEditorForAsset(Blueprint, false))
								{
									if(IBlueprintEditor* BPEditor = StaticCast<IBlueprintEditor*>(EditorInstance))
									{
										BPEditor->OpenGraphAndBringToFront(TargetGraph, true);
									}
								}
							}
						})
				));
			}

			return true;
		}
	}

	return false;
}

bool UCircularDependencyValidator::DetectCycle(const FName& StartName, TMap<FName, TArray<FName>>& GraphMap, TSet<FName>& Visited, TSet<FName>& Stack, TArray<FName>& OutCyclePath)
{
	if(Stack.Contains(StartName))
	{
		OutCyclePath.Add(StartName);
		return true;
	}

	if(Visited.Contains(StartName))
	{
		return false;
	}

	Visited.Add(StartName);
	Stack.Add(StartName);

	if(TArray<FName>* CalledList = GraphMap.Find(StartName))
	{
		for(const FName& Next : *CalledList)
		{
			if(DetectCycle(Next, GraphMap, Visited, Stack, OutCyclePath))
			{
				OutCyclePath.Insert(StartName, 0);
				return true;
			}
		}
	}

	Stack.Remove(StartName);
	return false;
}


