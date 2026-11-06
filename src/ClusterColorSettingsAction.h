#pragma once

#include <actions/ColorMap1DAction.h>
#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/OptionAction.h"
#include "actions/TriggerAction.h"

using namespace mv::gui;

/**
 * ClusterColorSettingsAction
 * 
 * Class that houses general settings for the UMAP analysis plugin
 *
 */
class ClusterColorSettingsAction : public GroupAction
{

public:
    /** Color options */
    enum class ColorBy : std::uint8_t {
        PseudoRandomColors,     /** Use pseudo-random colors */
        ColorMap                /** Color by continuous color map */
    };

    const QStringList ColorByNames = {
        "Pseudo-random colors",
        "Color map"
    };

public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    ClusterColorSettingsAction(QObject* parent = nullptr);

public: // Action getters

    OptionAction& getColorByAction() { return _colorByAction; }
    ColorMap1DAction& getColorMapAction() { return _colorMapAction; }
    IntegralAction& getRandomColorSeedAction() { return _randomColorSeedAction; }
    ToggleAction& getUpdateColorsManuallyAction() { return _updateColorsManuallyAction; }

public: // Serialization

    /**
     * Load plugin from variant map
     * @param variantMap Variant map representation of the plugin
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    QVariantMap toVariantMap() const override;

protected:
    ToggleAction               _updateColorsManuallyAction;    /** Update colors manually action */
    IntegralAction             _randomColorSeedAction;         /** Random seed action (for coloring) */
    OptionAction               _colorByAction;                 /** Color by options action */
    ColorMap1DAction           _colorMapAction;                /** Color map action */
    TriggerAction              _applyColorsAction;             /** Apply colors action */
};
