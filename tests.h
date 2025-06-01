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
