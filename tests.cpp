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
    qDebug() << "Разница в " << containerName << ":";
    if (!extraInActual.isEmpty()) {
        qDebug() << "Лишние узлы: " << extraActualNames;
    }
    if (!extraInExpected.isEmpty()) {
        qDebug() << "Пропущенные узлы: " << extraExpectedNames;
    }
}

void Tests::printNodeSetDifferenceForRedundant(const QSet<QPair<Node*, Node*>>& actual, const QSet<QPair<Node*, Node*>>& expected, const QString& containerName) {
    QSet<QPair<Node*, Node*>> extraInActual = actual - expected; // Пары которые есть в контейнере после вызова метода, но нет в ожидаемом контейнере
    QSet<QPair<Node*, Node*>> extraInExpected = expected - actual; // Пары, которые есть в ожидаемом контейнере с узлами, но нет в контейнере после вызова метода
    QString extraActualNames, extraExpectedNames;

    // Собираем имена для extraInActual
    for (const QPair<Node*, Node*>& pair : extraInActual) {
        Node* first = pair.first;
        Node* second = pair.second;
        QString firstName = first ? (!first->name.isEmpty() ? first->name : QString("Unnamed(0x%1)").arg((quintptr)first, 0, 16)) : "nullptr";
        QString secondName = second ? (!second->name.isEmpty() ? second->name : QString("Unnamed(0x%1)").arg((quintptr)second, 0, 16)) : "nullptr";
        extraActualNames += firstName + "-" + secondName + " ";
    }

    // Собираем имена для extraInExpected
    for (const QPair<Node*, Node*>& pair : extraInExpected) {
        Node* first = pair.first;
        Node* second = pair.second;
        QString firstName = first ? (!first->name.isEmpty() ? first->name : QString("Unnamed(0x%1)").arg((quintptr)first, 0, 16)) : "nullptr";
        QString secondName = second ? (!second->name.isEmpty() ? second->name : QString("Unnamed(0x%1)").arg((quintptr)second, 0, 16)) : "nullptr";
        extraExpectedNames += firstName + "-" + secondName + " ";
    }

    qDebug() << "Difference in" << containerName << ":";
    if (!extraInActual.isEmpty()) {
        qDebug() << "Extra in actual (not expected):" << extraActualNames.trimmed();
    }
    if (!extraInExpected.isEmpty()) {
        qDebug() << "Missing in actual (expected):" << extraExpectedNames.trimmed();
    }
}

