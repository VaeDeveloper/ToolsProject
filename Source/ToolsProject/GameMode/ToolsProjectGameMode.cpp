// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolsProjectGameMode.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "DynamicResolutionState.h"


AToolsProjectGameMode::AToolsProjectGameMode()
{
	if(GEngine)
	{
		GEngine->SetDynamicResolutionUserSetting(true);
		
		UGameUserSettings* Settings = GEngine->GetGameUserSettings();
		if(Settings)
		{
			Settings->SetDynamicResolutionEnabled(true);
			GEngine->GameUserSettings->ApplyNonResolutionSettings();

			bool bIsEnabled = Settings->IsDynamicResolutionEnabled();

			FString Message = FString::Printf(TEXT("Dynamic Resolution is %s"), bIsEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Message);

			GEngine->Exec(GetWorld(), TEXT("r.DynamicRes.OperationMode 2"));
			GEngine->Exec(GetWorld(), TEXT("r.DynamicRes.FrameFraction 0.8"));
			GEngine->Exec(GetWorld(), TEXT("r.DynamicRes.MinScreenPercentage 50"));
			GEngine->Exec(GetWorld(), TEXT("r.DynamicRes.MaxScreenPercentage 100"));


			bool bRsyIsEnabled = Settings->IsDynamicResolutionEnabled();

			FString Message1 = FString::Printf(TEXT("Dynamic Resolution is %s"), bRsyIsEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Message1);
			GEngine->GameUserSettings->bUseDynamicResolution = true;



			IDynamicResolutionState* DynResState = GEngine->GetDynamicResolutionState();
			if(!DynResState)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("DynamicResolutionState is NULL"));
				return;
			}

			bool bSupported = DynResState->IsSupported();

			FString Info = FString::Printf(TEXT("DynamicRes: %s / Supported: %s"),
				bIsEnabled ? TEXT("ENABLED") : TEXT("DISABLED"),
				bSupported ? TEXT("YES") : TEXT("NO"));

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Info);

		}
	}
}
// 