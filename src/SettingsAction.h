#pragma once

#include "actions/GroupAction.h"
#include "actions/HorizontalGroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/OptionAction.h"
#include "actions/TriggerAction.h"

#include <cellclustering/options.hpp>

using namespace mv::gui;

/// ////////////////// ///
/// Start Stop Buttons ///
/// ////////////////// ///

class ButtonsGroupAction : public HorizontalGroupAction
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    ButtonsGroupAction(QObject* parent);

    void changeEnabled(bool enable)
    {
        _startComputationAction.setEnabled(enable);
        _stopComputationAction.setEnabled(!enable);
    }

    void setStarted()
    {
        changeEnabled(false);
    }

    void setFinished()
    {
        changeEnabled(true);
    }

public: // Action getters

    TriggerAction& getStartComputationAction() { return _startComputationAction; }
    TriggerAction& getStopComputationAction() { return _stopComputationAction; }

protected:
    TriggerAction   _startComputationAction;        /** Start computation action */
    TriggerAction   _stopComputationAction;         /** Stop computation action */
};

/**
 * SettingsAction
 * 
 * Class that houses general settings for the UMAP analysis plugin
 *
 */
class SettingsAction : public GroupAction
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
    SettingsAction(QObject* parent = nullptr);

    void changeEnabled(bool enable)
    {
        _clusteringAlgorithmAction.setEnabled(enable);
        _minClusterSizeAction.setEnabled(enable);
        _knnAlgorithmAction.setEnabled(enable);
        _knnMetricAction.setEnabled(enable);
        _knnKAction.setEnabled(enable);

    }

    void setStarted()
    {
        _startStopActions.setStarted();
        changeEnabled(false);
    }

    void setFinished()
    {
        _startStopActions.setFinished();
        changeEnabled(true);
    }

public: // Settings getters
    cellclustering::ClusteringAlgo getClusteringAlgo() const;
    cellclustering::NNMethod getKnnAlgorithm() const;
    cellclustering::Metric getKnnMetric() const;
    size_t getMinClusterSize() const
    {
        return static_cast<size_t>(_minClusterSizeAction.getValue());
    }

public: // Action getters

    ButtonsGroupAction& getStartStopAction() { return _startStopActions; }
    TriggerAction& getStartAction() { return _startStopActions.getStartComputationAction(); }
    TriggerAction& getStopAction() { return _startStopActions.getStopComputationAction(); }
    OptionAction& getClusteringAlgorithmAction() { return _clusteringAlgorithmAction; }
    OptionAction& getknnAlgorithmAction() { return _knnAlgorithmAction; }
    OptionAction& getknnMetricAction() { return _knnMetricAction; }
    IntegralAction& getMinClusterSizeAction() { return _minClusterSizeAction; }
    IntegralAction& getKnnKAction() { return _knnKAction; }

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
    ButtonsGroupAction  _startStopActions;              /** Buttons that control start and top of computation */
    OptionAction        _knnAlgorithmAction;            /** knn algorithm */
    OptionAction        _knnMetricAction;               /** knn metric */
    IntegralAction      _knnKAction;                    /** number of knn */
    OptionAction        _clusteringAlgorithmAction;     /** Community detection algorithm */
    IntegralAction      _minClusterSizeAction;          /** clusters with size <= this get label std::numeric_limits<size_t>::max */
};
