// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define LOCTEXT_NAMESPACE "AssetCleaner"

/**
 * A Slate widget container that displays a list of named filters as toggleable UI elements.
 * Notifies listeners when individual filter states change.
 */
class SFilterContainerWidget final : public SCompoundWidget
{
	/** Delegate that is triggered when the state of a filter changes.
	 * @param FilterName - The name of the filter that changed.
	 * @param bIsEnabled - The new enabled state of the filter.
	 */	
	DECLARE_DELEGATE_TwoParams(FOnFilterChanged, const FString& /*FilterName*/, bool /*bIsEnabled*/);
public:
	/**
	 * Represents metadata for a named filter, including its display name and tooltip text.
	 */
	struct FNamedFilterData
	{
		/** Display name of the filter */
		FString FilterName;

		/** Tooltip text associated with the filter */
		FString ToolTipText;

		FNamedFilterData(const FString& InName, const FString& InToolTipText)
			: FilterName(InName), ToolTipText(InToolTipText)
		{
		}
	};

	SLATE_BEGIN_ARGS(SFilterContainerWidget) {}
		/** List of filters to display */
		SLATE_ARGUMENT(TArray<FNamedFilterData>, FilterList)
		/** Delegate called when a filter is toggled */
		SLATE_EVENT(FOnFilterChanged, OnFilterChanged)
	SLATE_END_ARGS()

	/** Constructs the widget with the specified arguments */
	void Construct(const FArguments& InArgs);

	/** Delegate that is triggered when any filter is toggled */
	FOnFilterChanged OnFilterChangedDelegate;
protected:
	/**
	 * A UI element representing a single filter, with support for enabling/disabling and context menu.
	 */
	class FCustomFilter : public SCompoundWidget
	{
		/** Delegate that is triggered to disable all filters */
		DECLARE_DELEGATE(FOnRequestDisableAll);

	public:
		SLATE_BEGIN_ARGS(FCustomFilter) {}
			/** Name of the filter this widget represents */
			SLATE_ARGUMENT(FString, FilterName)

			/** Tooltip text for the filter */
			SLATE_ARGUMENT(FString, ToolTipText)

			/** Delegate called when this filter is toggled */
			SLATE_EVENT(FOnFilterChanged, OnFilterChanged)
		SLATE_END_ARGS()

		/** Constructs the filter widget with the given arguments */
		void Construct(const FArguments& InArgs);

	protected:

		/** Initializes internal state and builds UI */
		void Construct_Internal(const FString& FilterName);

		/** Returns true if this filter is currently enabled */
		FORCEINLINE bool IsEnabled() const 
		{
			return bEnabled;
		}

		/** Returns the display name of the filter */
		FORCEINLINE FString GetFilterDisplayName() const
		{
			return FilterDispayName;
		}

		/** Returns the tooltip text of the filter */
		FORCEINLINE FString GetToolTipText() const
		{
			return ToolTipText;
		}

		/** Sets the tooltip text for the filter */
		FORCEINLINE void SetToolTipText(const FString& InToolTipText)
		{
			ToolTipText = InToolTipText;
		}

		/** Returns the padding of the filter name text based on button press state */
		FORCEINLINE FMargin GetFilterNamePadding() const
		{
			return ToggleButtonPtr->IsPressed() ? FMargin(4, 2, 4, 0) : FMargin(4, 1, 4, 1);
		}

		/** Returns the current checked state of the filter's checkbox */
		FORCEINLINE ECheckBoxState IsChecked() const
		{
			return bEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		
		}

		/** Called when the checkbox is toggled */
		FORCEINLINE void FilterToggled(ECheckBoxState NewState)
		{
			bEnabled = NewState == ECheckBoxState::Checked;
			OnFilterChanged.ExecuteIfBound(FilterDispayName, bEnabled);
		}

		/** Returns the image color based on whether the filter is enabled */
		FORCEINLINE FSlateColor GetFilterImageColorAndOpacity() const
		{
			return bEnabled ? FSlateColor(FColor::White) : FAppStyle::Get().GetSlateColor("Colors.Recessed");
		}

		/** Builds the right-click context menu for this filter */
		TSharedRef<SWidget> GetRightClickMenuContent()
		{
			FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, NULL);

			MenuBuilder.BeginSection("FilterOptions", LOCTEXT("FilterContextHeading", "Filter Options"));
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("DisableAllFilters", "Disable All Filters"),
					LOCTEXT("DisableAllFiltersTooltip", "Disables all active filters."),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &FCustomFilter::DisableAllFilters)));
			}
			MenuBuilder.EndSection();

			return MenuBuilder.MakeWidget();
		}

		/** Disables all filters by triggering the corresponding delegate */
		FORCEINLINE void DisableAllFilters()
		{
			OnRequestDisableAll.ExecuteIfBound();
		}

	private:
		/** Delegate called when this filter is toggled */
		FOnFilterChanged OnFilterChanged;

		/** Delegate called to request all filters be disabled */
		FOnRequestDisableAll OnRequestDisableAll;

		/** The checkbox used to enable/disable this filter */
		TSharedPtr<SCheckBox> ToggleButtonPtr;

		/** Whether the filter is currently enabled */
		bool bEnabled;

		/** Display name of the filter */
		FString FilterDispayName;

		/** Tooltip text for the filter */
		FString ToolTipText;
	};

private:
	/** Rebuilds the UI for all filters */
	void RebuildFilters();

	/** The scroll box containing all filter widgets */
	TSharedPtr<SScrollBox> ScrollBox;

	/** List of filters to construct widgets from */
	TArray<FNamedFilterData> FilterList;

	/** Constructed filter widgets corresponding to the filter list */
	TArray<TSharedPtr<SCompoundWidget>> FilterWidgets;
};

#undef LOCTEXT_NAMESPACE