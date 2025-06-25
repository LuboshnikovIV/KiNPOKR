#include "tests.h"
#include <QString>
#define NODE_PARENT_HASH QHash<Node*, int>
#define REDUNDANT_NODES QSet<QPair<Node*, Node*>>

void Tests::printNodeSetDifference(const QSet<Node*>& actual, const QSet<Node*>& expected, const QString& containerName) {
    QSet<Node*> extraInActual = actual - expected; // Узлы которые есть в контейнере после вызова метода, но нет в ожидаемом контейнере
    QSet<Node*> extraInExpected = expected - actual; // Узлы которые есть в ожидаемом контейнере с узлами, но нет в контейнере после вызова метода
    QString extraActualNames, extraExpectedNames;
    for (Node* n : extraInActual) {
        extraActualNames += n->name + " ";
    }
    for (Node* n : extraInExpected) {
        extraExpectedNames += n->name + " ";
    }
    if (!extraInActual.isEmpty()) {
        qDebug() << "Лишние узлы: " << extraActualNames;
    }
    if (!extraInExpected.isEmpty()) {
        qDebug() << "Пропущенные узлы: " << extraExpectedNames;
    }
}

void Tests::printNodeSetDifferenceForCycles(const QSet<QList<Node*>>& actual, const QSet<QList<Node*>>& expected, const QString& containerName) {
    QSet<QList<Node*>> extraInActual = actual - expected;
    QSet<QList<Node*>> extraInExpected = expected - actual;
    QString extraActualNames, extraExpectedNames;

    for (const QList<Node*>& nodeList : extraInActual) {
        QString listNames;
        for (Node* n : nodeList) {
            listNames += n->name + ",";
        }
        extraActualNames += "[" + (listNames.isEmpty() ? "" : listNames.chopped(1)) + "] ";
    }

    for (const QList<Node*>& nodeList : extraInExpected) {
        QString listNames;
        for (Node* n : nodeList) {
            listNames += n->name + ",";
        }
        extraExpectedNames += "[" + (listNames.isEmpty() ? "" : listNames.chopped(1)) + "] ";
    }

    if (!extraInActual.isEmpty()) {
        qDebug() << "Extra lists in actual:" << extraActualNames.trimmed();
    }
    if (!extraInExpected.isEmpty()) {
        qDebug() << "Missing lists in actual:" << extraExpectedNames.trimmed();
    }
}

void Tests::printNodeSetDifferenceForRedundant(const QSet<QPair<Node*, Node*>>& actual, const QSet<QPair<Node*, Node*>>& expected, const QString& containerName) {
    QSet<QPair<Node*, Node*>> extraInActual = actual - expected;
    QSet<QPair<Node*, Node*>> extraInExpected = expected - actual;
    QString extraActualNames, extraExpectedNames;

    for (const auto& pair : extraInActual) {
        extraActualNames += pair.first->name + "-" + pair.second->name + " ";
    }
    for (const auto& pair : extraInExpected) {
        extraExpectedNames += pair.first->name + "-" + pair.second->name + " ";
    }

    if (!extraInActual.isEmpty()) {
        qDebug() << "Extra pairs in actual:" << extraActualNames.trimmed();
    }
    if (!extraInExpected.isEmpty()) {
        qDebug() << "Missing pairs in actual:" << extraExpectedNames.trimmed();
    }
}

