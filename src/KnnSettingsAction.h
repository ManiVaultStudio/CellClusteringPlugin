#pragma once

#include "actions/DecimalAction.h"
#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"

#include <cellclustering/options.hpp>

using namespace mv::gui;

/// //////////// ///
/// KNN Settings ///
/// //////////// ///

class KnnSettingsAction : public GroupAction
{
public:

    KnnSettingsAction(QObject* parent = nullptr);

public: // Settings getters
    cellclustering::HnswParams getHnswParams() const;
    cellclustering::AnnoyParams getAnnoyParams() const;
    cellclustering::KMKNNParams getKMKNNParams() const;

public: // Action getters

    IntegralAction& getNumTreesAction() { return _numTreesAction; }
    IntegralAction& getNumChecksAction() { return _searchKAction; }
    IntegralAction& getMAction() { return _mAction; }
    IntegralAction& getEfSearchAction() { return _efSearchAction; }
    IntegralAction& getEfConstructionAction() { return _efConstructionAction; }
    DecimalAction& getPoewrAction() { return _powerAction; }

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
    IntegralAction          _numTreesAction;            /** Annoy parameter Trees action */
    IntegralAction          _searchKAction;             /** Annoy parameter searchK */
    IntegralAction          _mAction;                   /** HNSW parameter M action */
    IntegralAction          _efSearchAction;            /** HNSW parameter ef action */
    IntegralAction          _efConstructionAction;      /** HNSW parameter ef action */
    DecimalAction           _powerAction;               /** KMKN parameter power */
};
