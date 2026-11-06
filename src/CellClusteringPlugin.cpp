#include "CellClusteringPlugin.h"

#include <widgets/MarkdownDialog.h>
#include <util/Icon.h>
#include <util/Serialization.h>

#include <PointData/DimensionsPickerAction.h>
#include <PointData/InfoAction.h>
#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>

#include <cellclustering/cellclustering.hpp>

#include <QDebug>
#include <QtCore>

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <unordered_set>
#include <vector>

Q_PLUGIN_METADATA(IID "studio.manivault.CellClusteringAnalysisPlugin")

using namespace mv;
using namespace mv::plugin;

/// ////////// ///
///   Helper   ///
/// ////////// ///

namespace
{
    // modular helper to extract data from core
    std::tuple< std::vector<float>, std::vector<unsigned int>, size_t, size_t> extractEnabledDimensions(Dataset<Points>& dataset) {
        // Create list of data from the enabled dimensions
        std::vector<float> data;
        std::vector<unsigned int> indices;

        // Extract the enabled dimensions from the data
        const std::vector<bool> enabledDimensions = dataset->getDimensionsPickerAction().getEnabledDimensions();
        const auto numEnabledDimensions = static_cast<size_t>(std::ranges::count_if(enabledDimensions, [](const bool b) { return b; }));

        const size_t numPoints = dataset->isFull() ? dataset->getNumPoints() : dataset->indices.size();
        data.resize(numPoints * numEnabledDimensions);

        for (int i = 0; i < dataset->getNumDimensions(); i++) {
            if (enabledDimensions[i]) {
                indices.push_back(i);
            }
        }

        dataset->populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, indices);

        return { data, indices, numEnabledDimensions, numPoints };
    }

}

// =============================================================================
// Plugin
// =============================================================================

CellClusteringAnalysisPlugin::CellClusteringAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _settingsAction(this),
    _knnSettingsAction(this),
    _advSettingsAction(this),
    _colSettingsAction(this),
    _workerThread(nullptr),
    _cellClusteringWorker(nullptr)
{
}

CellClusteringAnalysisPlugin::~CellClusteringAnalysisPlugin()
{
    _workerThread.quit();           // Signal the thread to quit gracefully
    if (!_workerThread.wait(500))   // Wait for the thread to actually finish
        _workerThread.terminate();  // Terminate thread after 0.5 seconds

    deleteWorker();
}

void CellClusteringAnalysisPlugin::deleteWorker()
{
    if (_cellClusteringWorker)
    {
        _cellClusteringWorker->changeThread(QThread::currentThread());
        delete _cellClusteringWorker;
    }
}

void CellClusteringAnalysisPlugin::init()
{

    // Create clusters output dataset
    if (!outputDataInit())
    {
        setOutputDataset(mv::data().createDataset("Cluster", "Clusters", getInputDataset()));
    }
    
    const Dataset<Points> inputDataset = getInputDataset<Points>();
    Dataset<Clusters> ouputDataset = getOutputDataset<Clusters>();

    // Add settings to UI
    ouputDataset->addAction(_settingsAction);
    ouputDataset->addAction(_colSettingsAction);
    ouputDataset->addAction(_knnSettingsAction);
    ouputDataset->addAction(_advSettingsAction);
    
    auto dimensionsGroupAction = new GroupAction(this, "Dimensions", true);
    dimensionsGroupAction->addAction(&inputDataset->getFullDataset<Points>()->getDimensionsPickerAction());
    dimensionsGroupAction->setText(QString("Input dimensions (%1)").arg(inputDataset->getFullDataset<Points>()->text()));
    dimensionsGroupAction->setShowLabels(false);

    ouputDataset->addAction(*dimensionsGroupAction);

    // Automatically focus on the CellClustering data set
    ouputDataset->getDataHierarchyItem().select();
    ouputDataset->_infoAction->collapse();

    // Create CellClustering worker, which will be executed in another thread
    // Start the analysis when the user clicks the start analysis push button
    connect(&_settingsAction.getStartAction(), &mv::gui::TriggerAction::triggered, this, [this] {

        // Disable actions during analysis
        _settingsAction.setStarted();
        _knnSettingsAction.setReadOnly(true);

        deleteWorker();

        getOutputDataset()->getTask().setRunning();

        _cellClusteringWorker = new CellClusteringWorker(getInputDataset<Points>(), &getOutputDataset()->getTask(), &_settingsAction, &_knnSettingsAction, &_advSettingsAction, &_colSettingsAction);
        _cellClusteringWorker->changeThread(&_workerThread);

        // To-Worker signals
        connect(this, &CellClusteringAnalysisPlugin::startWorker, _cellClusteringWorker, &CellClusteringWorker::compute);

        // From-Worker signals
        connect(_cellClusteringWorker, &CellClusteringWorker::updateClusters, this, &CellClusteringAnalysisPlugin::updateClusters);

        connect(_cellClusteringWorker, &CellClusteringWorker::finished, this, [this]() {
            _settingsAction.setFinished();
            _knnSettingsAction.setReadOnly(false);
            getOutputDataset()->getTask().setFinished();
            });

        _workerThread.start();
        emit startWorker();
        });

    connect(&_settingsAction.getStopAction(), &mv::gui::TriggerAction::triggered, this, [this](bool checked) {
        emit stopWorker();
        }, 
        Qt::DirectConnection);

}

