#include "treecoverageanalyzer.h"

TreeCoverageAnalyzer::TreeCoverageAnalyzer() {
    clearData();
}

TreeCoverageAnalyzer::~TreeCoverageAnalyzer() {
    clearData();
}

void TreeCoverageAnalyzer::parseDOT(const QString& content) {
    clearData(); // Убеждаемся, что данные очищаются перед парсингом

    if (content.trimmed().isEmpty()) {
        errors.append(Error(Error::EmptyFile));
        return;
    }

    // Гибкое регулярное выражение для узлов
    QRegularExpression nodeRegex(R"((\w+(?:,\w+)*)\s*\[(.*?)\]\s*;)");
    QRegularExpressionMatchIterator nodeIter = nodeRegex.globalMatch(content);

    bool hasTargetNode = false;
    QHash<QString, Node*> nodeNameMap;

    while (nodeIter.hasNext()) {
        QRegularExpressionMatch match = nodeIter.next();
        QString nodeList = match.captured(1);
        QString attributesStr = match.captured(2);

        QStringList nodes = nodeList.split(',');
        Node::Shape nodeShape = Node::Base;
        bool shapeValid = false;

        // Регулярное выражение для атрибутов узлов: shape и, возможно, label
        QRegularExpression attrRegex(R"(\s*shape\s*=\s*(\w+|"[^"]*"|'[^']*')\s*(?:,\s*label\s*=\s*(\w+|"[^"]*"|'[^']*')\s*)?)");
        QRegularExpressionMatch attrMatch = attrRegex.match(attributesStr);
        QMap<QString, QString> attrMap;

        if (attrMatch.hasMatch()) {
            QString shapeValue = attrMatch.captured(1);
            if (shapeValue.startsWith('"') && shapeValue.endsWith('"')) {
                shapeValue = shapeValue.mid(1, shapeValue.length() - 2);
            } else if (shapeValue.startsWith('\'') && shapeValue.endsWith('\'')) {
                shapeValue = shapeValue.mid(1, shapeValue.length() - 2);
            }
            attrMap["shape"] = shapeValue;

            if (attrMatch.captured(2).isEmpty()) {
                // Если label отсутствует, ничего не делаем
            } else {
                QString labelValue = attrMatch.captured(2);
                if (labelValue.startsWith('"') && labelValue.endsWith('"')) {
                    labelValue = labelValue.mid(1, labelValue.length() - 2);
                } else if (labelValue.startsWith('\'') && labelValue.endsWith('\'')) {
                    labelValue = labelValue.mid(1, labelValue.length() - 2);
                }
                attrMap["label"] = labelValue;
            }
        } else {
            // Если строка атрибутов не соответствует ожидаемому формату
            for (const QString& name : nodes) {
                errors.append(Error(Error::ExtraLabel, QString("%1 имеет некорректные атрибуты: %2").arg(name.trimmed(), attributesStr)));
            }
            continue;
        }

        // Проверяем атрибут shape
        QString shape = attrMap.value("shape").toLower();
        if (!shape.isEmpty()) {
            if (shape == "square") {
                nodeShape = Node::Target;
                hasTargetNode = true;
                shapeValid = true;
            } else if (shape == "diamond") {
                nodeShape = Node::Selected;
                shapeValid = true;
            } else if (shape == "circle") {
                nodeShape = Node::Base;
                shapeValid = true;
            }
            if (!shapeValid) {
                for (const QString& name : nodes) {
                    errors.append(Error(Error::InvalidNodeShape, name.trimmed()));
                }
                continue;
            }
        }

        // Проверяем наличие label (ExtraLabel для узлов)
        if (attrMap.contains("label")) {
            for (const QString& name : nodes) {
                errors.append(Error(Error::ExtraLabel, QString("%1 label=\"%2\"").arg(name.trimmed(), attrMap["label"])));
            }
        }

        for (const QString& name : nodes) {
            QString trimmedName = name.trimmed();
            if (nodeNameMap.contains(trimmedName)) {
                continue;
            }
            Node* node = new Node(trimmedName, nodeShape);
            treeMap.append(node);
            nodeNameMap[trimmedName] = node;
        }
    }

    if (!hasTargetNode) {
        errors.append(Error(Error::NoTargetNode));
        return;
    }

    // Обработка направленных связей
    QRegularExpression edgeRegex(R"((\w+)\s*->\s*(\w+)\s*(?:\[([^\]]+)\])?\s*;)");
    QRegularExpressionMatchIterator edgeIter = edgeRegex.globalMatch(content);

    while (edgeIter.hasNext()) {
        QRegularExpressionMatch match = edgeIter.next();
        QString parentName = match.captured(1);
        QString childName = match.captured(2);
        QString edgeAttrsStr = match.captured(3);

        Node* parent = nodeNameMap.value(parentName);
        Node* child = nodeNameMap.value(childName);

        if (!parent || !child) {
            continue;
        }

        parent->children.append(child);
        amountOfParents[child] = amountOfParents.value(child, 0) + 1;

        // Проверяем атрибуты ребра: только label
        if (!edgeAttrsStr.isEmpty()) {
            QRegularExpression attrRegex(R"(\s*label\s*=\s*(\w+|"[^"]*"|'[^']*')\s*)");
            QRegularExpressionMatch attrMatch = attrRegex.match(edgeAttrsStr);
            if (attrMatch.hasMatch()) {
                errors.append(Error(Error::EdgeLabel, QString("%1 и %2").arg(parentName, childName)));
            } else {
                errors.append(Error(Error::ExtraLabel, QString("Ребро %1->%2 имеет некорректные атрибуты: %3").arg(parentName, childName, edgeAttrsStr)));
            }
        }
    }

    // Обработка ненаправленных рёбер
    QRegularExpression undirectedEdgeRegex(R"((\w+)\s*--\s*(\w+)\s*(?:\[([^\]]+)\])?\s*;)");
    QRegularExpressionMatchIterator undirectedIter = undirectedEdgeRegex.globalMatch(content);

    bool hasUndirected = false;
    while (undirectedIter.hasNext()) {
        hasUndirected = true;
        QRegularExpressionMatch match = undirectedIter.next();
        QString node1Name = match.captured(1);
        QString node2Name = match.captured(2);
        QString edgeAttrsStr = match.captured(3);

        Node* node1 = nodeNameMap.value(node1Name);
        Node* node2 = nodeNameMap.value(node2Name);

        if (!node1 || !node2) {
            continue;
        }

        node1->children.append(node2);
        node2->children.append(node1);
        amountOfParents[node2] = amountOfParents.value(node2, 0) + 1;
        amountOfParents[node1] = amountOfParents.value(node1, 0) + 1;

        // Проверяем атрибуты ребра: только label
        if (!edgeAttrsStr.isEmpty()) {
            QRegularExpression attrRegex(R"(\s*label\s*=\s*(\w+|"[^"]*"|'[^']*')\s*)");
            QRegularExpressionMatch attrMatch = attrRegex.match(edgeAttrsStr);
            if (attrMatch.hasMatch()) {
                errors.append(Error(Error::EdgeLabel, QString("%1 и %2").arg(node1Name, node2Name)));
            } else {
                errors.append(Error(Error::ExtraLabel, QString("Ребро %1--%2 имеет некорректные атрибуты: %3").arg(node1Name, node2Name, edgeAttrsStr)));
            }
        }
    }

    if (hasUndirected) {
        errors.append(Error(Error::UndirectedEdge));
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
    nodeIsCovered = false;
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
        } else if (parentCount == 0) {
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
            } else {
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
        for (int i = cycleStartIndex; i < currentPath.size(); ++i) {
            cycle.append(currentPath[i]);
        }
        cycle.append(node); // Завершаем цикл
        cycles.insert(cycle);
        if(!errors.contains(Error::Cycle)){
            errors.append(Error(Error::Cycle, QString("В графе присутствует цикл %1").arg(node->name)));
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
            } else if (childStatus == PartiallyCovered) {
                allFullyCovered = false;
                allNotCovered = false;
                hasFullyOrPartiallyCovered = true;
            } else if (childStatus == NotCovered) {
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
    } else {
        // 3. Иначе
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
    // 4. Вернуться из рекурсии (автоматически)
}

QString TreeCoverageAnalyzer::getResult() const {
    // Находим целевой узел
    Node* targetNode = nullptr;
    for (Node* node : treeMap) {
        if (node->shape == Node::Target) {
            targetNode = node;
            break;
        }
    }

    // 1. Проверка наличия лишних узлов (узлы, не являющиеся потомками целевого узла)
    if (!extraNodes.isEmpty()) {
        QString extraNodeNames;
        for (Node* node : extraNodes) {
            extraNodeNames += node->name + " ";
        }
        extraNodeNames = extraNodeNames.trimmed();
        return QString("Отмеченный узел %1 не является потомком целевого узла %2.")
            .arg(extraNodeNames, targetNode->name);
    }

    // 2. Проверка наличия избыточных узлов (redundantNodes)
    if (!redundantNodes.isEmpty()) {
        QString ancestorNodeNames;
        QString redundantNodeNames;
        for (const QPair<Node*, Node*>& pair : redundantNodes) {
            // Проверяем, что первый узел в паре является предком второго
            Node* ancestor = pair.first;
            Node* descendant = pair.second;
            ancestorNodeNames += ancestor->name + " ";
            redundantNodeNames += descendant->name + " ";
        }
        redundantNodeNames = redundantNodeNames.trimmed();
        return QString("Предок %1 отмеченного узла %2 тоже отмечен, следует не отмечать детей, если отмечен их предок.")
            .arg(ancestorNodeNames, redundantNodeNames);
    }

    // 3. Проверка наличия узлов, которых не хватает для покрытия (missingNodes)
    if (!missingNodes.isEmpty()) {
        QString missingNodeNames;
        for (Node* node : missingNodes) {
            missingNodeNames += node->name + " ";
        }
        missingNodeNames = missingNodeNames.trimmed();
        return QString("Узел %1 – не покрыт, следует отметить узлы %2 для того чтобы узел %1 стал покрытым.")
            .arg(targetNode->name, missingNodeNames);
    }

    // 4. Если ошибок нет, возвращаем сообщение об успешном покрытии
    QString selectedNodeNames;
    for (Node* node : treeMap) {
        if (node->shape == Node::Selected) {
            selectedNodeNames += node->name + " ";
        }
    }
    selectedNodeNames = selectedNodeNames.trimmed();
    if (selectedNodeNames.isEmpty()) {
        // Если нет отмеченных узлов, но покрытие полное (например, целевой узел без детей)
        return QString("Целевой узел %1 покрыт.").arg(targetNode->name);
    }
    return QString("Помеченные узлы %1 покрывают вышележащий узел %2.")
        .arg(selectedNodeNames, targetNode->name);
}