void Tests::parseDOT_test(){
    QFETCH(QString, content);
    QFETCH(bool, shouldSucceed);
    QFETCH(int, expectedTreeMapSize);
    QFETCH(Error::ErrorType, expectedErrorType);
    QFETCH(QString, expectedErrorMessage);

    TreeCoverageAnalyzer analyzer;

    // Проверка результатов
    if (shouldSucceed) {
        try {
            analyzer.parseDOT(content);
            QCOMPARE(analyzer.treeMap.size(), expectedTreeMapSize);
            if (expectedTreeMapSize == 3) {
                QVERIFY(analyzer.treeMap.size() >= 3);
                QVERIFY(analyzer.treeMap[0] && analyzer.treeMap[0]->name == "a" && analyzer.treeMap[0]->shape == Node::Shape::Target);
                QVERIFY(analyzer.treeMap[1] && analyzer.treeMap[1]->name == "b" && analyzer.treeMap[1]->shape == Node::Shape::Selected);
                QVERIFY(analyzer.treeMap[2] && analyzer.treeMap[2]->name == "c" && analyzer.treeMap[2]->shape == Node::Shape::Base);
            }
        } catch (const Error& e) {
            QFAIL("Не ожидалось исключение для корректного случая");
        } catch (...) {
            QFAIL("Неизвестное исключение в корректном случае");
        }
    } else {
        try {
            analyzer.parseDOT(content);
            QFAIL("Ожидалось исключение для некорректного случая");
        } catch (const Error& e) {
            QCOMPARE(e.type, expectedErrorType);
            QCOMPARE(e.errMessage(), expectedErrorMessage);
        } catch (...) {
            QFAIL("Неизвестное исключение в некорректном случае");
        }
    }

    // Отчистка результатов
    analyzer.clearData();
}
void Tests::parseDOT_test_data(){
    QTest::addColumn<QString>("content");
    QTest::addColumn<bool>("shouldSucceed");
    QTest::addColumn<int>("expectedTreeMapSize");
    QTest::addColumn<Error::ErrorType>("expectedErrorType");
    QTest::addColumn<QString>("expectedErrorMessage");

    // Тест 1: Пустой файл
    QTest::newRow("EmptyFile") << ""
                               << false
                               << 0
                               << Error::EmptyFile
                               << QString("Файл пустой.");

    // Тест 2: Отсутствует целевой узел
    QTest::newRow("NoTargetNode") << "digraph test {\n"
                                                "a[shape=circle];\n"
                                                "b[shape=diamond];\n"
                                                "a->b;\n"
                                                "}"
                               << false
                               << 0
                               << Error::NoTargetNode
                               << QString("Некорректная ситуация, нет узла, для которого определяем покрытие. Добавьте узел с формой square.");

    // Тест 3: Ненаправленная связь
    QTest::newRow("UndirectedEdge") << "graph test {\n"
                                       "a[shape=square];\n"
                                       "b[shape=diamond];\n"
                                       "a--b;\n"
                                       "}"
                               << false
                               << 0
                               << Error::UndirectedEdge
                               << QString("Граф не является деревом, связи между узлами не направлены.");

    // Тест 4: Метка на связи
    QTest::newRow("EdgeLabel") << "digraph test {\n"
                                  "a[shape=square];\n"
                                  "b[shape=diamond];\n"
                                  "a->b[lable=\"test\"];\n"
                                  "}"
                               << false
                               << 0
                               << Error::EdgeLabel
                               << QString("На связи между узлом a и b задана метка. Уберите метку на связи.");

    // Тест 5: Неверная форма узла
    QTest::newRow("InvalidNodeShape") << "digraph test {\n"
                                         "a[shape=square];\n"
                                         "b[shape=star];\n"
                                         "a->b;\n"
                                         "}"
                               << false
                               << 0
                               << Error::InvalidNodeShape
                               << QString("Форма узла b не поддерживается программой. Измените ее на одну из предоставленных (square/diamond/oval/circle).");

    // Тест 6: Корректный граф
    QTest::newRow("CorrectGraph") << "digraph test {\n"
                                     "a[shape=square];\n"
                                     "b[shape=diamond];\n"
                                     "c[shape=circle];\n"
                                     "a->b;\n"
                                     "a->c;\n"
                                     "}"
                               << true
                               << 3
                               << Error::ErrorType(0)
                               << QString("");

    // Тест 7: Дополнительная метка узла
    QTest::newRow("ExtraLabel") << "digraph test {\n"
                                   "a[shape=square,lable=\"test\"];\n"
                                   "b[shape=diamond];\n"
                                   "a->b;\n"
                                   "}"
                               << false
                               << 0
                               << Error::ExtraLabel
                               << QString("На узле a задана метка. Уберите метку на узле.");

    // Тест 8: Комплексный
    QTest::newRow("ComplexCase") << "graph test {\n"
                                    "a[shape=circle,label=\"test\"];\n"
                                    "b[shape=star];\n"
                                    "a--b[label=\"test\"];\n"
                                    "}"
                               << false
                               << 0
                               << Error::NoTargetNode // Основная ошибка именно эта но сообщение влючает все
                               << QString("Некорректная ситуация, нет узла, для которого определяем покрытие. Добавьте узел с формой square.\n"
                                            "Граф не является деревом, связи между узлами не направлены.\n"
                                            "На связи между узлом a и b задана метка. Уберите метку на связи.\n"
                                            "Форма узла b не поддерживается программой. Измените ее на одну из представленных (square/diamond/oval/circle).\n"
                                            "На связи между узлом a и b задана метка. Уберите метку на связи./n");

}