bool compareNodes(const Node* node1, const Node* node2, QSet<QPair<const Node*, const Node*>>& visited) {
    // Проверка на уже посещённую пару для предотвращения рекурсии
    QPair<const Node*, const Node*> pair(node1, node2);
    if (visited.contains(pair)) {
        return true; // Считаем узлы эквивалентными, если уже посещены
    }
    visited.insert(pair);

    // Проверка name и shape
    if (node1->name != node2->name || node1->shape != node2->shape || node1->children.size() != node2->children.size()) {
        return false;
    }

    // Сравнение children
    QSet<Node*> children1(node1->children.begin(), node1->children.end());
    QSet<Node*> children2(node2->children.begin(), node2->children.end());
    if (children1.size() != children2.size()) {
        return false;
    }

    for (Node* child1 : children1) {
        bool foundMatch = false;
        for (Node* child2 : children2) {
            if (compareNodes(child1, child2, visited)) {
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch) {
            return false;
        }
    }
    return true;
}

void Tests::parseDOT_test(){
    QFETCH(QString, content);
    QFETCH(bool, shouldSucceed);
    QFETCH(QList<Error>, expectedErrors);
    QFETCH(QList<Node*>, expectedTreeMap);

    TreeCoverageAnalyzer analyzer;

    // Проверка результатов
    if (shouldSucceed) {
        try {
            analyzer.parseDOT(content);
            QVERIFY(!analyzer.treeMap.isEmpty());
            // Проверка размера treeMap
            QCOMPARE(analyzer.treeMap.size(), expectedTreeMap.size());
            // Поэлементное сравнение treeMap
            QSet<QPair<const Node*, const Node*>> visited;
            for (int i = 0; i < analyzer.treeMap.size(); ++i) {
                if (!analyzer.treeMap[i] || !expectedTreeMap[i]) {
                    QFAIL(("Обнаружен нулевой указатель в treeMap или expectedTreeMap на индексе " + QString::number(i)).toUtf8());
                }
                if (!compareNodes(analyzer.treeMap[i], expectedTreeMap[i], visited)) {
                    // Отладка
                    /*qDebug() << "Actual name:" << analyzer.treeMap[i]->name << ", Expected name:" << expectedTreeMap[i]->name;
                    qDebug() << "Actual shape:" << (int)analyzer.treeMap[i]->shape << ", Expected shape:" << (int)expectedTreeMap[i]->shape;
                    QString actualChildrenNames, expectedChildrenNames;
                    for (Node* child : analyzer.treeMap[i]->children) {
                        actualChildrenNames += child->name + " ";
                    }
                    for (Node* child : expectedTreeMap[i]->children) {
                        expectedChildrenNames += child->name + " ";
                    }
                    qDebug() << "Actual children:" << actualChildrenNames.trimmed() << ", Expected children:" << expectedChildrenNames.trimmed();*/
                    QFAIL(("Узлы различаются на индексе " + QString::number(i)).toUtf8());
                }
                visited.clear(); // Очищаем visited для следующей пары узлов
            }
        } catch (const Error& e) {
            QFAIL("Не ожидалось исключение для корректного случая");
        } catch (...) {
            QFAIL("Неизвестное исключение в корректном случае");
        }
    } else {
        try {
            analyzer.parseDOT(content);
            if (analyzer.errors.isEmpty()) {
                QFAIL("Ожидались ошибки для некорректного случая");
            }
        } catch (const Error& e) {
            QCOMPARE(analyzer.errors, expectedErrors); // Проверяем, что ошибки совпадают
        } catch (...) {
            QFAIL("Неизвестное исключение в некорректном случае");
        }
    }

    // Отчистка результатов
    analyzer.clearData();
    qDeleteAll(expectedTreeMap);
}
void Tests::parseDOT_test_data(){
    QTest::addColumn<QString>("content");
    QTest::addColumn<bool>("shouldSucceed");
    QTest::addColumn<QList<Error>>("expectedErrors");
    QTest::addColumn<QList<Node*>>("expectedTreeMap");

    // Тест 1: Пустой файл
    {
        QTest::newRow("EmptyFile") << ""
                               << false
                               << (QList<Error>{Error(Error::EmptyFile)})
                               << QList<Node*>();
    }

    // Тест 2: Отсутствует целевой узел
    {
    QTest::newRow("NoTargetNode") << "digraph test {\n"
                                                "a[shape=circle];\n"
                                                "b[shape=diamond];\n"
                                                "a->b;\n"
                                                "}"
                               << false
                               << (QList<Error>{Error(Error::NoTargetNode)})
                               << QList<Node*>();
    }

    // Тест 3: Ненаправленная связь
    {
    QTest::newRow("UndirectedEdge") << "graph test {\n"
                                       "a[shape=square];\n"
                                       "b[shape=diamond];\n"
                                       "a--b;\n"
                                       "}"
                               << false
                               << (QList<Error>{Error(Error::UndirectedEdge)})
                               << QList<Node*>();
    }

    // Тест 4: Метка на связи
    {
    QTest::newRow("EdgeLabel") << "digraph test {\n"
                                  "a[shape=square];\n"
                                  "b[shape=diamond];\n"
                                  "a->b[label=\"test\"];\n"
                                  "}"
                               << false
                               << (QList<Error>{Error(Error::EdgeLabel)})
                               << QList<Node*>();
    }

    // Тест 5: Неверная форма узла
    {
    QTest::newRow("InvalidNodeShape") << "digraph test {\n"
                                         "a[shape=square];\n"
                                         "b[shape=star];\n"
                                         "a->b;\n"
                                         "}"
                               << false
                                      << (QList<Error>{Error(Error::InvalidNodeShape, "b")})
                                   << QList<Node*>();
    }

    // Тест 6: Корректный граф
    {
    QHash<Node*, int> amountOfParents;
    QList<Node*> expectedTreeMap;
    Node* a = createNode("a", Node::Shape::Target);
    Node* b = createNode("b", Node::Shape::Selected);
    Node* c = createNode("c", Node::Shape::Base);
    addEdge(a, b, amountOfParents);
    addEdge(a, c, amountOfParents);
    expectedTreeMap << a << b << c;
    QTest::newRow("CorrectGraph") << "digraph test {\n"
                                     "a[shape=square];\n"
                                     "b[shape=diamond];\n"
                                     "c[shape=circle];\n"
                                     "a->b;\n"
                                     "a->c;\n"
                                     "}"
                               << true
                                  << (QList<Error>{})
                                   << expectedTreeMap;
    }

    // Тест 7: Дополнительная метка узла
    {
    QTest::newRow("ExtraLabel") << "digraph test {\n"
                                   "a[shape=square,label=\"test\"];\n"
                                   "b[shape=diamond];\n"
                                   "a->b;\n"
                                   "}"
                               << false
                                << (QList<Error>{Error(Error::ExtraLabel, "a lable=\"test\"")})
                                << QList<Node*>();
    }

    // Тест 8: Комплексный
    {
    QTest::newRow("ComplexCase") << "graph test {\n"
                                    "a[shape=circle,label=\"test\"];\n"
                                    "b[shape=star];\n"
                                    "a--b[label=\"test\"];\n"
                                    "}"
                               << false
                                 << (QList<Error>{
                                        Error(Error::ExtraLabel, "a label=\"test\""),
                                        Error(Error::InvalidNodeShape, "b"),
                                        Error(Error(Error::EdgeLabel)),
                                        Error(Error::NoTargetNode)
                                    })
                                   << QList<Node*>();
    }

    // Тест 9: Граф с циклом
    {
    QList<Node*> expectedTreeMap;
    Node* a = createNode("a", Node::Shape::Target);
    Node* b = createNode("b", Node::Shape::Base);
    Node* c = createNode("c", Node::Shape::Base);
    Node* d = createNode("d", Node::Shape::Base);
    a->children << b;
    b->children << c;
    c->children << d;
    d->children << b;
    expectedTreeMap << a << b << c << d;
    QTest::newRow("GraphWithCycle") << "digraph test {\n"
                                    "a[shape=square];\n"
                                    "b,c,d[shape=circle];\n"
                                    "a->b;\n"
                                    "b->c;\n"
                                    "c->d;\n"
                                    "d->b;\n"
                                    "}"
                                 << true
                                 << (QList<Error>{})
                                   << expectedTreeMap;
    }

    // Тест 10: Несвязный граф
    {
    QList<Node*> expectedTreeMap;
    Node* a = createNode("a", Node::Shape::Target);
    Node* b = createNode("b", Node::Shape::Base);
    Node* c = createNode("c", Node::Shape::Base);
    Node* d = createNode("d", Node::Shape::Base);
    Node* e = createNode("e", Node::Shape::Base);
    a->children << b << c;
    d->children << e;
    expectedTreeMap << a << b << c << d << e;
    QTest::newRow("DisconnectedGraph") << "digraph test {\n"
                                       "a[shape=square];\n"
                                       "b,c,d,e[shape=circle];\n"
                                       "a->b;\n"
                                       "a->c;\n"
                                       "d->e;\n"
                                       "}"
                                    << true
                                       << (QList<Error>{})
                                   << expectedTreeMap;
    }

    // Тест 11: Граф с летающим циклом
    {
    QList<Node*> expectedTreeMap;
    Node* a = createNode("a", Node::Shape::Target);
    Node* b = createNode("b", Node::Shape::Base);
    Node* c = createNode("c", Node::Shape::Base);
    Node* d = createNode("d", Node::Shape::Base);
    Node* e = createNode("e", Node::Shape::Base);
    Node* f = createNode("f", Node::Shape::Base);
    a->children << b << c;
    d->children << e;
    e->children << f;
    f->children << d;
    expectedTreeMap << a << b << c << d << e << f;
    QTest::newRow("GraphWithLevitateCycle") << "digraph test {\n"
                                          "a[shape=square];\n"
                                          "b,c,d,e,f[shape=circle];\n"
                                          "a->b;\n"
                                          "a->c;\n"
                                          "d->e;\n"
                                          "e->f;\n"
                                          "f->d;\n"
                                          "}"
                                       << true
                                            << (QList<Error>{})
                               << expectedTreeMap;
    }
}

void Tests::treeGraphTakeErrors_test(){
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(QSet<Node*>, expectedRootNodes);
    QFETCH(bool, expectedIsConnected);
    QFETCH(QSet<Node*>, expectedMultiParents);
    QFETCH(QList<Error>, expectedErrors);

    TreeCoverageAnalyzer analyzer;

    // Вызов метода
    analyzer.treeGraphTakeErrors(amountOfParents);

    // Сравниваем заполнение контейнеров
    if (!QTest::qCompare(analyzer.rootNodes, expectedRootNodes, "analyzer.rootNodes", "expectedRootNodes", __FILE__, __LINE__)) {
        printNodeSetDifference(analyzer.rootNodes, expectedRootNodes, "rootNodes");
    }
    if (!QTest::qCompare(analyzer.multiParents, expectedMultiParents, "analyzer.multiParents", "expectedMultiParents", __FILE__, __LINE__)) {
        printNodeSetDifference(analyzer.multiParents, expectedMultiParents, "multiParents");
    }

    // Проверка результатов
    QCOMPARE(analyzer.rootNodes, expectedRootNodes);
    QCOMPARE(analyzer.isConnected, expectedIsConnected);
    QCOMPARE(analyzer.multiParents, expectedMultiParents);
    QCOMPARE(analyzer.errors, expectedErrors);

    // Очистка через clearData
    analyzer.clearData();
}
void Tests::treeGraphTakeErrors_test_data(){
    QTest::addColumn<NODE_PARENT_HASH>("amountOfParents");
    QTest::addColumn<QSet<Node*>>("expectedRootNodes");
    QTest::addColumn<bool>("expectedIsConnected");
    QTest::addColumn<QSet<Node*>>("expectedMultiParents");
    QTest::addColumn<QList<Error>>("expectedErrors");

    // Тест 1: Дерево состоящее из одного узла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        amountOfParents[a] = 0;
        QSet<Node*> expectedRootNodes;
        expectedRootNodes << a;
        QTest::newRow("GraphConsistOfOneNode") << amountOfParents
                                               << expectedRootNodes
                                               << true
                                               << QSet<Node*>()
                                               << QList<Error>();
    }

    // Тест 2: Узел с двумя родителями
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        amountOfParents[a] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(c, d, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << d;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::MultiParents);
        QTest::newRow("NodeWithTwoParents") << amountOfParents
                                            << rootNodes
                                            << true
                                            << expectedMultiParents
                                            << expectedErrors;
    }

    // Тест 3: Несвязный граф
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        amountOfParents[a] = 0;
        amountOfParents[c] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(c, g, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a << c;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::DisconnectedGraph);
        QTest::newRow("DisconnectedGraph") << amountOfParents
                                           << rootNodes
                                           << false
                                           << QSet<Node*>()
                                           << expectedErrors;
    }

    // Тест 4: Множественные родители у нескольких узлов
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* k = createNode("k", Node::Shape::Base);
        amountOfParents[a] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(a, d, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(d, k, amountOfParents);
        addEdge(c, k, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << d << k;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::MultiParents) << Error(Error::MultiParents);
        QTest::newRow("MultiNodesHaveMultiParents") << amountOfParents
                                                    << rootNodes
                                                    << true
                                                    << expectedMultiParents
                                                    << expectedErrors;
    }

    // Тест 5: Граф с циклом
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        amountOfParents[a] = 0;
        addEdge(b, c, amountOfParents);
        addEdge(a, b, amountOfParents);
        addEdge(c, a, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::Cycle);
        QTest::newRow("GraphWithCycle") << amountOfParents
                                        << rootNodes
                                        << true
                                        << QSet<Node*>()
                                        << expectedErrors;
    }

    // Тест 6: Граф с узлом без родителей (не корень)
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        amountOfParents[a] = 0;
        amountOfParents[c] = 0;
        addEdge(a, b, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a << c;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::DisconnectedGraph);
        QTest::newRow("GraphWithNodeWithoutParents") << amountOfParents
                                                     << rootNodes
                                                     << false
                                                     << QSet<Node*>()
                                                     << expectedErrors;
    }

    // Тест 7: Граф где все узлы имеют 2 родителя
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        amountOfParents[a] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, b, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << b << c;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::MultiParents) << Error(Error::MultiParents) << Error(Error::Cycle);
        QTest::newRow("NonrootNodesHaveTwoParents") << amountOfParents
                                                    << rootNodes
                                                    << true
                                                    << expectedMultiParents
                                                    << expectedErrors;
    }

    // Тест 8: Граф с одним узлом и циклом на себя
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        amountOfParents[a] = 0;
        addEdge(a, a, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::Cycle);
        QTest::newRow("OneNodeWithCycle") << amountOfParents
                                          << rootNodes
                                          << true
                                          << QSet<Node*>()
                                          << expectedErrors;
    }

    // Тест 9: Граф с летающим циклом
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        amountOfParents[a] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, c, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::DisconnectedGraph);
        QTest::newRow("GraphWithLevitateCycle") << amountOfParents
                                                << rootNodes
                                                << false
                                                << QSet<Node*>()
                                                << expectedErrors;
    }

    // Тест 10: Три родителя у узла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        amountOfParents[a] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(a, d, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(c, d, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << d;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::MultiParents);
        QTest::newRow("ThreeParentsForNode") << amountOfParents
                                             << rootNodes
                                             << true
                                             << expectedMultiParents
                                             << expectedErrors;
    }

    // Тест 11: Сложный граф с множеством родителей
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        Node* r = createNode("r", Node::Shape::Base);
        Node* l = createNode("l", Node::Shape::Base);
        amountOfParents[a] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(a, l, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(e, l, amountOfParents);
        addEdge(e, r, amountOfParents);
        addEdge(d, c, amountOfParents);
        addEdge(d, g, amountOfParents);
        addEdge(c, g, amountOfParents);
        addEdge(g, r, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << c << g << r << l;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::MultiParents) << Error(Error::MultiParents) << Error(Error::MultiParents) << Error(Error::MultiParents);
        QTest::newRow("GraphWithHightNestingAndMultiParents") << amountOfParents
                                                              << rootNodes
                                                              << true
                                                              << expectedMultiParents
                                                              << expectedErrors;
    }

    // Тест 12: Связный граф с двумя корнями
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        amountOfParents[a] = 0;
        amountOfParents[c] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(c, b, amountOfParents);
        addEdge(b, d, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a << c;
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << b;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::MultiParents);
        QTest::newRow("ConnectedGraphWithTwoRoots") << amountOfParents
                                                              << rootNodes
                                                              << true
                                                              << expectedMultiParents
                                                              << expectedErrors;
    }

    // Тест 13: Граф с множеством корней и летающий цикл
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        amountOfParents[a] = 0;
        amountOfParents[c] = 0;
        addEdge(a, b, amountOfParents);
        addEdge(c, b, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, g, amountOfParents);
        addEdge(g, e, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a << c;
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << b;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::MultiParents) << Error(Error::DisconnectedGraph);
        QTest::newRow("GraphWithMultiRootsAndLevitatingCycle") << amountOfParents
                                                               << rootNodes
                                                               << false
                                                               << expectedMultiParents
                                                               << expectedErrors;
    }

    // Тест 14: Два летающих цикла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        amountOfParents[a] = 0;
        addEdge(c, a, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(a, b, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, d, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QList<Error> expectedErrors;
        expectedErrors << Error(Error::Cycle) << Error(Error::DisconnectedGraph);
        QTest::newRow("TwoLevitateCycles") << amountOfParents
                                                               << rootNodes
                                                               << false
                                                               << QSet<Node*>()
                                                               << expectedErrors;
    }
}

