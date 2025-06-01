#include "treecoverageanalyzer.h"

TreeCoverageAnalyzer::TreeCoverageAnalyzer() {}
TreeCoverageAnalyzer::~TreeCoverageAnalyzer() = default;

void TreeCoverageAnalyzer::parseDOT(const QString& content){
    Q_UNUSED(content);
}
void TreeCoverageAnalyzer::clearData(){}
void TreeCoverageAnalyzer::fillHash(QList<Node*>& treeMap, QHash<Node*, int>& amountOfParents){
    Q_UNUSED(treeMap);
    Q_UNUSED(amountOfParents);
}
void TreeCoverageAnalyzer::isConnectedOrHasMultiParents(QHash<Node*, int>& amountOfParents){
    Q_UNUSED(amountOfParents);
}
void TreeCoverageAnalyzer::hasCycles(Node* node, QSet<QList<Node*>>& cycles, QHash<Node*, int>& amountOfParents, QList<Node*>& currentPath){
    Q_UNUSED(node);
    Q_UNUSED(cycles);
    Q_UNUSED(amountOfParents);
    Q_UNUSED(currentPath);
}
bool TreeCoverageAnalyzer::checkLevitateConected(QHash<Node*, int>& amountOfParents){
    Q_UNUSED(amountOfParents);
    return true;
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
