// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValidatorX.h"
#include "ValidatorXManager.h"
#include "Widgets/SValidatorWidget.h"
#include "EditorValidatorSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogValidatorX, All, All);

#define LOCTEXT_NAMESPACE "FValidatorXModule"

const FName FValidatorXModule::ValidatorXTabName = "ValidatorX";

void FValidatorXModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FValidatorXModule::HandlePostEngineInit);

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ValidatorXTabName, FOnSpawnTab::CreateRaw(this, &FValidatorXModule::OnSpawnValidatorXTab))
        .SetDisplayName(NSLOCTEXT("ValidatorX", "TabTitle", "ValidatorX"))
        .SetMenuType(GetVisibleModule());
}

void FValidatorXModule::ShutdownModule()
{

}

ETabSpawnerMenuType::Type FValidatorXModule::GetVisibleModule() const
{
	if(FModuleManager::Get().IsModuleLoaded("ToolProjectEditor"))
	{
		ETabSpawnerMenuType::Enabled;
	}
	return ETabSpawnerMenuType::Hidden;
}

void FValidatorXModule::HandlePostEngineInit()
{
	UE_LOG(LogTemp, Warning, TEXT("Startup Begin"));

	if(GEditor)
	{
		UE_LOG(LogTemp, Warning, TEXT("GEditor is valid"));

		UEditorValidatorSubsystem* ValidatorSubsystem = GEditor->GetEditorSubsystem<UEditorValidatorSubsystem>();
		if(ValidatorSubsystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("ValidatorSubsystem is valid"));

			ValidatorSubsystem->ForEachEnabledValidator(
				[this] (UEditorValidatorBase* Validator)
				{
					UE_LOG(LogTemp, Warning, TEXT("Validator found: %s"), *Validator->GetName());

					if(UBlueprintValidatorBase* BlueprintValidator = Cast<UBlueprintValidatorBase>(Validator))
					{
						UE_LOG(LogTemp, Warning, TEXT("Registering BlueprintValidator: %s"), *BlueprintValidator->GetName());
						FValidatorXManager::Get().RegisterValidator(BlueprintValidator);
						BlueprintValidator->SetValidationEnabled(false);
					}
					return true;
				});
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ValidatorSubsystem is nullptr"));
		}
	}
}

TSharedRef<SDockTab> FValidatorXModule::OnSpawnValidatorXTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SValidatorWidget)
			.Validators(FValidatorXManager::Get().GetValidators())
		];
}

void FValidatorXModule::OpenValidatorXTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ValidatorXTabName);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FValidatorXModule, ValidatorX)