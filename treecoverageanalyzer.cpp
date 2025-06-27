/*!
* \file
* \brief Файл содержит реализацию функций, использующихся в ходе работы программы GetConclusionAboutNodeCoverage.
*/
#include "treecoverageanalyzer.h"

TreeCoverageAnalyzer::TreeCoverageAnalyzer() {
    clearData();
}

TreeCoverageAnalyzer::~TreeCoverageAnalyzer() {
    clearData();
}

void TreeCoverageAnalyzer::writeErrorsToFileAndExit(const QString& filename) {
    if (errors.isEmpty()) {
        return; // Если ошибок нет, продолжаем выполнение
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Если не удалось открыть файл, выводим в консоль и завершаем
        QTextStream stderrStream(stderr);
        stderrStream << "Ошибка: не удалось открыть файл " << filename << " для записи.\n";
        for (const Error& error : errors) {
            stderrStream << "Ошибка: " << error.errMessage() << "\n";
        }
        exit(1);
    }

    QTextStream out(&file);
    out << "Отчет об ошибках:\n";
    for (const Error& error : errors) {
        out << "Ошибка: " << error.errMessage() << "\n";
    }
    file.close();

    exit(1); // Завершаем программу
}

void TreeCoverageAnalyzer::checkErrorsAfterParseDOT() {
    writeErrorsToFileAndExit("parse_errors.txt");
}

void TreeCoverageAnalyzer::checkErrorsAfterTreeGraphTakeErrors() {
    writeErrorsToFileAndExit("graph_errors.txt");
}

