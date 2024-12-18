// Fill out your copyright notice in the Description page of Project Settings.

#include "FunctionLibraries/VS_FunctionLibrary.h"

#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h" 
#include "UObject/Class.h"       
#include "UObject/UnrealType.h"


FString UVS_FunctionLibrary::ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	if (! FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read String From File Failed - File doesn't exist - '%s'"), *FilePath);
		return FString("");
	}

	FString RetString = "";

	if (! FFileHelper::LoadFileToString(RetString, *FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read String From File Failed - Was not able to read file. Is this a text file - '%s'"), *FilePath);
		return FString("");
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read String From File Successed - '%s'"), *FilePath);
	return RetString;
}

void UVS_FunctionLibrary::WriteStringToFile(FString FilePath, FString String, bool& bOutSuccess, FString& OutInfoMessage) 
{
	if (! FFileHelper::SaveStringToFile(String, *FilePath))
	{
		
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Write String To File Failed - Was not able to read file. Is your file read only? Is the path valid ? - '%s'"), *FilePath);
		return;
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write String To File Successed!!! - '%s'"), *FilePath);
}

UAssetImportTask* UVS_FunctionLibrary::CreateImportTask(FString SourcePath, FString DestinationPath)
{
	return nullptr;
}

TSharedPtr<FJsonObject> UVS_FunctionLibrary::ReadJson(FString JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	FString JsonStr = ReadStringFromFile(JsonFilePath, bOutSuccess, OutInfoMessage);
	if (! bOutSuccess) return nullptr;
	
	TSharedPtr<FJsonObject> ReturnJsonObject;
	
	if (! FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonStr), ReturnJsonObject))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read Json File Failed - Was not able Deserialize the json string. Is it the rigth format :  '%s'"), *JsonStr);
		return nullptr;
	}

	if (! ReturnJsonObject.IsValid())
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read Json File Failed - JSON object is invalid. Is the format correct? '%s'"), *JsonStr);
		return nullptr;
	}
	
	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read Json File Success - '%s'"), *JsonStr);
	return ReturnJsonObject;
}

void UVS_FunctionLibrary::WriteJson(FString JsonFilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess, FString& OutInfoMessage) 
{
	FString JsonString;

	if (!JsonObject.IsValid())
	{
		bOutSuccess = false;
		OutInfoMessage = TEXT("Write Json File Failed - Invalid JSON object provided.");
		return;
	}

	if (! FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString)))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Jsong file write failed"));
		return;
	}

	WriteStringToFile(JsonFilePath, JsonString, bOutSuccess, OutInfoMessage);
	if (! bOutSuccess) return;

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write Json file Success - '%s'"), *JsonFilePath);
}


