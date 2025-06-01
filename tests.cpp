#include "tests.h"
#define NODE_PARENT_HASH QHash<Node*, int>

void Tests::parseDOT_test(){ QFAIL("Not implemented"); }
void Tests::parseDOT_test_data(){}

void Tests::isConnectedOrHasMultiParents_test(){ QFAIL("Not implemented"); }
void Tests::isConnectedOrHasMultiParents_test_data(){}

void Tests::hasCycles_test_data()
{
    QTest::addColumn<QList<Node*>>("nodes");
    QTest::addColumn<QHash<Node*, int>>("amountOfParents");
    QTest::addColumn<Node*>("startNode");
    QTest::addColumn<int>("expectedCycleCount");
    QTest::addColumn<QSet<QList<Node*>>>("expectedCycles");

    // Тест 1: Граф без цикла
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        nodes << a << b << c;
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        QTest::newRow("GraphWithoutCycle") << nodes << amountOfParents << a << 0 << QSet<QList<Node*>>();
    }

    // Тест 2: Граф с циклом
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        nodes << a << b << c;
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, a, amountOfParents);
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{a, b, c};
        QTest::newRow("GraphWithCycle") << nodes << amountOfParents << a << 1 << expectedCycles;
    }

    // Тест 3: Граф с левитирующим циклом
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        nodes << a << b << c;
        addEdge(b, c, amountOfParents);
        addEdge(c, b, amountOfParents);
        QTest::newRow("GraphWithLevitatingCycle") << nodes << amountOfParents << a << 0 << QSet<QList<Node*>>();
    }

    // Тест 4: Граф с циклом на себя
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        nodes << a;
        addEdge(a, a, amountOfParents);
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{a};
        QTest::newRow("GraphWithSelfCycle") << nodes << amountOfParents << a << 1 << expectedCycles;
    }

    // Тест 5: Граф с несколькими циклами
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        Node* d = createNode("d");
        Node* e = createNode("e");
        Node* f = createNode("f");
        Node* g = createNode("g");
        nodes << a << b << c << d << e << f << g;
        addEdge(a, b, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(c, f, amountOfParents);
        addEdge(f, g, amountOfParents);
        addEdge(g, c, amountOfParents);
        addEdge(e, g, amountOfParents);
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, d, e} << QList<Node*>{c, f, g} << QList<Node*>{e, g};
        QTest::newRow("GraphWithMultipleCycles") << nodes << amountOfParents << a << 3 << expectedCycles;
    }

    // Тест 6: Граф с простым циклом
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        Node* d = createNode("d");
        nodes << a << b << c << d;
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(d, b, amountOfParents);
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, c, d};
        QTest::newRow("GraphWithSimpleCycle") << nodes << amountOfParents << a << 1 << expectedCycles;
    }

    // Тест 7: Граф с циклом и ответвлением
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        Node* d = createNode("d");
        nodes << a << b << c << d;
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, a, amountOfParents);
        addEdge(b, d, amountOfParents);
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{a, b, c};
        QTest::newRow("GraphWithCycleAndBranch") << nodes << amountOfParents << a << 1 << expectedCycles;
    }

    // Тест 8: Граф с большим циклом
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        Node* e = createNode("e");
        Node* f = createNode("f");
        Node* g = createNode("g");
        nodes << a << b << c << e << f << g;
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, g, amountOfParents);
        addEdge(g, b, amountOfParents);
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, c, e, f, g};
        QTest::newRow("GraphWithLargeCycle") << nodes << amountOfParents << a << 1 << expectedCycles;
    }

    // Тест 9: Узел с тремя циклами
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        Node* d = createNode("d");
        Node* e = createNode("e");
        nodes << a << b << c << d << e;
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, b, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(d, b, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(e, b, amountOfParents);
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, c} << QList<Node*>{b, d} << QList<Node*>{b, e};
        QTest::newRow("NodeWithThreeCycles") << nodes << amountOfParents << a << 3 << expectedCycles;
    }

    // Тест 10: Два циклических графа
    {
        QList<Node*> nodes;
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b");
        Node* c = createNode("c");
        nodes << a << b << c;
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, a, amountOfParents);
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{a, b, c};
        QTest::newRow("TwoCyclicGraphs") << nodes << amountOfParents << a << 1 << expectedCycles;
    }
}

void Tests::hasCycles_test()
{
    QFETCH(QList<Node*>, nodes);
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(Node*, startNode);
    QFETCH(int, expectedCycleCount);
    QFETCH(QSet<QList<Node*>>, expectedCycles);

    TreeCoverageAnalyzer analyzer;
    QSet<QList<Node*>> cycles;
    QList<Node*> currentPath;


    analyzer.hasCycles(startNode, cycles, amountOfParents, currentPath);

    QCOMPARE(cycles.size(), expectedCycleCount);
    if (expectedCycleCount > 0) {
        QVERIFY(cycles == expectedCycles);
    }

    cleanupNodes(nodes);
}

void Tests::analyzeZoneWithExtraNodes_test(){ QFAIL("Not implemented"); }
void Tests::analyzeZoneWithExtraNodes_test_data(){}

void Tests::analyzeZoneWithMissingNodes_test(){ QFAIL("Not implemented"); }
void Tests::analyzeZoneWithMissingNodes_test_data(){}

void Tests::analyzeZoneWithRedundant_test(){ QFAIL("Not implemented"); }
void Tests::analyzeZoneWithRedundant_test_data(){}