namespace
{
    struct Counts {
        std::size_t unique;
        std::size_t max;
    };

    Counts countUnique(const std::vector<std::size_t>& values)
    {
        constexpr auto maxValue = std::numeric_limits<std::size_t>::max();

        std::unordered_set<std::size_t> uniqueValues;
        std::size_t maxCount = 0;

        for (const auto value : values)
        {
            if (value == maxValue)
            {
                ++maxCount;
            }
            else
            {
                uniqueValues.insert(value);
            }
        }

        return { uniqueValues.size(), maxCount };
    }
}

void CellClusteringAnalysisPlugin::updateClusters(std::shared_ptr<std::vector<size_t>> communityIDs)
{
    std::vector<size_t>& clusterIDs = *communityIDs;
    const auto [uniqueClusters, invalidClusters] = countUnique(clusterIDs);

    // better convert std::numeric_limits<size_t>::max() into it's own
    auto numClusters = uniqueClusters + invalidClusters;

    // Replace invalid cluster values with their own ID
    if (invalidClusters > 0)
    {
        std::size_t nextValue = uniqueClusters + 1;

        for (auto& clusterID : clusterIDs)
        {
            if (clusterID == std::numeric_limits<std::size_t>::max())
            {
                clusterID = nextValue++;
            }
        }
    }

    // Assign cluster IDs to point IDs
    std::vector<std::vector<std::uint32_t>> clusters(numClusters);

    std::vector<std::uint32_t> counts(numClusters, 0);
    for (const auto& clusterID: clusterIDs)
        ++counts[clusterID];

    for (size_t c = 0; c < numClusters; ++c)
        clusters[c].reserve(counts[c]);

    for (std::uint32_t i = 0; i < clusterIDs.size(); ++i)
        clusters[clusterIDs[i]].push_back(i);

    auto inputDataset = getInputDataset<Points>();
    auto outputDataset = getOutputDataset<Clusters>();

    // Remove existing clusters
    outputDataset->getClusters().clear();

    // Add found clusters
    std::vector<std::uint32_t> globalIndices;
    inputDataset->getSourceDataset<Points>()->getGlobalIndices(globalIndices);

    std::int32_t clusterIndex = 1;
    for (auto& clusterIndicesLocal : clusters)
    {
        std::vector<std::uint32_t> clusterIndicesGlobal;
        clusterIndicesGlobal.reserve(clusterIndicesLocal.size());

        for (auto clusterIndexLocal : clusterIndicesLocal)
            clusterIndicesGlobal.push_back(globalIndices[clusterIndexLocal]);

        Cluster cluster = {
            /* name  */   QString("cluster %1").arg(QString::number(clusterIndex)),
            /* color */   Qt::gray, // will be set in updateClusterColors
            /* indices */ clusterIndicesGlobal,
        };

        outputDataset->addCluster(cluster);

        clusterIndex++;
    }

    updateClusterColors();
}

void CellClusteringAnalysisPlugin::updateClusterColors()
{
    switch (static_cast<SettingsAction::ColorBy>(_colSettingsAction.getColorByAction().getCurrentIndex())) {
    case SettingsAction::ColorBy::PseudoRandomColors:
        Cluster::colorizeClusters(
            getOutputDataset<Clusters>()->getClusters(),
            _colSettingsAction.getRandomColorSeedAction().getValue());
        break;

    case SettingsAction::ColorBy::ColorMap:
        Cluster::colorizeClusters(
            getOutputDataset<Clusters>()->getClusters(),
            _colSettingsAction.getColorMapAction().getColorMapImage());
        break;
    }

    events().notifyDatasetDataChanged(getOutputDataset());
}

void CellClusteringAnalysisPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    AnalysisPlugin::fromVariantMap(variantMap);

    mv::util::variantMapMustContain(variantMap, "CellClustering Settings");
    mv::util::variantMapMustContain(variantMap, "Knn Settings");
    mv::util::variantMapMustContain(variantMap, "Advanced Settings");

    _settingsAction.fromParentVariantMap(variantMap);
    _knnSettingsAction.fromParentVariantMap(variantMap);
    _advSettingsAction.fromParentVariantMap(variantMap);
}

