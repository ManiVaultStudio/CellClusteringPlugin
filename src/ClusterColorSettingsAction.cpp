#include "ClusterColorSettingsAction.h"

ClusterColorSettingsAction::ClusterColorSettingsAction(QObject* parent) :
    GroupAction(parent, "ClusterColorSettingsAction", true),
    _colorByAction(this, "Color by", ColorByNames, ColorByNames[1]),
    _colorMapAction(this, "Color map"),
    _randomColorSeedAction(this, "Color candom seed"),
    _updateColorsManuallyAction(this, "Update colors manually", false),
    _applyColorsAction(this, "Update colors")
{
    setText("Cluster Color Settings");
    setSerializationName("Cluster Color Settings");

    GroupAction::addAction(&_colorByAction);
    GroupAction::addAction(&_colorMapAction);
    GroupAction::addAction(&_randomColorSeedAction);
    GroupAction::addAction(&_updateColorsManuallyAction);
    GroupAction::addAction(&_applyColorsAction);
}


void ClusterColorSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _colorByAction.fromParentVariantMap(variantMap);
    _colorMapAction.fromParentVariantMap(variantMap);
    _randomColorSeedAction.fromParentVariantMap(variantMap);
    _updateColorsManuallyAction.fromParentVariantMap(variantMap);
    _applyColorsAction.fromParentVariantMap(variantMap);
}

QVariantMap ClusterColorSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _colorByAction.insertIntoVariantMap(variantMap);
    _colorMapAction.insertIntoVariantMap(variantMap);
    _randomColorSeedAction.insertIntoVariantMap(variantMap);
    _updateColorsManuallyAction.insertIntoVariantMap(variantMap);
    _applyColorsAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