void Tests::hasCycles_test(){
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(QSet<Node*>, rootNodes);
    QFETCH(QSet<QList<Node*>>, expectedCycles);
    QFETCH(QSet<Node*>, expectedVisitedNodes);
    QFETCH(QList<Error>, expectedError);

    TreeCoverageAnalyzer analyzer;
    QList<Node*> currentPath;

    for(Node* startNode: rootNodes){
        if (!startNode) {
            //qDebug() << "Warning: Null startNode detected!";
            continue;
        }

        // Вызов метода
        analyzer.hasCycles(startNode, currentPath);
    }

    // Сравниваем заполнение контейнеров
    if (!QTest::qCompare(analyzer.cycles, expectedCycles, "analyzer.cycles", "expectedCycles", __FILE__, __LINE__)) {
        printNodeSetDifferenceForCycles(analyzer.cycles, expectedCycles, "multiParents");
    }

    // Проверка результатов
    QCOMPARE(analyzer.cycles, expectedCycles);
    QCOMPARE(analyzer.visitedNodes, expectedVisitedNodes);
    QCOMPARE(analyzer.errors, expectedError);

    // Отчистка данных
    analyzer.clearData();
}
void Tests::hasCycles_test_data(){
    QTest::addColumn<NODE_PARENT_HASH>("amountOfParents");
    QTest::addColumn<QSet<Node*>>("rootNodes");
    QTest::addColumn<QSet<QList<Node*>>>("expectedCycles");
    QTest::addColumn<QSet<Node*>>("expectedVisitedNodes");
    QTest::addColumn<QList<Error>>("expectedError");

    // Тест 1: Граф без цикла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c;
        QTest::newRow("GraphWithoutCycle") << amountOfParents
                                           << rootNodes
                                           << QSet<QList<Node*>>()
                                           << expectedVisitedNodes
                                           << QList<Error>();
    }

    // Тест 2: Граф состоящий из цикла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, a, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{a, b, c, a};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("GraphWithCycle") << amountOfParents
                                        << rootNodes
                                        << expectedCycles
                                        << expectedVisitedNodes
                                        << expectedError;
    }

    // Тест 3: Граф с левитирующим циклом
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, d, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c;
        QTest::newRow("GraphWithLevitatingCycle") << amountOfParents
                                                  << rootNodes
                                                  << QSet<QList<Node*>>()
                                                  << expectedVisitedNodes
                                                  << QList<Error>();
    }

    // Тест 4: Граф с циклом на себя
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        addEdge(a, a, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{a, a};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("GraphWithSelfCycle") << amountOfParents
                                            << rootNodes
                                            << expectedCycles
                                            << expectedVisitedNodes
                                            << expectedError;
    }

    // Тест 5: Граф с несколькими циклами
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(c, f, amountOfParents);
        addEdge(f, g, amountOfParents);
        addEdge(g, c, amountOfParents);
        addEdge(e, g, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, d, e, b} << QList<Node*>{c, f, g, c} << QList<Node*>{g, c, f, g};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c << d << e << f << g;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("GraphWithMultipleCycles") << amountOfParents
                                                 << rootNodes
                                                 << expectedCycles
                                                 << expectedVisitedNodes
                                                 << expectedError;
    }

    // Тест 6: Граф с циклом
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(d, b, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, c, d, b};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c << d;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("GraphWithSimpleCycle") << amountOfParents
                                              << rootNodes
                                              << expectedCycles
                                              << expectedVisitedNodes
                                              << expectedError;
    }

    // Тест 7: Граф с циклом и ответвлением
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, a, amountOfParents);
        addEdge(b, d, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{a, b, c, a};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c << d;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("GraphWithCycleAndBranch") << amountOfParents
                                                 << rootNodes
                                                 << expectedCycles
                                                 << expectedVisitedNodes
                                                 << expectedError;
    }

    // Тест 8: Граф с большим циклом
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, g, amountOfParents);
        addEdge(g, b, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, c, d, e, f, g, b};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c << d << e << f << g;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("GraphWithLargeCycle") << amountOfParents
                                             << rootNodes
                                             << expectedCycles
                                             << expectedVisitedNodes
                                             << expectedError;
    }

    // Тест 9: Узел с тремя циклами
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, b, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(d, b, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(e, b, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, c, b} << QList<Node*>{b, d, b} << QList<Node*>{b, e, b};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c << d << e;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("NodeWithThreeCycles") << amountOfParents
                                             << rootNodes
                                             << expectedCycles
                                             << expectedVisitedNodes
                                             << expectedError;
    }

    // Тест 10: Два циклических графа
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, a, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, d, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{a, b, c, a};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("TwoCyclicGraphs") << amountOfParents
                                         << rootNodes
                                         << expectedCycles
                                         << expectedVisitedNodes
                                         << expectedError;
    }

    // Тест 11: Два графа с циклами
    {
        QHash<Node*, int> amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        Node* h = createNode("h", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(d, b, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, g, amountOfParents);
        addEdge(g, h, amountOfParents);
        addEdge(h, f, amountOfParents);
        QSet<Node*> rootNodes;
        rootNodes << a;
        QSet<QList<Node*>> expectedCycles;
        expectedCycles << QList<Node*>{b, c, d, b};
        QSet<Node*> expectedVisitedNodes;
        expectedVisitedNodes << a << b << c << d;
        QList<Error> expectedError;
        expectedError << Error(Error::Cycle);
        QTest::newRow("TwoGraphsWithCycle") << amountOfParents
                                         << rootNodes
                                         << expectedCycles
                                         << expectedVisitedNodes
                                         <<expectedError;
    }

}