QVariantMap CellClusteringAnalysisPlugin::toVariantMap() const
{
    QVariantMap variantMap = AnalysisPlugin::toVariantMap();

    _settingsAction.insertIntoVariantMap(variantMap);
    _knnSettingsAction.insertIntoVariantMap(variantMap);
    _advSettingsAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

// =============================================================================
// Worker
// =============================================================================

CellClusteringWorker::CellClusteringWorker(const Dataset<Points>& inputPoints, DatasetTask* parentTask, SettingsAction* settings, 
    KnnSettingsAction* knnSettings, AdvancedSettingsAction* advSettings, ClusterColorSettingsAction* colSettingsAction):
    _inputDataset(inputPoints),
    _settingsAction(settings),
    _knnSettingsAction(knnSettings),
    _advSettingsAction(advSettings),
    _colSettingsAction(colSettingsAction),
    _parentTask(parentTask)
{
}

void CellClusteringWorker::changeThread(QThread* targetThread)
{
    this->moveToThread(targetThread);
}

void CellClusteringWorker::resetThread()
{
    changeThread(QCoreApplication::instance()->thread());
}

void CellClusteringWorker::compute()
{
    _parentTask->setRunning();
    _parentTask->setProgress(0.1f, "Phenograph: kNN");

    QCoreApplication::processEvents();

    // get data
    auto [data, indices, numEnabledDimensions, numPoints] = extractEnabledDimensions(_inputDataset);

    cellclustering::Options opt;

    // ---- Community detection ----
    opt.clusteringAlgo = _settingsAction->getClusteringAlgo();
    opt.louvainParams = _advSettingsAction->getLouvainParams();
    opt.directed = _advSettingsAction->getDirectedAction().isChecked();
    opt.prune = _advSettingsAction->getPruneAction().isChecked();
    opt.minClusterSize = _settingsAction->getMinClusterSize();
    opt.jaccard = _advSettingsAction->getJaccardAction().isChecked();
    opt.returnGraph = false;

    // ---- knn ----
    opt.k = _settingsAction->getKnnKAction().getValue();
    opt.nnMetric = _settingsAction->getKnnMetric();
    opt.nnMethod = _settingsAction->getKnnAlgorithm();
    opt.hnsw = _knnSettingsAction->getHnswParams();
    opt.annoy = _knnSettingsAction->getAnnoyParams();
    opt.kmknn = _knnSettingsAction->getKMKNNParams();

    // ---- extras ----
    opt.seed = _advSettingsAction->getSeed();
    opt.verbose = true;
    opt.nJobs = _advSettingsAction->getNJobs();

    const auto dataMatrix = cellclustering::DenseMatrixView(data, numPoints, numEnabledDimensions);

    const auto knn = cellclustering::findNeighbors(dataMatrix, opt);

    _parentTask->setProgress(0.3f, "Phenograph: Louvain...");

    auto [communities, _, Q] = cellclustering::phenograph(knn, opt);

    auto communitiesShared = std::make_shared<std::vector<size_t>>(std::move(communities));
    emit updateClusters(communitiesShared);

    _parentTask->setFinished();
    emit finished();
    resetThread();
}

// =============================================================================
// Factory
// =============================================================================

CellClusteringAnalysisPluginFactory::CellClusteringAnalysisPluginFactory()
{
    setIcon(mv::util::StyledIcon(createPluginIcon("PHCL")));

    connect(&getPluginMetadata().getTriggerHelpAction(), &TriggerAction::triggered, this, [this]() -> void {
        if (!getReadmeMarkdownUrl().isValid() || _helpMarkdownDialog.get())
            return;

        _helpMarkdownDialog = new util::MarkdownDialog(getReadmeMarkdownUrl());

        _helpMarkdownDialog->setWindowTitle(QString("%1").arg(getKind()));
        _helpMarkdownDialog->setAttribute(Qt::WA_DeleteOnClose);
        _helpMarkdownDialog->setWindowModality(Qt::NonModal);
        _helpMarkdownDialog->show();
        });
}

QUrl CellClusteringAnalysisPluginFactory::getReadmeMarkdownUrl() const
{
    return { "https://raw.githubusercontent.com/ManiVaultStudio/CellClusteringPlugin/main/README.md" };
}

QUrl CellClusteringAnalysisPluginFactory::getRepositoryUrl() const
{
    return {"https://github.com/ManiVaultStudio/CellClusteringPlugin"};
}

AnalysisPlugin* CellClusteringAnalysisPluginFactory::produce()
{
    return new CellClusteringAnalysisPlugin(this);
}

mv::DataTypes CellClusteringAnalysisPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;

    // This CellClustering analysis plugin is compatible with points datasets
    supportedTypes.append(PointType);

    return supportedTypes;
}

mv::gui::PluginTriggerActions CellClusteringAnalysisPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this](const Dataset<Points>& dataset) -> CellClusteringAnalysisPlugin* {
        return dynamic_cast<CellClusteringAnalysisPlugin*>(plugins().requestPlugin(getKind(), { dataset }));
    };

    const auto numberOfDatasets = datasets.count();

    if (numberOfDatasets >= 1 && PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<CellClusteringAnalysisPluginFactory*>(this), this, "CellClustering Analysis", "Perform an CellClustering Analysis", icon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (const auto& dataset : datasets)
                getPluginInstance(dataset);
            });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}
