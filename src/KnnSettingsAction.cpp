#include "KnnSettingsAction.h"

KnnSettingsAction::KnnSettingsAction(QObject* parent) :
    GroupAction(parent, "Knn Settings"),
    _numTreesAction(this, "Annoy Trees"),
    _searchKAction(this, "Annoy searchK"),
    _mAction(this, "HNSW M"),
    _efSearchAction(this, "HNSW ef search"),
    _efConstructionAction(this, "HNSW ef const"),
    _powerAction(this, "KMKN power")
{
    setText("Knn Settings");
    setSerializationName("Knn Settings");

    GroupAction::addAction(&_numTreesAction);
    GroupAction::addAction(&_searchKAction);
    GroupAction::addAction(&_mAction);
    GroupAction::addAction(&_efSearchAction);
    GroupAction::addAction(&_efConstructionAction);
    GroupAction::addAction(&_powerAction);

    _numTreesAction.setToolTip("Annoy: provided during build time and affects the build time and the index size.\nA larger value will give more accurate results, but larger indexes.");
    _searchKAction.setToolTip("Annoy: provided in runtime and affects the search performance.\nA larger value will give more accurate results, but will take longer time to return.");
    _mAction.setToolTip("HNSW: number of bi-directional links created for every new element during construction. Reasonable range for M is 2-100.");
    _efSearchAction.setToolTip("HNSW: Sets both search and construction ef parameter.");
    _efConstructionAction.setToolTip("HNSW: Sets both search and construction ef parameter.");

    _numTreesAction.initialize(1, 10000, 8);
    _searchKAction.initialize(1, 10000, 512);
    _mAction.initialize(2, 300, 16);
    _efSearchAction.initialize(1, 1000, 200);
    _efConstructionAction.initialize(1, 1000, 10);
    _powerAction.initialize(0, 1, 0.5, 2);

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();
        _numTreesAction.setEnabled(enable);
        _searchKAction.setEnabled(enable);
        _mAction.setEnabled(enable);
        _efSearchAction.setEnabled(enable);
        _efConstructionAction.setEnabled(enable);
        _powerAction.setEnabled(enable);
        };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
        });

    updateReadOnly();
}

cellclustering::HnswParams KnnSettingsAction::getHnswParams() const
{
    return {
        .numLinks = _mAction.getValue(),
        .efConstruction = _efConstructionAction.getValue(),
        .efSearch = _efSearchAction.getValue(),
    };
}

cellclustering::AnnoyParams KnnSettingsAction::getAnnoyParams() const
{
    return {
        .nTrees = _numTreesAction.getValue(),
        .searchK = static_cast<double>(_searchKAction.getValue()),
    };
}

cellclustering::KMKNNParams KnnSettingsAction::getKMKNNParams() const
{
    return {
        .power = _powerAction.getValue()
    };
}


void KnnSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _numTreesAction.fromParentVariantMap(variantMap);
    _searchKAction.fromParentVariantMap(variantMap);
    _mAction.fromParentVariantMap(variantMap);
    _efSearchAction.fromParentVariantMap(variantMap);
    _efConstructionAction.fromParentVariantMap(variantMap);
    _powerAction.fromParentVariantMap(variantMap);
}

QVariantMap KnnSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _numTreesAction.insertIntoVariantMap(variantMap);
    _searchKAction.insertIntoVariantMap(variantMap);
    _mAction.insertIntoVariantMap(variantMap);
    _efSearchAction.insertIntoVariantMap(variantMap);
    _efConstructionAction.insertIntoVariantMap(variantMap);
    _powerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
