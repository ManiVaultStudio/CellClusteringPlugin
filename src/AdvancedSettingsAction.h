#pragma once

#include <cstdint>

#include "actions/DecimalAction.h"
#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/ToggleAction.h"

#include <cellclustering/options.hpp>

using namespace mv::gui;


/// ///////////////// ///
/// Advanced Settings ///
/// ///////////////// ///

class AdvancedSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    AdvancedSettingsAction(QObject* parent = nullptr);

public: // Settings getters
    cellclustering::LouvainParams getLouvainParams() const;
    int getNJobs() const
    {
        return _multithreadActions.isChecked() ? -1 : 1;
    }
    uint64_t getSeed() const
    {
        return static_cast<uint64_t>(_seed.getValue());
    }

public: // Action getters

    ToggleAction& getMultithreadAction() { return _multithreadActions; }
    ToggleAction& getDirectedAction() { return _directed; }
    ToggleAction& getPruneAction() { return _prune; }
    ToggleAction& getJaccardAction() { return _jaccard; }
    IntegralAction& getSeedAction() { return _seed; }

    DecimalAction& getQTolAction() { return _qTol; }
    DecimalAction& getTimeLimitAction() { return _timeLimit; }
    IntegralAction& getMaxRunsAction() { return _maxRuns; }
    IntegralAction& getPatienceAction() { return _patience; }

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
    ToggleAction            _multithreadActions;
    ToggleAction            _directed;
    ToggleAction            _prune;
    ToggleAction            _jaccard;
    IntegralAction          _seed;

    // Louvain
    DecimalAction           _qTol;
    DecimalAction           _timeLimit;
    IntegralAction          _maxRuns;
    IntegralAction          _patience;
};
