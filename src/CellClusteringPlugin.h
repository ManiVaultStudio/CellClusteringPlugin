/**
 * Cell clustering plugin 
 *
 * This analysis plugin class provides acces to the cell clustering algorithms 
 * implemented in https://github.com/alxvth/CellClustering as an analysis plugin in ManiVault
 *
 * @authors A.Vieth
 */
#pragma once

#include <AnalysisPlugin.h>

#include <PointData/PointData.h>

#include "AdvancedSettingsAction.h"
#include "ClusterColorSettingsAction.h"
#include "KnnSettingsAction.h"
#include "SettingsAction.h"

#include <QThread>
#include <QPointer>

#include <cstdint>
#include <memory>

using namespace mv::plugin;
using namespace mv;

namespace mv::util {
    class MarkdownDialog;
}

// =============================================================================
// Worker
// =============================================================================

/**
 * Runs CellClustering computation in non-UI thread
 */
class CellClusteringWorker : public QObject
{
    Q_OBJECT

public:
    CellClusteringWorker(const Dataset<Points>& inputPoints, DatasetTask* parentTask, SettingsAction* settings, 
        KnnSettingsAction* knnSettings, AdvancedSettingsAction* advSettings, ClusterColorSettingsAction* colSettingsAction);

    void changeThread(QThread* targetThread);

private:
    void resetThread();

public slots:
    void compute();

signals:
    void updateClusters(std::shared_ptr < std::vector<size_t>> communityIDs);
    void finished();

private:
    Dataset<Points>             _inputDataset;

    SettingsAction*             _settingsAction;            /** General settings */
    KnnSettingsAction*          _knnSettingsAction;         /** knn settings */
    AdvancedSettingsAction*     _advSettingsAction;         /** Advanced settings */
    ClusterColorSettingsAction* _colSettingsAction;         /** Color settings */
    DatasetTask*                _parentTask;
};

// =============================================================================
// Plugin
// =============================================================================

class CellClusteringAnalysisPlugin : public AnalysisPlugin
{
    Q_OBJECT

public:

    CellClusteringAnalysisPlugin(const PluginFactory* factory);
    ~CellClusteringAnalysisPlugin() override;

    void init() override;

private:
    void updateClusters(std::shared_ptr < std::vector<size_t>> communityIDs);
    void updateClusterColors();

    void deleteWorker();

signals:
    void startWorker();
    void stopWorker();

public: // Serialization

    /**
     * Load plugin from variant map
     * @param variantMap map representation of the plugin
     */
    Q_INVOKABLE void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    Q_INVOKABLE QVariantMap toVariantMap() const override;

private:
    SettingsAction              _settingsAction;            /** General settings */
    KnnSettingsAction           _knnSettingsAction;         /** knn settings */
    AdvancedSettingsAction      _advSettingsAction;         /** Advanced settings */
    ClusterColorSettingsAction  _colSettingsAction;         /** Color settings */

    QThread                     _workerThread;
    CellClusteringWorker*       _cellClusteringWorker;
};


// =============================================================================
// Factory
// =============================================================================

class CellClusteringAnalysisPluginFactory : public AnalysisPluginFactory
{
    Q_INTERFACES(mv::plugin::AnalysisPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "studio.manivault.CellClusteringAnalysisPlugin"
                      FILE  "PluginInfo.json")

public:

    CellClusteringAnalysisPluginFactory();

    ~CellClusteringAnalysisPluginFactory() override {}

    /**
     * Get the read me markdown file URL
     * @return URL of the read me markdown file
     */
    QUrl getReadmeMarkdownUrl() const override;

    bool hasHelp() const override { return true; }

    /**
     * Get the URL of the GitHub repository
     * @return URL of the GitHub repository
     */
    QUrl getRepositoryUrl() const override;

    /** Creates an instance of the CellClustering analysis plugin */
    AnalysisPlugin* produce() override;

    /** Returns the data types that are supported by the CellClustering analysis plugin */
    mv::DataTypes supportedDataTypes() const override;

    /** Enable right-click on data set to open analysis */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;

private:
    QPointer<util::MarkdownDialog>   _helpMarkdownDialog = {};
};
