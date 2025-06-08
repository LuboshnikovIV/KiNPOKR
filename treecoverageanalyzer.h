#ifndef TREECOVERAGEANALYZER_H
#define TREECOVERAGEANALYZER_H

#include <QHash>
#include <QSet>
#include <QList>
#include <QPair>
#include "Node.h"
#include "Error.h"

class TreeCoverageAnalyzer
{
public:
    enum CoverageStatus {
        FullyCovered,
        PartiallyCovered,
        NotCovered
    };

    TreeCoverageAnalyzer();
    ~TreeCoverageAnalyzer();

    void parseDOT(const QString& content);
    void clearData();
    void fillHash(QList<Node*>& treeMap, QHash<Node*, int>& amountOfParents);
    void treeGraphTakeErrors(QHash<Node*, int>& amountOfParents, bool* isConnected, QSet<Node*>& multiParents, QSet<Node*>& rootNodes);
    void hasCycles(Node* node, QSet<QList<Node*>>& cycles, QSet<Node*>& visitedNodes, QList<Node*>& currentPath);
    void analyzeTreeCoverage();
    void analyzeZoneWithExtraNodes(Node* node);
    CoverageStatus analyzeZoneWithMissingNodes(Node* node);
    void analyzeZoneWithRedundantNodes(Node* node);
    QString getResult() const;

    QSet<Node*> rootNodes;
    QSet<QList<Node*>> cycles;
    QSet<Node*> multiParents;
    QHash<Node*, int> amountOfParents;
    bool isConnected;
    QList<Node*> treeMap;
    QSet<Node*> visitedNodes;
    QSet<Node*> missingNodes;
    QSet<Node*> extraNodes;
    QSet<QPair<Node*, Node*>> redundantNodes;
    bool nodeIsCovered;
};

#endif // TREECOVERAGEANALYZER_H
