// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateBrush.h"
#include "Kismet/KismetSystemLibrary.h"

class CONSOLECOMMANDCENTER_API SStatCommandGrid : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SStatCommandGrid) {}
        SLATE_ARGUMENT(TArray<FString>, StatCommands)
        SLATE_ARGUMENT(FSlateFontInfo, Font)
        SLATE_ARGUMENT(const FCheckBoxStyle*, CheckBoxStyle)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        Commands = InArgs._StatCommands;
        Font = InArgs._Font;
        Style = InArgs._CheckBoxStyle;

        ChildSlot
            [
                SNew(SBorder)
                    .Padding(5)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    .BorderBackgroundColor(FColor::White)
                    [
                        SNew(SVerticalBox)
                            + SVerticalBox::Slot()
                            .FillHeight(1.0f)
                            .VAlign(VAlign_Center)
                            .HAlign(HAlign_Center)
                            [
                                SNew(STextBlock)
                                    .Font(Font)
                                    .Text(FText::FromString(TEXT("Statistics")))
                            ]
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            [
                                GenerateCommandGrid()
                            ]
                    ]
            ];
    }

private:
    TArray<FString> Commands;
    FSlateFontInfo Font;
    const FCheckBoxStyle* Style;

    TSharedRef<SWidget> GenerateCommandGrid()
    {
        const int32 NumCols = 2;
        TSharedRef<SUniformGridPanel> Grid =
            SNew(SUniformGridPanel).SlotPadding(4.0f);

        for(int32 Index = 0; Index < Commands.Num(); ++Index) {
            int32 Row = Index / NumCols;
            int32 Col = Index % NumCols;

            const FString& Command = Commands[Index];

            Grid->AddSlot(Col, Row)
                [
                    SNew(SBox)
                        .WidthOverride(140.f)
                        .HeightOverride(20.f)
                        .VAlign(VAlign_Fill)
                        .HAlign(HAlign_Fill)
                        [
                            SNew(SCheckBox)
                                .Style(Style)
                                .OnCheckStateChanged(this, &SStatCommandGrid::OnStatCheckboxChanged, Command)
                                [
                                    SNew(SBorder)
                                        .HAlign(HAlign_Center)
                                        .VAlign(VAlign_Center)
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString(Command))
                                                .ColorAndOpacity(FColor::Black)
                                        ]
                                ]
                        ]
                ];
        }

        return Grid;
    }
    void OnStatCheckboxChanged(ECheckBoxState NewState, FString Command) {
        UWorld* World = nullptr;

#if WITH_EDITOR
        for(const FWorldContext& Context : GEngine->GetWorldContexts()) {
            if(Context.WorldType == EWorldType::PIE ||
                Context.WorldType == EWorldType::Editor) {
                World = Context.World();
                break;
            }
        }
#else
        World = GEngine->GetCurrentPlayWorld();
#endif

        if(!World) {
            return;
        }

        APlayerController* PC = World->GetFirstPlayerController();
        UKismetSystemLibrary::ExecuteConsoleCommand(World, Command, PC);
    }
};