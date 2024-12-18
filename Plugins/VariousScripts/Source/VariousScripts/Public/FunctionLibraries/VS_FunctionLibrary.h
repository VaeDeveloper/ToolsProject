// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Templates/SharedPointer.h"

#include "VS_FunctionLibrary.generated.h"

class FJsonObject;

/**
 *
 */
UCLASS()
class VARIOUSSCRIPTS_API UVS_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	// FILE
public:
	UFUNCTION(BlueprintCallable, Category = "VariousScripts|Read/Write")
	static FString ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);

	UFUNCTION(BlueprintCallable, Category = "VariousScripts|Read/Write")
	static void WriteStringToFile(FString FilePath, FString String, bool& bOutSuccess, FString& OutInfoMessage);

	UFUNCTION(BlueprintCallable, Category = "VariousScripts|Import/Export")
	static UAssetImportTask* CreateImportTask(FString SourcePath, FString DestinationPath);



private:
	static TSharedPtr<FJsonObject> ReadJson(FString JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage);
	static void WriteJson(FString JsonFilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess, FString& OutInfoMessage);
};