void Tests::analyzeZoneWithExtraNodes_test(){
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(Node*, node);
    QFETCH(QSet<Node*>, expectedExtraNodes);

    TreeCoverageAnalyzer analyzer;

    // Вызов метода
    analyzer.analyzeZoneWithExtraNodes(node);

    // Выводом разницы при провале
    if (!QTest::qCompare(analyzer.extraNodes, expectedExtraNodes, "analyzer.extraNodes", "expectedExtraNodes", __FILE__, __LINE__)) {
        printNodeSetDifference(analyzer.extraNodes, expectedExtraNodes, "extraNodes");
    }

    // Проверка результатов
    QCOMPARE(analyzer.extraNodes, expectedExtraNodes);

    // Очистка
    analyzer.clearData();
}
void Tests::analyzeZoneWithExtraNodes_test_data(){
    QTest::addColumn<NODE_PARENT_HASH>("amountOfParents");
    QTest::addColumn<Node*>("node");
    QTest::addColumn<QSet<Node*>>("expectedExtraNodes");

    // Тест 1: Нет узлов в зоне
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        QTest::newRow("NoNodesInExtraZone") << amountOfParents
                                           << a
                                           << QSet<Node*>();
    }

    // Тест 2: Один лишний узел до целевого узла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        addEdge(c, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        QSet<Node*> expectedExtraNodes;
        expectedExtraNodes << c;
        QTest::newRow("OneExtraNodeBeforeTarget") << amountOfParents
                                                 << c
                                                 << expectedExtraNodes;
    }

    // Тест 3: Несколько лишних узлов до целевого узла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Selected);
        addEdge(d, c, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(c, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        QSet<Node*> expectedExtraNodes;
        expectedExtraNodes << c << e;
        QTest::newRow("SomeExtraNodeBeforeTarget") << amountOfParents
                                                    << d
                                                    << expectedExtraNodes;
    }

    // Тест 4: В одной ветке два отмеченных узла до целевого
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Selected);
        addEdge(c, d, amountOfParents);
        addEdge(d, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        QSet<Node*> expectedExtraNodes;
        expectedExtraNodes << c;
        QTest::newRow("SomeExtraNodesInOneBranch") << amountOfParents
                                                   << c
                                                   << expectedExtraNodes;
    }

    // Тест 5: Смешанные типы узлов до целевого
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Base);
        addEdge(c, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, g, amountOfParents);
        addEdge(g, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        QSet<Node*> expectedExtraNodes;
        expectedExtraNodes << c;
        QTest::newRow("MixedTypeOfNodesBeforeTarget") << amountOfParents
                                                   << c
                                                   << expectedExtraNodes;
    }

    // Тест 6: Все узлы до целевого не отмечены
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        addEdge(c, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e,a, amountOfParents);
        addEdge(a, b, amountOfParents);
        QTest::newRow("BaseTypeOfNodesBeforeTarget") << amountOfParents
                                                   << c
                                                   << QSet<Node*>();
    }

    // Тест 7: Множество лишних узлов в различных ветках
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Selected);
        Node* h = createNode("h", Node::Shape::Base);
        Node* i = createNode("i", Node::Shape::Selected);
        Node* k = createNode("k", Node::Shape::Selected);
        addEdge(c, d, amountOfParents);
        addEdge(c, e, amountOfParents);
        addEdge(d, f, amountOfParents);
        addEdge(d, g, amountOfParents);
        addEdge(f, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        addEdge(e, k, amountOfParents);
        addEdge(e, h, amountOfParents);
        addEdge(h, i, amountOfParents);
        QSet<Node*> expectedExtraNodes;
        expectedExtraNodes << f << g << i << k;
        QTest::newRow("MultiExtraNodesInDifferentBranches") << amountOfParents
                                                     << c
                                                     << expectedExtraNodes;
    }

    // Тест 8: Все узлы до целевого отмечены
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* f = createNode("f", Node::Shape::Selected);
        addEdge(c, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        QSet<Node*> expectedExtraNodes;
        expectedExtraNodes << c;
        QTest::newRow("AllNodesAreSelectedType") << amountOfParents
                                                            << c
                                                            << expectedExtraNodes;
    }

    // Тест 9: Сложный шраф с высокой вложенностью
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Base);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Target);
        Node* h = createNode("h", Node::Shape::Base);
        Node* i = createNode("i", Node::Shape::Base);
        Node* j = createNode("j", Node::Shape::Selected);
        Node* k = createNode("k", Node::Shape::Selected);
        Node* l = createNode("l", Node::Shape::Selected);
        Node* m = createNode("m", Node::Shape::Base);
        Node* n = createNode("n", Node::Shape::Selected);
        Node* o = createNode("o", Node::Shape::Base);
        Node* p = createNode("p", Node::Shape::Selected);
        Node* r = createNode("r", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(a, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, g, amountOfParents);
        addEdge(c, h, amountOfParents);
        addEdge(c, i, amountOfParents);
        addEdge(h, j, amountOfParents);
        addEdge(i, k, amountOfParents);
        addEdge(i, l, amountOfParents);
        addEdge(d, m, amountOfParents);
        addEdge(m, n, amountOfParents);
        addEdge(m, p, amountOfParents);
        addEdge(m, o, amountOfParents);
        addEdge(p, r, amountOfParents);
        QSet<Node*> expectedExtraNodes;
        expectedExtraNodes << e << j << k << l << n << p;
        QTest::newRow("DifficultGraphWithHightNesting") << amountOfParents
                                                            << a
                                                            << expectedExtraNodes;
    }

    // Тест 10: В графе есть все типы зон
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Base);
        Node* b = createNode("b", Node::Shape::Target);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        Node* h = createNode("h", Node::Shape::Selected);
        Node* i = createNode("i", Node::Shape::Selected);
        Node* j = createNode("j", Node::Shape::Selected);
        Node* k = createNode("k", Node::Shape::Selected);
        Node* l = createNode("l", Node::Shape::Base);
        Node* m = createNode("m", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(b, f, amountOfParents);
        addEdge(d, h, amountOfParents);
        addEdge(e, i, amountOfParents);
        addEdge(f, j, amountOfParents);
        addEdge(i, l, amountOfParents);
        addEdge(i, m, amountOfParents);
        addEdge(c, g, amountOfParents);
        addEdge(g, k, amountOfParents);
        QSet<Node*> expectedExtraNodes;
        expectedExtraNodes << c;
        QTest::newRow("GraphHasAllTypesOfZones") << amountOfParents
                                                            << a
                                                            << expectedExtraNodes;
    }
}

