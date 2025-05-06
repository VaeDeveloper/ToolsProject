// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConsoleCommandCenter.h"
#include "LevelEditor.h"
#include "ToolMenus.h"


#define LOCTEXT_NAMESPACE "FConsoleCommandCenterModule"
/* clang-format off */




void FConsoleCommandCenterModule::StartupModule() 
{
    FSlateStyleSet* CheckBoxStyleSet = new FSlateStyleSet("CheckBoxCustomStyle");
   
    FSlateBrush* NormalBrush = new FSlateBrush();
    NormalBrush->TintColor = FLinearColor(0.495466, 0.495466, 0.495466, 1.0f);
    NormalBrush->DrawAs = ESlateBrushDrawType::RoundedBox;
    NormalBrush->OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
    NormalBrush->OutlineSettings.CornerRadii = FVector4(4.0f, 4.0f, 4.0f, 4.0f);
    
  
    FSlateBrush* HoveredBrush = new FSlateBrush();
    HoveredBrush->TintColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
    HoveredBrush->DrawAs = ESlateBrushDrawType::RoundedBox;
    HoveredBrush->OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
    HoveredBrush->OutlineSettings.CornerRadii = FVector4(4.0f, 4.0f, 4.0f, 4.0f);
    
    FSlateBrush* CheckedBrush = new FSlateBrush(*NormalBrush);
    CheckedBrush->TintColor = FLinearColor(FColor::FromHex("057FFFFF"));
    CheckedBrush->DrawAs = ESlateBrushDrawType::RoundedBox;
    CheckedBrush->OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
    CheckedBrush->OutlineSettings.CornerRadii = FVector4(4.0f, 4.0f, 4.0f, 4.0f);

    FSlateBrush* UndeterminedBrush = new FSlateBrush(*NormalBrush);
    UndeterminedBrush->TintColor = FLinearColor::Gray;
    
    FCheckBoxStyle CustomCheckBoxStyle;
    CustomCheckBoxStyle
        .SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
        .SetUncheckedImage(*NormalBrush)
        .SetUncheckedHoveredImage(*HoveredBrush)
        .SetCheckedImage(*CheckedBrush)
        .SetCheckedHoveredImage(*HoveredBrush)
        .SetUndeterminedImage(*UndeterminedBrush)
        .SetUndeterminedHoveredImage(*HoveredBrush);
    
    CheckBoxStyleSet->Set("CheckBoxCustomStyle", CustomCheckBoxStyle);
    FSlateStyleRegistry::RegisterSlateStyle(*CheckBoxStyleSet);

    FSlateFontInfo FontInfo = FAppStyle::GetFontStyle("NormalFont");
    FontInfo.Size = 15.0f;
    
    TArray<FString> StatCommands = 
    {
    	TEXT("Stat FPS"),
    	TEXT("Stat Unit"),
    	TEXT("Stat UnitGraph"),
    	TEXT("Stat Game"),
    	TEXT("Stat GPU"),
    	TEXT("Stat SceneRendering"),
    	TEXT("Stat RHI"),
    	TEXT("Stat Memory"),
    	TEXT("Stat Streaming"),
    	TEXT("Stat InitViews"),
    	TEXT("Stat Slate"),
    	TEXT("Stat TickGroups"),
    	TEXT("Stat AI"),
    	TEXT("Stat Navigation"),
    	TEXT("Stat Physics"),
    	TEXT("Stat Collision"),
    	TEXT("Stat Anim"),
    	TEXT("Stat AnimationBudget"),
    	TEXT("Stat Niagra"),
    	TEXT("Stat NiagaraOverview"),
    	TEXT("Stat Audio"),
    	TEXT("Stat SoundWaves"),
    	TEXT("Stat SoundCues"),
    	TEXT("Stat Levels"),
    	TEXT("Stat Hitches"),
    	TEXT("Stat Raw"),
    	TEXT("Stat NET"),
    	TEXT("Stat NETPkt"),
    	TEXT("Stat NETPktLoss"),
    	TEXT("Stat NetTraffic"),
    	TEXT("Stat Engine"),
    	TEXT("Stat Timecode"),
    	TEXT("Stat FileIO"),
    	TEXT("Stat Tasks"),
    	TEXT("Stat TaskGraphTasks"),
    	TEXT("Stat TaskGraphWorkers"),
    	TEXT("Stat AsyncLoading"),
    	TEXT("Stat Threads"),
    	TEXT("Stat VirtualTexture"),
    	TEXT("Stat VirtualTextureMemory"),
    	TEXT("Stat D3D12RHI"),
    	TEXT("Stat VulkanRHI"),
    	TEXT("Stat PSO"),
    	TEXT("Stat GPUFrameTime"),
    	TEXT("Stat GPUStats"),
    	TEXT("Stat STATS"),
    };
    const FCheckBoxStyle* CheckBoxStyle = &FSlateStyleRegistry::FindSlateStyle("CheckBoxCustomStyle")
    ->GetWidgetStyle<FCheckBoxStyle>("CheckBoxCustomStyle");

    TSharedPtr<SWidget> StatCommandGridWidget = SNew(SStatCommandGrid)
                .StatCommands(StatCommands)
                .Font(FontInfo)
                .CheckBoxStyle(CheckBoxStyle);

    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    ToolbarExtender->AddToolBarExtension(
        "Play", EExtensionHook::After, nullptr,
        FToolBarExtensionDelegate::CreateLambda([StatCommandGridWidget](FToolBarBuilder &Builder) 
            {
              Builder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([]() 
                  {
                      FGlobalTabmanager::Get()->TryInvokeTab(FName("CustomGraphPanel"));
                  })),
                  NAME_None, LOCTEXT("OpenGraphPanel", "Open Graph"),
                  LOCTEXT("OpenGraphPanelTooltip", "Opens the custom graph panel"),
                  FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

              Builder.AddComboButton(
                  FUIAction(),
                  FOnGetContent::CreateLambda([StatCommandGridWidget]()
                      {
                          return SNew(SBox)
                                 [
                                     StatCommandGridWidget.ToSharedRef()
                                 ];
                      }),

                  LOCTEXT("CustomComboButton", "Debug"),
                  LOCTEXT("CustomComboTooltip", "Shows grid of buttons"),
                  FSlateIcon(FAppStyle::GetAppStyleSetName(), "PlayWorld.RepeatLastLaunch"),
                  true
              );
        }));

    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
}

void FConsoleCommandCenterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FConsoleCommandCenterModule, ConsoleCommandCenter)