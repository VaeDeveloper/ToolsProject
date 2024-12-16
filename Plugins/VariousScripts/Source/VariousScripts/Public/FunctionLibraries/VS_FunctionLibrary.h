// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VS_FunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class VARIOUSSCRIPTS_API UVS_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()



public:
	UFUNCTION(BlueprintCallable, Category = "VariousScripts|Read/Write")
	static FString ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);

	UFUNCTION(BlueprintCallable, Category = "VariousScripts|Read/Write")
	static void WriteStringToFile(FString FilePath, FString String, bool& bOutSuccess, FString& OutInfoMessage);

	
};