void Tests::isConnectedOrHasMultiParents_test(){
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(Node*, rootNode);
    QFETCH(bool, expectedIsConnected);
    QFETCH(QSet<Node*>, expectedMultiParents);

    TreeCoverageAnalyzer analyzer;
    analyzer.root = rootNode;

    // Добавляем узлы из amountOfParents в treeMap
    for (auto it = amountOfParents.constBegin(); it != amountOfParents.constEnd(); ++it) {
        analyzer.treeMap.append(it.key());
    }
    if (rootNode && !analyzer.treeMap.contains(rootNode)) {
        analyzer.treeMap.append(rootNode);
    }

    // Вызов метода
    analyzer.isConnectedOrHasMultiParents(amountOfParents);

    // Проверка результатов
    QCOMPARE(analyzer.isConnected, expectedIsConnected);
    QCOMPARE(analyzer.multiParents, expectedMultiParents);

    // Очистка через clearData
    analyzer.clearData();
}
void Tests::isConnectedOrHasMultiParents_test_data(){
    QTest::addColumn<NODE_PARENT_HASH>("amountOfParents");
    QTest::addColumn<Node*>("rootNode");
    QTest::addColumn<bool>("expectedIsConnected");
    QTest::addColumn<QSet<Node*>>("expectedMultiParents");

    // Тест 1: Дерево состоящее из одного узла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        QTest::newRow("GraphConsistOfOneNode") << amountOfParents
                                               << a
                                               << true
                                               << QSet<Node*>();
    }

    // Тест 2: Узел с двумя родителями
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(c, d, amountOfParents);
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << d;
        QTest::newRow("NodeWithTwoParents") << amountOfParents
                                            << a
                                            << true
                                            << expectedMultiParents;
    }

    // Тест 3: Несвязный граф
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* g = createNode("g", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(c, g, amountOfParents);
        QTest::newRow("DisconnectedGraph") << amountOfParents
                                           << a
                                           << false
                                           << QSet<Node*>();
    }

    // Тест 4: Множественные родители у нескольких узлов
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* k = createNode("k", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, d, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(d, k, amountOfParents);
        addEdge(c, k, amountOfParents);
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << d << k;
        QTest::newRow("MultiNodesHaveMultiParents") << amountOfParents
                                                    << a
                                                    << true
                                                    << expectedMultiParents;
    }

    // Тест 5: Граф с циклом
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, a, amountOfParents);
        QTest::newRow("GraphWithCycle") << amountOfParents
                                        << a
                                        << true
                                        << QSet<Node*>();
    }

    // Тест 6: Граф с узлом без родителей (не корень)
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        QTest::newRow("GraphWithNodeWithoutParents") << amountOfParents
                                                     << a
                                                     << false
                                                     << QSet<Node*>();
    }

    // Тест 7: Граф где все узлы имеют 2 родителя
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, b, amountOfParents);
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << b << c;
        QTest::newRow("NonrootNodesHaveTwoParents") << amountOfParents
                                                    << a
                                                    << true
                                                    << expectedMultiParents;
    }

    // Тест 8: Граф с одним узлом и циклом на себя
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        addEdge(a, a, amountOfParents);
        QTest::newRow("OneNodeWithCycle") << amountOfParents
                                          << a
                                          << true
                                          << QSet<Node*>();
    }

    // Тест 9: Граф с летающим циклом
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(d, e, amountOfParents);
        addEdge(e, c, amountOfParents);
        QTest::newRow("GraphWithLevitateCycle") << amountOfParents
                                                << a
                                                << false // Исправлено: узлы c, d, e недостижимы
                                                << QSet<Node*>();
    }

    // Тест 10: Три родителя у узла
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Base);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        addEdge(a, b, amountOfParents);
        addEdge(a, c, amountOfParents);
        addEdge(a, d, amountOfParents);
        addEdge(b, d, amountOfParents);
        addEdge(c, d, amountOfParents);
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << d;
        QTest::newRow("ThreeParentsForNode") << amountOfParents
                                             << a
                                             << true
                                             << expectedMultiParents;
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
        QSet<Node*> expectedMultiParents;
        expectedMultiParents << c << g << r << l;
        QTest::newRow("GraphWithHightNestingAndMultiParents") << amountOfParents
                                                              << a
                                                              << true
                                                              << expectedMultiParents;
    }
}

