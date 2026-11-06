#include "SettingsAction.h"

SettingsAction::SettingsAction(QObject* parent) :
    GroupAction(parent, "SettingsAction", true),
    _startStopActions(this),
    _knnAlgorithmAction(this, "knn Algorithm", { "HNSW", "Annoy", "KMKNN" }),
    _knnMetricAction(this, "knn metric", { "Euclidean", "Cosine", "Manhattan" }),
    _knnKAction(this, "Num neighbors", 1, 100, 30),
    _clusteringAlgorithmAction(this, "Community", {"Louvain" }),
    _minClusterSizeAction(this, "Min. cluster size", 1, 100, 10)
{
    setText("Phenograph Settings");
    setSerializationName("Phenograph Settings");

    _startStopActions.setToolTip("Computation control");
    _clusteringAlgorithmAction.setToolTip("Community detection algorithm");
    _minClusterSizeAction.setToolTip("clusters with size <= are handled seperately");

    _clusteringAlgorithmAction.setCurrentIndex(0);
    _knnMetricAction.setCurrentIndex(0);
    _knnAlgorithmAction.setCurrentIndex(0);

    GroupAction::addAction(&_knnMetricAction);
    GroupAction::addAction(&_knnAlgorithmAction);
    GroupAction::addAction(&_knnKAction);
    GroupAction::addAction(&_clusteringAlgorithmAction);
    GroupAction::addAction(&_minClusterSizeAction);
    GroupAction::addAction(&_startStopActions);
}

cellclustering::ClusteringAlgo SettingsAction::getClusteringAlgo() const
{
    const auto currentIndex = _clusteringAlgorithmAction.getCurrentIndex();

    if (currentIndex == 0)
        return cellclustering::ClusteringAlgo::Louvain;

    return cellclustering::ClusteringAlgo::Louvain;
}

cellclustering::NNMethod SettingsAction::getKnnAlgorithm() const
{
    const auto currentIndex = _knnAlgorithmAction.getCurrentIndex();

    if (currentIndex == 0)
        return cellclustering::NNMethod::HNSW;
    if (currentIndex == 1)
        return cellclustering::NNMethod::Annoy;
    if (currentIndex == 2)
        return cellclustering::NNMethod::KMKNN;

    return cellclustering::NNMethod::HNSW;

}

cellclustering::Metric SettingsAction::getKnnMetric() const
{
    const auto currentIndex = _knnMetricAction.getCurrentIndex();

    if (currentIndex == 0)
        return cellclustering::Metric::Euclidean;
    if (currentIndex == 1)
        return cellclustering::Metric::Cosine;
    if (currentIndex == 2)
        return cellclustering::Metric::Manhattan;

    return cellclustering::Metric::Euclidean;

}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _clusteringAlgorithmAction.fromParentVariantMap(variantMap);
    _knnAlgorithmAction.fromParentVariantMap(variantMap);
    _knnMetricAction.fromParentVariantMap(variantMap);
    _knnKAction.fromParentVariantMap(variantMap);
    _minClusterSizeAction.fromParentVariantMap(variantMap);
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _clusteringAlgorithmAction.insertIntoVariantMap(variantMap);
    _knnAlgorithmAction.insertIntoVariantMap(variantMap);
    _knnMetricAction.insertIntoVariantMap(variantMap);
    _knnKAction.insertIntoVariantMap(variantMap);
    _minClusterSizeAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

/// ////////////////// ///
/// Start Stop Buttons ///
/// ////////////////// ///

ButtonsGroupAction::ButtonsGroupAction(QObject* parent) :
    HorizontalGroupAction(parent, "UmapComputationAction"),
    _startComputationAction(this, "Start"),
    _stopComputationAction(this, "Stop")
{
    setText("Compute");

    GroupAction::addAction(&_startComputationAction);
    GroupAction::addAction(&_stopComputationAction);

    _startComputationAction.setToolTip("Start computation");
    _stopComputationAction.setToolTip("Stop computation");

    _stopComputationAction.setEnabled(false);
}