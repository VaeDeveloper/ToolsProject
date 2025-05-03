// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * SDeveloperSettingsWidget
 *
 * A detail customization class used to customize the appearance and behavior
 * of UDataAssetManagerSettings in the Editor's details panel.
 *
 * This customization is registered in the PropertyEditorModule and is shown
 * when editing UDataAssetManagerSettings in Project Settings or Plugin Settings.
 */
class DATAASSETMANAGER_API SDeveloperSettingsWidget : public IDetailCustomization
{
public:
    /**
     * Creates an instance of the detail customization.
     *
     * @return A shared reference to the new detail customization instance.
     */
    static TSharedRef<IDetailCustomization> MakeInstance();
   
    /**
     * Called to customize the details panel layout.
     *
     * @param DetailBuilder The detail layout builder used to customize the panel.
     */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