void Tests::hasCycles_test(){
    QFETCH(QList<Node*>, nodes);
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(Node*, startNode);
    QFETCH(int, expectedCycleCount);
    QFETCH(QSet<QList<Node*>>, expectedCycles);

    TreeCoverageAnalyzer analyzer;
    QSet<QList<Node*>> cycles;
    QList<Node*> currentPath;

    // Вызов метода
    analyzer.hasCycles(startNode, cycles, amountOfParents, currentPath);

    // Проверка результатов
    QCOMPARE(cycles.size(), expectedCycleCount);
    if (expectedCycleCount > 0) {
        QVERIFY(cycles == expectedCycles);
    }

    // Отчистка данных
    analyzer.clearData();
}
void Tests::hasCycles_test_data(){
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
        QTest::newRow("MultiExtraNodesInDifferentBranches") << amountOfParents
                                                            << c
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
        addEdge(d, e, amountOfParents);
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
}

void Tests::analyzeZoneWithRedundant_test(){
    QFETCH(NODE_PARENT_HASH, amountOfParents);
    QFETCH(Node*, node);
    QFETCH(REDUNDANT_NODES, expectedRedundantNodes);

    TreeCoverageAnalyzer analyzer;

    // Выводом разницы при провале
    if (!QTest::qCompare(analyzer.redundantNodes, expectedRedundantNodes, "analyzer.redundantNodes", "expectedRedundantNodes", __FILE__, __LINE__)) {
        printNodeSetDifferenceForRedundant(analyzer.redundantNodes, expectedRedundantNodes, "redundantNodes");
    }

    // Вызов метода
    analyzer.analyzeZoneWithExtraNodes(node);

    // Проверка результатов
    QCOMPARE(analyzer.redundantNodes, expectedRedundantNodes);

    // Очистка
    analyzer.clearData();
}
void Tests::analyzeZoneWithRedundant_test_data(){
    QTest::addColumn<NODE_PARENT_HASH>("amountOfParents");
    QTest::addColumn<Node*>("node");
    QTest::addColumn<REDUNDANT_NODES>("expectedRedundantNodes");

    // Тест 1: Один узел, нет избыточных
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        QTest::newRow("NoRedundant") << amountOfParents
                                     << a
                                     << QSet<QPair<Node*, Node*>>();
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
                                          << a
                                          << expectedRedundantNodes;
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
        expectedRedundantNodes << qMakePair(b, c) << qMakePair(c, d);
        QTest::newRow("MultiRedundantNodes") << amountOfParents
                                             << a
                                             << expectedRedundantNodes;
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
        QTest::newRow("RedundantNodesInDiffrentBranches") << amountOfParents
                                                          << a
                                                          << expectedRedundantNodes;
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
                                                                      << a
                                                                      << QSet<QPair<Node*, Node*>>();
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
        expectedRedundantNodes << qMakePair(d, f);
        QTest::newRow("RedundantNodesHasHightNesting") << amountOfParents
                                                       << a
                                                       << expectedRedundantNodes;
    }

    // Тест 7: Не во всех ветках есть избыточный узел
    {
        NODE_PARENT_HASH amountOfParents;
        Node* a = createNode("a", Node::Shape::Target);
        Node* b = createNode("b", Node::Shape::Selected);
        Node* c = createNode("c", Node::Shape::Base);
        Node* d = createNode("d", Node::Shape::Base);
        Node* e = createNode("e", Node::Shape::Base);
        Node* f = createNode("f", Node::Shape::Selected);
        addEdge(a, b, amountOfParents);
        addEdge(a, e, amountOfParents);
        addEdge(b, c, amountOfParents);
        addEdge(c, d, amountOfParents);
        addEdge(e, f, amountOfParents);
        QSet<QPair<Node*, Node*>> expectedRedundantNodes;
        expectedRedundantNodes << qMakePair(b, d);
        QTest::newRow("NotInAllBranchesHaveRedundantNode") << amountOfParents
                                                           << a
                                                           << expectedRedundantNodes;
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
                                                       << a
                                                       << expectedRedundantNodes;
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
        QTest::newRow("RedundantNodeNotLocateInBranchWithTarget") << amountOfParents
                                                                  << a
                                                                  << expectedRedundantNodes;
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
        QTest::newRow("MixedLocateRedundantNodes") << amountOfParents
                                                   << a
                                                   << expectedRedundantNodes;
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
        QTest::newRow("DifficultGraphWithHightNesting") << amountOfParents
                                                        << a
                                                        << expectedRedundantNodes;
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
        QTest::newRow("AllNodesSelectedExceptTarget") << amountOfParents
                                                      << z
                                                      << expectedRedundantNodes;
    }
}
