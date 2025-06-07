#ifndef TESTS_H
#define TESTS_H

#include <QObject>
#include <QtTest/QtTest>
#include "node.h"
#include "error.h"
#include "treecoverageanalyzer.h"

class Tests : public QObject
{
    Q_OBJECT

private:
    Node* createNode(const QString& name, Node::Shape shape = Node::Shape::Base) {
        return new Node(name, shape);
    }

    void addEdge(Node* parent, Node* child, QHash<Node*, int>& amountOfParents) {
        parent->children.append(child);
        amountOfParents[child] = amountOfParents.value(child, 0) + 1;
    }

    void printNodeSetDifference(const QSet<Node*>& actual, const QSet<Node*>& expected, const QString& containerName);
    void printNodeSetDifferenceForRedundant(const QSet<QPair<Node*, Node*>>& actual, const QSet<QPair<Node*, Node*>>& expected, const QString& containerName);

private slots:
    // data driven тесты для функции parseDOT
    void parseDOT_test();
    void parseDOT_test_data();

    // data driven тесты для функции isConnectedOrHasMultiParents
    void isConnectedOrHasMultiParents_test();
    void isConnectedOrHasMultiParents_test_data();

    // data driven тесты для функции hasCycles
    void hasCycles_test();
    void hasCycles_test_data();

    // data driven тесты для функции analyzeZoneWithExtraNodes
    void analyzeZoneWithExtraNodes_test();
    void analyzeZoneWithExtraNodes_test_data();

    // data driven тесты для функции analyzeZoneWithMissingNodes
    void analyzeZoneWithMissingNodes_test();
    void analyzeZoneWithMissingNodes_test_data();

    // data driven тесты для функции analyzeZoneWithRedundant
    void analyzeZoneWithRedundant_test();
    void analyzeZoneWithRedundant_test_data();
};

#endif // TESTS_H
