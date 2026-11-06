#include "AdvancedSettingsAction.h"

AdvancedSettingsAction::AdvancedSettingsAction(QObject* parent) :
    GroupAction(parent, "Advanced Settings"),
    _multithreadActions(this, "Multithreading", true),
    _seed(this, "seed"),
    _directed(this, "directed", false),
    _prune(this, "prune", false),
    _jaccard(this, "Jaccard", true),
    _qTol(this, "qTol"),
    _timeLimit(this, "time limit"),
    _maxRuns(this, "max runs"),
    _patience(this, "patience")
{
    setText("Advanced Settings");
    setSerializationName("Advanced Settings");

    _seed.setToolTip("Seed to use for the Mersenne Twister when sampling negative observations");

    GroupAction::addAction(&_directed);
    GroupAction::addAction(&_prune);
    GroupAction::addAction(&_jaccard);
    GroupAction::addAction(&_qTol);
    GroupAction::addAction(&_timeLimit);
    GroupAction::addAction(&_maxRuns);
    GroupAction::addAction(&_patience);
    GroupAction::addAction(&_seed);
    GroupAction::addAction(&_multithreadActions);

    _qTol.initialize(0, 10, 1e-3f, 4);
    _timeLimit.initialize(1, 4000, 2000, 1);
    _maxRuns.initialize(1, 1000, 100);
    _patience.initialize(1, 100, 20);
    _seed.initialize(1, 200000000, 42);

    _multithreadActions.setToolTip("Use more memory to increase computation speed\nThis option is only useful if you have many cores\n4 is not a lot.");

    // Close the advanced settings by default
    collapse();

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _directed.setEnabled(enable);
        _prune.setEnabled(enable);
        _jaccard.setEnabled(enable);
        _qTol.setEnabled(enable);
        _timeLimit.setEnabled(enable);
        _maxRuns.setEnabled(enable);
        _patience.setEnabled(enable);
        _multithreadActions.setEnabled(enable);
        _seed.setEnabled(enable);
        };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
        });

    updateReadOnly();
}

cellclustering::LouvainParams AdvancedSettingsAction::getLouvainParams() const
{
    return {
        .qTol = _qTol.getValue(),
        .timeLimit = _timeLimit.getValue(),
        .maxRuns = _maxRuns.getValue(),
        .patience = _patience.getValue(),
    };
}

void AdvancedSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _directed.fromParentVariantMap(variantMap);
    _prune.fromParentVariantMap(variantMap);
    _jaccard.fromParentVariantMap(variantMap);
    _qTol.fromParentVariantMap(variantMap);
    _timeLimit.fromParentVariantMap(variantMap);
    _maxRuns.fromParentVariantMap(variantMap);
    _patience.fromParentVariantMap(variantMap);
    _multithreadActions.fromParentVariantMap(variantMap);
    _seed.fromParentVariantMap(variantMap);
}

QVariantMap AdvancedSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _directed.insertIntoVariantMap(variantMap);
    _prune.insertIntoVariantMap(variantMap);
    _jaccard.insertIntoVariantMap(variantMap);
    _qTol.insertIntoVariantMap(variantMap);
    _timeLimit.insertIntoVariantMap(variantMap);
    _maxRuns.insertIntoVariantMap(variantMap);
    _patience.insertIntoVariantMap(variantMap);
    _multithreadActions.insertIntoVariantMap(variantMap);
    _seed.insertIntoVariantMap(variantMap);

    return variantMap;
}
