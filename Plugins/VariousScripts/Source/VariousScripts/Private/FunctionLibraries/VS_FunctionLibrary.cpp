// Fill out your copyright notice in the Description page of Project Settings.

#include "FunctionLibraries/VS_FunctionLibrary.h"

#include "HAL/PlatformFileManager.h"

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
		OutInfoMessage = FString::Printf(TEXT("Write String To File Failed - Was not able to read file. Is your file read only? Is the path valid ? - '%s'"));
		return;
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write String To File Successed!!! - '%s'"), *FilePath);
}
