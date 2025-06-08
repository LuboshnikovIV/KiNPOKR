#include "treecoverageanalyzer.h"

TreeCoverageAnalyzer::TreeCoverageAnalyzer() {}
TreeCoverageAnalyzer::~TreeCoverageAnalyzer() = default;

void TreeCoverageAnalyzer::parseDOT(const QString& content){
    Q_UNUSED(content);
}
void TreeCoverageAnalyzer::clearData(){
    // Очищаем treeMap и освобождаем память
    qDeleteAll(treeMap);
    treeMap.clear();

    // Очищаем остальные поля
    rootNodes.clear();
    cycles.clear();
    multiParents.clear();
    amountOfParents.clear();
    visitedNodes.clear();
    missingNodes.clear();
    extraNodes.clear();
    redundantNodes.clear();


    // Сбрасываем флаги
    isConnected = false;
    nodeIsCovered = false;
}
void TreeCoverageAnalyzer::fillHash(QList<Node*>& treeMap, QHash<Node*, int>& amountOfParents){
    Q_UNUSED(treeMap);
    Q_UNUSED(amountOfParents);
}
void TreeCoverageAnalyzer::treeGraphTakeErrors(QHash<Node*, int>& amountOfParents, bool* isConnected, QSet<Node*>& multiParents, QSet<Node*>& rootNodes){
    Q_UNUSED(amountOfParents);
    Q_UNUSED(isConnected);
    Q_UNUSED(multiParents);
    Q_UNUSED(rootNodes);
}
void TreeCoverageAnalyzer::hasCycles(Node* node, QSet<QList<Node*>>& cycles, QSet<Node*>& visitedNodes, QList<Node*>& currentPath){
    Q_UNUSED(node);
    Q_UNUSED(cycles);
    Q_UNUSED(visitedNodes);
    Q_UNUSED(currentPath);
}
void TreeCoverageAnalyzer::analyzeTreeCoverage(){}
void TreeCoverageAnalyzer::analyzeZoneWithExtraNodes(Node* node){
    Q_UNUSED(node);
}
TreeCoverageAnalyzer::CoverageStatus TreeCoverageAnalyzer::analyzeZoneWithMissingNodes(Node* node){
    Q_UNUSED(node);
    return FullyCovered;
}
void TreeCoverageAnalyzer::analyzeZoneWithRedundantNodes(Node* node){
    Q_UNUSED(node);
}
QString TreeCoverageAnalyzer::getResult() const{
    return QString("Hi");
}