void TreeCoverageAnalyzer::parseDOT(const QString& content) {
    clearData();

    if (content.trimmed().isEmpty()) {
        errors.append(Error(Error::EmptyFile));
        return;
    }

    // Собираем все имена узлов и их атрибуты
    QRegularExpression nodeRegex(R"((\w+(?:,\w+)*)\s*\[(.*?)\]\s*;|(\w+(?:,\w+)*)\s*;)");
    QRegularExpressionMatchIterator nodeIter = nodeRegex.globalMatch(content);
    QStringList nodeNames;
    QMap<QString, QString> nodeAttributes; // Для хранения атрибутов

    // Обработка узлов
    while (nodeIter.hasNext()) {
        QRegularExpressionMatch match = nodeIter.next();
        QString nodeList = match.captured(1).isEmpty() ? match.captured(3) : match.captured(1);
        QString attributesStr = match.captured(2);
        QStringList nodes = nodeList.split(',');
        for (const QString& name : nodes) {
            QString trimmedName = name.trimmed();
            if (!nodeNames.contains(trimmedName)) {
                nodeNames.append(trimmedName);
            }
            if (!attributesStr.isEmpty()) {
                nodeAttributes[trimmedName] = attributesStr;
            }
        }
    }

    // Сортируем имена узлов для детерминированного порядка
    nodeNames.sort();

    bool hasTargetNode = false;
    QHash<QString, Node*> nodeNameMap;

    // Создаём узлы
    for (const QString& name : nodeNames) {
        Node::Shape nodeShape = Node::Base;
        bool shapeValid = true;
        QString attributesStr = nodeAttributes.value(name);

        if (!attributesStr.isEmpty()) {
            QRegularExpression attrRegex(R"(\s*shape\s*=\s*(\w+|"[^"]*"|'[^']*')\s*(?:,\s*label\s*=\s*(\w+|"[^"]*"|'[^']*')\s*)?)");
            QRegularExpressionMatch attrMatch = attrRegex.match(attributesStr);
            QMap<QString, QString> attrMap;

            if (attrMatch.hasMatch()) {
                QString shapeValue = attrMatch.captured(1);
                if (shapeValue.startsWith('"') && shapeValue.endsWith('"')) {
                    shapeValue = shapeValue.mid(1, shapeValue.length() - 2);
                }
                else if (shapeValue.startsWith('\'') && shapeValue.endsWith('\'')) {
                    shapeValue = shapeValue.mid(1, shapeValue.length() - 2);
                }
                attrMap["shape"] = shapeValue;

                if (attrMatch.captured(2).isEmpty()) {
                    // Если label отсутствует, ничего не делаем
                }
                else {
                    QString labelValue = attrMatch.captured(2);
                    if (labelValue.startsWith('"') && labelValue.endsWith('"')) {
                        labelValue = labelValue.mid(1, labelValue.length() - 2);
                    }
                    else if (labelValue.startsWith('\'') && labelValue.endsWith('\'')) {
                        labelValue = labelValue.mid(1, labelValue.length() - 2);
                    }
                    attrMap["label"] = labelValue;
                }
            }
            else {
                errors.append(Error(Error::ExtraLabel, QString("для узла %1: %2").arg(name, attributesStr)));
                continue;
            }

            QString shape = attrMap.value("shape").toLower();
            if (!shape.isEmpty()) {
                if (shape == "square") {
                    nodeShape = Node::Target;
                    hasTargetNode = true;
                }
                else if (shape == "diamond") {
                    nodeShape = Node::Selected;
                }
                else {
                    shapeValid = false;
                    errors.append(Error(Error::InvalidNodeShape, name));
                    continue;
                }
            }
            if (attrMap.contains("label")) {
                errors.append(Error(Error::ExtraLabel, QString("для узла %1: label=\"%2\"").arg(name, attrMap["label"])));
            }
        }

        Node* node = new Node(name, nodeShape);
        treeMap.append(node);
        nodeNameMap[name] = node;
    }

    // Обработка рёбер (остальной код остаётся без изменений)
    QRegularExpression edgeRegex(R"((\w+)\s*->\s*(\w+)\s*(?:\[([^\]]+)\])?\s*;)");
    QRegularExpressionMatchIterator edgeIter = edgeRegex.globalMatch(content);
    while (edgeIter.hasNext()) {
        QRegularExpressionMatch match = edgeIter.next();
        QString parentName = match.captured(1);
        QString childName = match.captured(2);
        QString edgeAttrsStr = match.captured(3);

        if (!nodeNameMap.contains(parentName)) {
            Node* parent = new Node(parentName, Node::Base);
            treeMap.append(parent);
            nodeNameMap[parentName] = parent;
        }
        if (!nodeNameMap.contains(childName)) {
            Node* child = new Node(childName, Node::Base);
            treeMap.append(child);
            nodeNameMap[childName] = child;
        }

        Node* parent = nodeNameMap.value(parentName);
        Node* child = nodeNameMap.value(childName);

        parent->children.append(child);
        amountOfParents[child] = amountOfParents.value(child, 0) + 1;

        if (!edgeAttrsStr.isEmpty()) {
            QRegularExpression attrRegex(R"(\s*label\s*=\s*(\w+|"[^"]*"|'[^']*')\s*)");
            QRegularExpressionMatch attrMatch = attrRegex.match(edgeAttrsStr);
            if (attrMatch.hasMatch()) {
                errors.append(Error(Error::EdgeLabel, QString("%1 и %2").arg(parentName, childName)));
            }
            else {
                errors.append(Error(Error::ExtraLabel, QString("для ребра %1->%2: %3").arg(parentName, childName, edgeAttrsStr)));
            }
        }
    }

    // Обработка ненаправленных рёбер (без изменений)
    QRegularExpression undirectedEdgeRegex(R"((\w+)\s*--\s*(\w+)\s*(?:\[([^\]]+)\])?\s*;)");
    QRegularExpressionMatchIterator undirectedIter = undirectedEdgeRegex.globalMatch(content);
    bool hasUndirected = false;
    while (undirectedIter.hasNext()) {
        hasUndirected = true;
        QRegularExpressionMatch match = undirectedIter.next();
        QString node1Name = match.captured(1);
        QString node2Name = match.captured(2);
        QString edgeAttrsStr = match.captured(3);

        if (!nodeNameMap.contains(node1Name)) {
            Node* node1 = new Node(node1Name, Node::Base);
            treeMap.append(node1);
            nodeNameMap[node1Name] = node1;
        }
        if (!nodeNameMap.contains(node2Name)) {
            Node* node2 = new Node(node2Name, Node::Base);
            treeMap.append(node2);
            nodeNameMap[node2Name] = node2;
        }

        Node* node1 = nodeNameMap.value(node1Name);
        Node* node2 = nodeNameMap.value(node2Name);

        node1->children.append(node2);
        node2->children.append(node1);
        amountOfParents[node2] = amountOfParents.value(node2, 0) + 1;
        amountOfParents[node1] = amountOfParents.value(node1, 0) + 1;

        if (!edgeAttrsStr.isEmpty()) {
            QRegularExpression attrRegex(R"(\s*label\s*=\s*(\w+|"[^"]*"|'[^']*')\s*)");
            QRegularExpressionMatch attrMatch = attrRegex.match(edgeAttrsStr);
            if (attrMatch.hasMatch()) {
                errors.append(Error(Error::EdgeLabel, QString("%1 и %2").arg(node1Name, node2Name)));
            }
            else {
                errors.append(Error(Error::ExtraLabel, QString("для ребра %1--%2: %3").arg(node1Name, node2Name, edgeAttrsStr)));
            }
        }
    }

    if (hasUndirected) {
        errors.append(Error(Error::UndirectedEdge));
    }

    if (!hasTargetNode) {
        errors.append(Error(Error::NoTargetNode));
        return;
    }
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
    errors.clear();


    // Сбрасываем флаги
    isConnected = false;
}

void TreeCoverageAnalyzer::fillHash(QList<Node*>& treeMap, QHash<Node*, int>& amountOfParents){
    // 1. Инициализируем хэш-таблицу, устанавливая количество родителей в 0 для каждого узла
    for (Node* node : treeMap) {
        amountOfParents[node] = 0; // Для каждого отдельного узла заполняем кол-во родителей
    }

    // 2. Обходим все узлы и увеличиваем счетчик родителей для каждого дочернего узла
    for (Node* node : treeMap) {
        for (Node* child : node->children) {
            amountOfParents[child]++; // Увеличиваем счетчик родителей у детей узла
        }
    }

    // 3. Проверяем связанность графа, наличие узлов с несколькими родителями и наличие циклов в графе
    treeGraphTakeErrors(amountOfParents);
}

void TreeCoverageAnalyzer::treeGraphTakeErrors(QHash<Node*, int>& amountOfParents){
    // 1. Проверяем узлы на наличие нескольких родителей и находим корневые узлы
    for (auto it = amountOfParents.constBegin(); it != amountOfParents.constEnd(); ++it) {
        Node* node = it.key();
        int parentCount = it.value();
        // Заполняем multiParents или находим корни
        if (parentCount >= 2) {
            multiParents.insert(node);
            errors.append(Error(Error::MultiParents, node->name));
        }
        else if (parentCount == 0) {
            rootNodes.insert(node);
        }
    }


    // 2. Если корневых узлов нет, добавляем первый узел из таблицы как корень
    if (rootNodes.isEmpty() && !amountOfParents.isEmpty()) {
        Node* firstNode = amountOfParents.constBegin().key();
        rootNodes.insert(firstNode);
    }

    // 3. Проверяем цикличность и собираем посещенные узлы
    QSet<QSet<Node*>> allVisitedNodes;
    for (Node* root : rootNodes) {
        QList<Node*> currentPath;
        hasCycles(root, currentPath);
        allVisitedNodes.insert(visitedNodes);
        visitedNodes.clear();
    }

    // 4. Проверяем связанность графа
    // Проверяем связанность графа
    isConnected = !allVisitedNodes.isEmpty();
    if (isConnected) {
        // Проверяем наличие общих узлов
        QSet<Node*> commonNodes;
        bool firstSet = true;
        for (const QSet<Node*>& visitedSet : allVisitedNodes) {
            if (firstSet) {
                commonNodes = visitedSet;
                firstSet = false;
            }
            else {
                commonNodes.intersect(visitedSet);
            }
            if (commonNodes.isEmpty()) {
                isConnected = false;
                break;
            }
        }
        // Проверяем, что объединение покрывает все узлы
        if (isConnected) {
            QSet<Node*> allNodesVisited;
            for (const QSet<Node*>& visitedSet : allVisitedNodes) {
                allNodesVisited.unite(visitedSet);
            }
            isConnected = (allNodesVisited.size() == amountOfParents.size());
        }
    }
    if (!isConnected) {
        errors.append(Error(Error::DisconnectedGraph));
    }
}

void TreeCoverageAnalyzer::hasCycles(Node* node, QList<Node*>& currentPath) {
    // 1. Если текущий узел NULL, вернуться
    if (!node) {
        return;
    }

    // 2. Если узел уже находится в currentPath, значит найден цикл
    if (currentPath.contains(node)) {
        // 2.1. Извлечь индекс начала цикла
        int cycleStartIndex = currentPath.indexOf(node);
        // 2.2. Сохранить цикл (часть пути от начала цикла до конца currentPath)
        QList<Node*> cycle;
        QString cycleNames;
        for (int i = cycleStartIndex; i < currentPath.size(); ++i) {
            cycle.append(currentPath[i]);
            cycleNames += currentPath[i]->name + " ";
        }
        cycle.append(node); // Завершаем цикл
        cycleNames += node->name;
        cycles.insert(cycle);
        if (!errors.contains(Error(Error::Cycle))) {
            errors.append(Error(Error::Cycle, cycleNames.trimmed()));
        }
        return;
    }

    // 3. Добавить текущий узел в currentPath и visitedNodes
    currentPath.append(node);
    visitedNodes.insert(node);

    // 4. Для каждого дочернего узла вызвать hasCycles
    for (Node* child : node->children) {
        hasCycles(child, currentPath);
    }

    // 5. Удалить текущий узел из currentPath при возврате из рекурсии
    currentPath.removeLast();
}

void TreeCoverageAnalyzer::analyzeTreeCoverage(){
    // Проверяем что граф соответсвует дереву
    if(errors.isEmpty()){
        Node* root = *rootNodes.begin(); // Так как граф соответствует дереву, понимаем что корень у дерева всего лишь один
        analyzeZoneWithExtraNodes(root); // Вызываем анализ зоны с возможными лишними узлами
    }

    getResult(); // Формуруем результат
}

void TreeCoverageAnalyzer::analyzeZoneWithExtraNodes(Node* node){
    // 1 Если текущий узел равен NULL, вернуться
    if (!node) {
        return;
    }

    // 2 Если текущий узел имеет тип Target
    if (node->shape == Node::Target) {
        // 2.1 Вызвать analyzeZoneWithMissingNodes для этого узла
        analyzeZoneWithMissingNodes(node);
        // 2.2 Вернуться из функции
        return;
    }

    // 3 Если текущий узел имеет тип Selected и до этого не был встречен целевой узел
    if (node->shape == Node::Selected) {
        // 3.1 Добавить его в список extraNodes
        extraNodes.insert(node);
        // 3.2 Для каждого дочернего узла вызвать analyzeZoneWithRedundantNodes
        for (Node* child : node->children) {
            analyzeZoneWithRedundantNodes(child, node);
        }
        // 3.3 Вернуться из рекурсии
        return;
    }

    // 4 Для каждого дочернего узла
    for (Node* child : node->children) {
        // 4.1 Рекурсивно обойти часть дерева до целевого узла
        analyzeZoneWithExtraNodes(child);
    }
}

TreeCoverageAnalyzer::CoverageStatus TreeCoverageAnalyzer::analyzeZoneWithMissingNodes(Node* node) {
    // 1. Если текущий узел равен NULL, вернуть NotCovered
    if (!node) {
        return NotCovered;
    }

    // 2. Если узел имеет тип Target
    if (node->shape == Node::Shape::Target) {
        // Если у целевого узла нет детей, возвращаем PartiallyCovered
        if (node->children.isEmpty()) {
            return PartiallyCovered;
        }

        // 2.1. Рекурсивно проверить всех детей
        bool allChildrenFullyCovered = true;
        for (Node* child : node->children) {
            CoverageStatus childStatus = analyzeZoneWithMissingNodes(child);
            // 2.2. Если хотя бы один ребенок не FullyCovered, устанавливаем флаг в false
            if (childStatus != FullyCovered) {
                allChildrenFullyCovered = false;
            }
        }

        // 2.2.1. Если все дети FullyCovered, вернуть FullyCovered
        if (allChildrenFullyCovered) {
            return FullyCovered;
        }
        // 2.3. Иначе вернуть PartiallyCovered
        return PartiallyCovered;
    }

    // 3. Если узел имеет тип Selected
    else if (node->shape == Node::Shape::Selected) {
        // 3.1. Для всех потомков вызвать analyzeZoneWithRedundantNodes
        for (Node* child : node->children) {
            analyzeZoneWithRedundantNodes(child, node);
        }
        // 3.2. Вернуть FullyCovered
        return FullyCovered;
    }

    // 4. Иначе (узел типа Base)
    else {
        // Если узел не имеет детей, он считается NotCovered, и добавляем его в missingNodes
        if (node->children.isEmpty()) {
            missingNodes.insert(node);
            return NotCovered;
        }

        bool allFullyCovered = true;
        bool allNotCovered = true;
        bool hasFullyOrPartiallyCovered = false;
        QSet<Node*> notCoveredChildren;

        // 4.1. Проверяем всех детей
        for (Node* child : node->children) {
            CoverageStatus childStatus = analyzeZoneWithMissingNodes(child);
            if (childStatus == FullyCovered) {
                allNotCovered = false;
                hasFullyOrPartiallyCovered = true;
            }
            else if (childStatus == PartiallyCovered) {
                allFullyCovered = false;
                allNotCovered = false;
                hasFullyOrPartiallyCovered = true;
            }
            else if (childStatus == NotCovered) {
                allFullyCovered = false;
                notCoveredChildren.insert(child);
            }
        }

        // 4.1. Если все дети FullyCovered
        if (allFullyCovered) {
            return FullyCovered;
        }

        // 4.2. Если все дети NotCovered
        if (allNotCovered) {
            // Очищаем missingNodes от детей, добавляем текущий узел
            for (Node* child : node->children) {
                missingNodes.remove(child);
            }
            missingNodes.insert(node);
            return NotCovered;
        }

        // 4.3. Если хотя бы один из детей FullyCovered или PartiallyCovered
        if (hasFullyOrPartiallyCovered) {
            // 4.3.1. Добавить всех детей с NotCovered в missingNodes
            missingNodes.unite(notCoveredChildren);
            // 4.3.2. Вернуть PartiallyCovered
            return PartiallyCovered;
        }

        // Этот случай не должен произойти, но для полноты возвращаем NotCovered
        return NotCovered;
    }
}

void TreeCoverageAnalyzer::analyzeZoneWithRedundantNodes(Node* node, Node* selectedNode) {
    // 1. Если текущий узел равен NULL, вернуться
    if (!node) {
        return;
    }

    // 2. Если узел имеет тип Target
    if (node->shape == Node::Target) {
        // 2.1. Для каждого дочернего узла текущего узла рекурсивно вызвать analyzeZoneWithMissingNodes
        for (Node* child : node->children) {
            analyzeZoneWithMissingNodes(child);
        }
    }

    // 3. Иначе
    else {
        // 3.1. Если узел имеет тип Selected
        if (node->shape == Node::Selected) {
            // 3.1.1. Добавить его в список redundantNodes как пару (selectedNode, node)
            redundantNodes.insert(qMakePair(selectedNode, node));
        }
        // 3.2. Для каждого дочернего узла вызвать analyzeZoneWithRedundantNodes
        for (Node* child : node->children) {
            analyzeZoneWithRedundantNodes(child, selectedNode);
        }
    }
}

void TreeCoverageAnalyzer::getResult() const {
    // Находим целевой узел
    Node* targetNode = nullptr;
    for (Node* node : treeMap) {
        if (node->shape == Node::Target) {
            targetNode = node;
            break;
        }
    }

    // Открываем файл для записи
    QFile file("coverage_result.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stderrStream(stderr);
        stderrStream << "Ошибка: не удалось открыть файл coverage_result.txt для записи.\n";
        exit(1);
    }
    QTextStream out(&file);

    bool hasErrors = false;

    // 1. Проверка наличия лишних узлов (узлы, не являющиеся потомками целевого узла)
    if (!extraNodes.isEmpty()) {
        QString extraNodeNames;
        for (Node* node : extraNodes) {
            extraNodeNames += node->name + " ";
        }
        extraNodeNames = extraNodeNames.trimmed();
        out << QString("Отмеченный узел %1 не является потомком целевого узла %2.\n").arg(extraNodeNames, targetNode->name);
        hasErrors = true;
    }

    // 2. Проверка наличия избыточных узлов (redundantNodes)
    if (!redundantNodes.isEmpty()) {
        QString ancestorNodeNames;
        QString redundantNodeNames;
        for (const QPair<Node*, Node*>& pair : redundantNodes) {
            Node* ancestor = pair.first;
            Node* descendant = pair.second;
            ancestorNodeNames += ancestor->name + " ";
            redundantNodeNames += descendant->name + " ";
        }
        redundantNodeNames = redundantNodeNames.trimmed();
        out << QString("Предок %1 отмеченного узла %2 тоже отмечен, следует не отмечать детей, если отмечен их предок.\n").arg(ancestorNodeNames, redundantNodeNames);
        hasErrors = true;
    }

    // 3. Проверка наличия узлов, которых не хватает для покрытия (missingNodes)
    if (!missingNodes.isEmpty()) {
        QString missingNodeNames;
        for (Node* node : missingNodes) {
            missingNodeNames += node->name + " ";
        }
        missingNodeNames = missingNodeNames.trimmed();
        out << QString("Узел %1 – не покрыт, следует отметить узлы %2 для того чтобы узел %1 стал покрытым.\n").arg(targetNode->name, missingNodeNames);
        hasErrors = true;
    }

    // 4. Если ошибок нет, возвращаем сообщение об успешном покрытии
    if (!hasErrors) {
        QString selectedNodeNames;
        for (Node* node : treeMap) {
            if (node->shape == Node::Selected) {
                selectedNodeNames += node->name + " ";
            }
        }
        selectedNodeNames = selectedNodeNames.trimmed();
        if (selectedNodeNames.isEmpty()) {
            out << QString("Целевой узел %1 покрыт.\n").arg(targetNode->name);
        }
        else {
            out << QString("Помеченные узлы %1 покрывают вышележащий узел %2.\n").arg(selectedNodeNames, targetNode->name);
        }
    }

    file.close();
}