void Tests::analyzeZoneWithMissingNodes_test(){
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(Node*, node);
    QFETCH(TreeCoverageAnalyzer::CoverageStatus, expectedCoverageStatus);
    QFETCH(QSet<Node*>, expectedMissingNodes);

    TreeCoverageAnalyzer analyzer;

    // Вызов метода
    TreeCoverageAnalyzer::CoverageStatus status = analyzer.analyzeZoneWithMissingNodes(node);

    // Выводом разницы при провале
    if (!QTest::qCompare(analyzer.missingNodes, expectedMissingNodes, "analyzer.missingNodes", "expectedMissingNodes", __FILE__, __LINE__)) {
        printNodeSetDifference(analyzer.missingNodes, expectedMissingNodes, "missingNodes");
    }

    // Проверка результатов
    QCOMPARE(status, expectedCoverageStatus);
    QCOMPARE(analyzer.missingNodes, expectedMissingNodes);

    // Очистка
    analyzer.clearData();
}
void Tests::analyzeZoneWithMissingNodes_test_data(){
    QTest::addColumn<NODE_PARENT_HASH>("amountOfParents");
    QTest::addColumn<Node*>("node");
    QTest::addColumn<TreeCoverageAnalyzer::CoverageStatus>("expectedCoverageStatus");
    QTest::addColumn<QSet<Node*>>("expectedMissingNodes");

    // Тест 1: Покрытие целевого узла - есть (простой граф)
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        QTest::newRow("TargetNodeHasCoverege") << amountOfParents
                                               << a
                                               << TreeCoverageAnalyzer::FullyCovered
                                               << QSet<Node*>();
    }

    // Тест 2: Покрытие целевого узла - есть (простой граф)
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << c;
        QTest::newRow("TargetNodeHasNotCoverege") << amountOfParents
                                                  << a
                                                  << TreeCoverageAnalyzer::PartiallyCovered
                                                  << expectedMissingNodes;
    }

    // Тест 3: Покрытие определяется потомками
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* f = createNode("f", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(c, f, amountOfParents);
        addEdge(c, g, amountOfParents);
        QTest::newRow("CoverageByGrandchildrens") << amountOfParents
                                                  << a
                                                  << TreeCoverageAnalyzer::FullyCovered
                                                  << QSet<Node*>();
    }

    // Тест 4: Смешанное покрытие
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        QTest::newRow("MixedCoverage") << amountOfParents
                                       << a
                                       << TreeCoverageAnalyzer::FullyCovered
                                       << QSet<Node*>();
    }

    // Тест 5: Узлы которых не хватает  для покрытия находятся на разных уровнях
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(c, f, amountOfParents);
        addEdge(c, g, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << e << c;
        QTest::newRow("MissingNodesLocateAtDiffrentLevel") << amountOfParents
                                                           << a
                                                           << TreeCoverageAnalyzer::PartiallyCovered
                                                           << expectedMissingNodes ;
    }

    // Тест 6: Узла которого не хватает для покрытия является внуком целевого
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* f = createNode("f", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(c, f, amountOfParents);
        addEdge(c, g, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << g;
        QTest::newRow("MissingNodeIsDiscendant") << amountOfParents
                                                 << a
                                                 << TreeCoverageAnalyzer::PartiallyCovered
                                                 << expectedMissingNodes ;
    }

    // Тест 7: Единственный узел - целевой
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        QTest::newRow("OnlyTargetNode") << amountOfParents
                                        << a
                                        << TreeCoverageAnalyzer::PartiallyCovered
                                        << QSet<Node*>();
    }

    // Тест 8: Все предки не отмечены
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* t = createNode("t", Node::Shape::Base);
        Node* r = createNode("r", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        Node* l = createNode("l", Node::Shape::Base);
        Node* y = createNode("y", Node::Shape::Base);
        Node* u = createNode("u", Node::Shape::Base);
        Node* i = createNode("i", Node::Shape::Base);
        Node* o = createNode("o", Node::Shape::Base);
        Node* p = createNode("p", Node::Shape::Base);
        Node* s = createNode("s", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(d, g, amountOfParents);
        addEdge(d, l, amountOfParents);
        addEdge(e, y, amountOfParents);
        addEdge(e, u, amountOfParents);
        addEdge(c, t, amountOfParents);
        addEdge(c, r, amountOfParents);
        addEdge(t, i, amountOfParents);
        addEdge(t, o, amountOfParents);
        addEdge(r, s, amountOfParents);
        addEdge(r, p, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << b << c;
        QTest::newRow("AllDescendantsNotSelected") << amountOfParents
                                                   << a
                                                   << TreeCoverageAnalyzer::PartiallyCovered
                                                   << expectedMissingNodes;
    }

    // Тест 9: Переход в redundant зону с покрытием
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        QTest::newRow("TransitionToRedundantZoneWithCoverage") << amountOfParents
                                                               << a
                                                               << TreeCoverageAnalyzer::FullyCovered
                                                               << QSet<Node*>();
    }

    // Тест 10: Переход в redundant зону без покрытия
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* l = createNode("l", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, l, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << l;
        QTest::newRow("TransitionToRedundantZoneWithoutCoverage") << amountOfParents
                                                                  << a
                                                                  << TreeCoverageAnalyzer::PartiallyCovered
                                                                  << expectedMissingNodes;
    }

    // Тест 11: Дети целевого узла отмечены, внуки не отмечены
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        Node* h = createNode("h", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(c, g, amountOfParents);
        addEdge(c, h, amountOfParents);
        QTest::newRow("TargetChildrebsSelectedButGrandchildrensNot") << amountOfParents
                                                                     << a
                                                                     << TreeCoverageAnalyzer::FullyCovered
                                                                     << QSet<Node*>();
    }

    // Тест 12: Сложный граф с высокой вложенностью
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Selected);
        Node* h = createNode("h", Node::Shape::Base);
        Node* i = createNode("i", Node::Shape::Base);
        Node* j = createNode("j", Node::Shape::Selected);
        Node* k = createNode("k", Node::Shape::Selected);
        Node* l = createNode("l", Node::Shape::Base);
        Node* m = createNode("m", Node::Shape::Base);
        Node* n = createNode("n", Node::Shape::Selected);
        Node* o = createNode("o", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(d, g, amountOfParents);
        addEdge(d, h, amountOfParents);
        addEdge(h, j, amountOfParents);
        addEdge(c, f, amountOfParents);
        addEdge(f, i, amountOfParents);
        addEdge(f, n, amountOfParents);
        addEdge(i, k, amountOfParents);
        addEdge(i, l, amountOfParents);
        addEdge(i, m, amountOfParents);
        addEdge(n, o, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << e << l << m;
        QTest::newRow("DifficultGraphWithHightNesting") << amountOfParents
                                                        << a
                                                        << TreeCoverageAnalyzer::PartiallyCovered
                                                        << expectedMissingNodes;
    }

    // Тест 13: Граф со всеми видами покрытия
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        Node* h = createNode("h", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(a, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(b, f, amountOfParents);
        addEdge(c, h, amountOfParents);
        addEdge(c, g, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << b << g;
        QTest::newRow("GraphWithAllTypesCoverage") << amountOfParents
                                                   << a
                                                   << TreeCoverageAnalyzer::PartiallyCovered
                                                   << expectedMissingNodes;
    }

    // Тест 14: Узлы которых не хватает для покрытия обладают высокой вложенностью
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        Node* h = createNode("h", Node::Shape::Base);
        Node* i = createNode("i", Node::Shape::Base);
        Node* p = createNode("p", Node::Shape::Selected);
        Node* u = createNode("u", Node::Shape::Base);
        Node* l = createNode("l", Node::Shape::Base);
        Node* o = createNode("o", Node::Shape::Base);
        Node* k = createNode("k", Node::Shape::Selected);
        Node* j = createNode("j", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(e, g, amountOfParents);
        addEdge(f, h, amountOfParents);
        addEdge(f, i, amountOfParents);
        addEdge(h, l, amountOfParents);
        addEdge(h, o, amountOfParents);
        addEdge(l, j, amountOfParents);
        addEdge(i, k, amountOfParents);
        addEdge(g, p, amountOfParents);
        addEdge(g, u, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << o << u;
        QTest::newRow("MissingNodesHaveHightNesting") << amountOfParents
                                                      << a
                                                      << TreeCoverageAnalyzer::PartiallyCovered
                                                      << expectedMissingNodes;
    }

    // Тест 15: Разноуровневые проверки полупокрытия узла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Selected);
        Node* h = createNode("h", Node::Shape::Selected);
        Node* i = createNode("i", Node::Shape::Base);
        Node* j = createNode("j", Node::Shape::Base);
        Node* k = createNode("k", Node::Shape::Selected);
        Node* l = createNode("l", Node::Shape::Base);
        Node* m = createNode("m", Node::Shape::Selected);
        Node* n = createNode("n", Node::Shape::Selected);
        Node* o = createNode("o", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(a, d, amountOfParents);
        addEdge(b, f, amountOfParents);
        addEdge(b, g, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(e, h, amountOfParents);
        addEdge(e, i, amountOfParents);
        addEdge(c, k, amountOfParents);
        addEdge(c, j, amountOfParents);
        addEdge(j, m, amountOfParents);
        addEdge(j, l, amountOfParents);
        addEdge(d, n, amountOfParents);
        addEdge(d, o, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << i << l << o;
        QTest::newRow("DiffrentLevelPartiallyCoverage") << amountOfParents
                                                        << a
                                                        << TreeCoverageAnalyzer::PartiallyCovered
                                                        << expectedMissingNodes;
    }

    // Тест 16: Покрытие целевого узла зависит от листьев
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Base);
        Node* h = createNode("h", Node::Shape::Selected);
        Node* i = createNode("i", Node::Shape::Base);
        Node* j = createNode("j", Node::Shape::Selected);
        Node* k = createNode("k", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(c, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(e, g, amountOfParents);
        addEdge(g, h, amountOfParents);
        addEdge(g, i, amountOfParents);
        addEdge(i, j, amountOfParents);
        addEdge(i, k, amountOfParents);
        QTest::newRow("CoverageDependsOnLeaves") << amountOfParents
                                                 << a
                                                 << TreeCoverageAnalyzer::FullyCovered
                                                 << QSet<Node*>();
    }

    // Тест 17: Множество неотмеченных которых не хватает для покрытия графа
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* l = createNode("l", Node::Shape::Base);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        Node* h = createNode("h", Node::Shape::Base);
        Node* i = createNode("i", Node::Shape::Base);
        Node* j = createNode("j", Node::Shape::Base);
        Node* k = createNode("k", Node::Shape::Selected);
        addEdge(a, l, amountOfParents);
        addEdge(l, b, amountOfParents);
        addEdge(l, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(c, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(e, g, amountOfParents);
        addEdge(g, h, amountOfParents);
        addEdge(g, i, amountOfParents);
        addEdge(i, k, amountOfParents);
        addEdge(i, j, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << j << h << f << d << b;
        QTest::newRow("MultiBaseTypeNodesAreMissing") << amountOfParents
                                                      << a
                                                      << TreeCoverageAnalyzer::PartiallyCovered
                                                      << expectedMissingNodes;
    }

    // Тест 18: Вызов функции производится не для целевого узла, узел без покрытия
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << b;
        QTest::newRow("NotTargetNodeIsNotCovered") << amountOfParents
                                                      << b
                                                      << TreeCoverageAnalyzer::NotCovered
                                                      << expectedMissingNodes;
    }

    // Тест 19: Вызов функции производится не для целевого узла, узел имеет полу-покрытие
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        QSet<Node*> expectedMissingNodes;
        expectedMissingNodes << c;
        QTest::newRow("NotTargetNodeIsPartiallyCovered") << amountOfParents
                                                   << b
                                                   << TreeCoverageAnalyzer::PartiallyCovered
                                                   << expectedMissingNodes;
    }

    // Тест 20: Вызов функции производится не для целевого узла, узел имеет полу-покрытие
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        QTest::newRow("NotTargetNodeIsFullyCovered") << amountOfParents
                                                         << b
                                                         << TreeCoverageAnalyzer::FullyCovered
                                                         << QSet<Node*>();
    }
}

void Tests::analyzeZoneWithRedundant_test(){
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(Node*, node);
    QFETCH(REDUNDANT_NODES, expectedRedundantNodes);
    QFETCH(REDUNDANT_NODES, predefinedRedundantNodes);
    QFETCH(Node*, selectedNode);

    TreeCoverageAnalyzer analyzer;

    analyzer.redundantNodes = predefinedRedundantNodes;

    // Вызов метода
    analyzer.analyzeZoneWithRedundantNodes(node, selectedNode);

    // Выводом разницы при провале
    if (!QTest::qCompare(analyzer.redundantNodes, expectedRedundantNodes, "analyzer.redundantNodes", "expectedRedundantNodes", __FILE__, __LINE__)) {
        printNodeSetDifferenceForRedundant(analyzer.redundantNodes, expectedRedundantNodes, "redundantNodes");
    }

    // Проверка результатов
    QCOMPARE(analyzer.redundantNodes, expectedRedundantNodes);

    // Очистка
    analyzer.clearData();
}
void Tests::analyzeZoneWithRedundant_test_data(){
    QTest::addColumn<NODE_PARENT_HASH>("amountOfParents");
    QTest::addColumn<Node*>("node");
    QTest::addColumn<REDUNDANT_NODES>("expectedRedundantNodes");
    QTest::addColumn<REDUNDANT_NODES>("predefinedRedundantNodes");
    QTest::addColumn<Node*>("selectedNode");

    // Тест 1: Один узел, нет избыточных
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        QTest::newRow("NoRedundant") << amountOfParents
                                     << a
                                     << QSet<QPair<Node*, Node*>>()
                                     << QSet<QPair<Node*, Node*>>()
                                     << a;
    }

    // Тест 2: Один избыточный узел после отмеченного
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(b, c);
        QTest::newRow("OneRedundantNode") << amountOfParents
                                          << c
                                          << expectedRedundantNodes
                                          << QSet<QPair<Node*, Node*>>()
                                          << b;
    }

    // Тест 3: Множество избыточных узлов после отмеченного
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(b, c) << qMakePair(b, d);
        QTest::newRow("MultiRedundantNodes") << amountOfParents
                                             << c
                                             << expectedRedundantNodes
                                             << QSet<QPair<Node*, Node*>>()
                                             << b;
    }

    // Тест 4: Избыточные узлы находятся в разных ветках
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(c, e, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(b, d) << qMakePair(c, e);
        QSet<QPair<Node*, Node*>> predefinedRedundantNodes;
        predefinedRedundantNodes << qMakePair(b, d);
        QTest::newRow("RedundantNodesInDiffrentBranches") << amountOfParents
                                                          << e
                                                          << expectedRedundantNodes
                                                          << predefinedRedundantNodes
                                                          << c;
    }

    // Тест 5: Зона с избыточными узлами есть, но избыточных узлов нет
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        QTest::newRow("ZoneWithRedundantIsThereButNotRedundantNodes") << amountOfParents
                                                                      << c
                                                                      << QSet<QPair<Node*, Node*>>()
                                                                      << QSet<QPair<Node*, Node*>>()
                                                                      << b;
    }

    // Тест 6: Избыточный узел имеет высокую вложенность
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(b, f);
        QTest::newRow("RedundantNodesHasHightNesting") << amountOfParents
                                                       << c
                                                       << expectedRedundantNodes
                                                       << QSet<QPair<Node*, Node*>>()
                                                       << b;
    }

    // Тест 7: Не во всех ветках есть избыточный узел
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, e, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(e, f, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(b, d);
        QSet<QPair<Node*, Node*>> predefinedRedundantNodes;
        predefinedRedundantNodes << qMakePair(b, d);
        QTest::newRow("NotInAllBranchesHaveRedundantNode") << amountOfParents
                                                           << f
                                                           << expectedRedundantNodes
                                                           << predefinedRedundantNodes
                                                           << e;
    }

    // Тест 8: Избыточный узел до целевого
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* r = createNode("r", Node::Shape::Selected);
        addEdge(e, r, amountOfParents);
        addEdge(r, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(e, r);
        QTest::newRow("RedundantNodeBeforeTargetNode") << amountOfParents
                                                       << r
                                                       << expectedRedundantNodes
                                                       << QSet<QPair<Node*, Node*>>()
                                                       << e;
    }

    // Тест 9: Избыточный узел не находится в ветке с целевым
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* r = createNode("r", Node::Shape::Selected);
        addEdge(e, r, amountOfParents);
        addEdge(e, d, amountOfParents);
        addEdge(d, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(e, r);
        QSet<QPair<Node*, Node*>> predefinedRedundantNodes;
        predefinedRedundantNodes << qMakePair(e, r);
        QTest::newRow("RedundantNodeNotLocateInBranchWithTarget") << amountOfParents
                                                                  << d
                                                                  << expectedRedundantNodes
                                                                  << predefinedRedundantNodes
                                                                  << e;
    }

    // Тест 10: Комбинированное расположение избыточных узлов
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Selected);
        Node* r = createNode("r", Node::Shape::Selected);
        addEdge(e, r, amountOfParents);
        addEdge(e, d, amountOfParents);
        addEdge(d, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        addEdge(b, g, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(e, r) << qMakePair(e, d) << qMakePair(b, g);
        QSet<QPair<Node*, Node*>> predefinedRedundantNodes;
        predefinedRedundantNodes << qMakePair(e, r) << qMakePair(e, d);
        QTest::newRow("MixedLocateRedundantNodes") << amountOfParents
                                                   << g
                                                   << expectedRedundantNodes
                                                   << predefinedRedundantNodes
                                                   << b;
    }

    // Тест 11: Сложный граф с высокой вложенностью
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Base);
        Node* b = createNode("b", Node::Shape::Target);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* f = createNode("f", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Base);
        Node* h = createNode("h", Node::Shape::Selected);
        Node* i = createNode("i", Node::Shape::Base);
        Node* j = createNode("j", Node::Shape::Selected);
        Node* k = createNode("k", Node::Shape::Selected);
        Node* l = createNode("l", Node::Shape::Selected);
        Node* m = createNode("m", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, j, amountOfParents);
        addEdge(b, g, amountOfParents);
        addEdge(b, i, amountOfParents);
        addEdge(j, l, amountOfParents);
        addEdge(j, m, amountOfParents);
        addEdge(g, k, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(e, h, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(j, l) << qMakePair(c, e) << qMakePair(c, f) << qMakePair(c, h);
        QSet<QPair<Node*, Node*>> predefinedRedundantNodes;
        predefinedRedundantNodes << qMakePair(j, l);
        QTest::newRow("DifficultGraphWithHightNesting") << amountOfParents
                                                        << d
                                                        << expectedRedundantNodes
                                                        << predefinedRedundantNodes
                                                        << c;
    }

    // Тест 12: Все узлы в графе кроме корня отмечены
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Selected);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Selected);
        Node* d = createNode("d", Node::Shape::Selected);
        Node* e = createNode("e", Node::Shape::Selected);
        Node* f = createNode("f", Node::Shape::Selected);
        Node* g = createNode("g", Node::Shape::Selected);
        Node* h = createNode("h", Node::Shape::Selected);
        Node* i = createNode("i", Node::Shape::Selected);
        Node* j = createNode("j", Node::Shape::Selected);
        Node* k = createNode("k", Node::Shape::Selected);
        Node* l = createNode("l", Node::Shape::Selected);
        Node* m = createNode("m", Node::Shape::Selected);
        Node* n = createNode("n", Node::Shape::Selected);
        Node* o = createNode("o", Node::Shape::Selected);
        Node* p = createNode("p", Node::Shape::Selected);
        Node* r = createNode("r", Node::Shape::Selected);
        Node* z = createNode("z", Node::Shape::Target);
        addEdge(z, a, amountOfParents);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(a, d, amountOfParents);
        addEdge(b, e, amountOfParents);
        addEdge(e, f, amountOfParents);
        addEdge(f, g, amountOfParents);
        addEdge(c, h, amountOfParents);
        addEdge(c, i, amountOfParents);
        addEdge(h, j, amountOfParents);
        addEdge(i, k, amountOfParents);
        addEdge(i, l, amountOfParents);
        addEdge(d, m, amountOfParents);
        addEdge(m, n, amountOfParents);
        addEdge(m, o, amountOfParents);
        addEdge(m, p, amountOfParents);
        addEdge(p, r, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(a, b) << qMakePair(a, e) << qMakePair(a, f) << qMakePair(a, g) << qMakePair(a, c) << qMakePair(a, h) << qMakePair(a, j) << qMakePair(a, i) << qMakePair(a, k) << qMakePair(a, l) << qMakePair(a, d) << qMakePair(a, m) << qMakePair(a, n) << qMakePair(a, o) << qMakePair(a, p) << qMakePair(a, r);
        QSet<QPair<Node*, Node*>> predefinedRedundantNodes;
        predefinedRedundantNodes << qMakePair(a, b) << qMakePair(a, e) << qMakePair(a, f) << qMakePair(a, g) << qMakePair(a, c) << qMakePair(a, h) << qMakePair(a, j) << qMakePair(a, i) << qMakePair(a, k) << qMakePair(a, l);
        QTest::newRow("AllNodesSelectedExceptTarget") << amountOfParents
                                                      << d
                                                      << expectedRedundantNodes
                                                      << predefinedRedundantNodes
                                                      << a;
    }
}
