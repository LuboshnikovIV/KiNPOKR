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
        //qDebug() << "Error: EmptyFile";
        return; // Вместо throw, чтобы избежать исключений в тестах
    }

    // Гибкое регулярное выражение для узлов с любым количеством атрибутов
    QRegularExpression nodeRegex(R"((\w+(?:,\w+)*)\s*\[(.*?)\]\s*;)");
    QRegularExpressionMatchIterator nodeIter = nodeRegex.globalMatch(content);

    bool hasTargetNode = false;
    QHash<QString, Node*> nodeNameMap;

    while (nodeIter.hasNext()) {
        QRegularExpressionMatch match = nodeIter.next();
        QString nodeList = match.captured(1); // Список узлов (например, "b,c,d")
        QString attributesStr = match.captured(2); // Полная строка атрибутов внутри []

        QStringList nodes = nodeList.split(',');
        Node::Shape nodeShape = Node::Base; // Значение по умолчанию
        bool shapeValid = false;

        // Регулярное выражение для извлечения всех атрибутов (ключ=значение)
        QRegularExpression attrRegex(R"(\s*(\w+)\s*=\s*(\w+|"[^"]*"|'[^']*')\s*(?:,|\s*$))");
        QRegularExpressionMatchIterator attrIter = attrRegex.globalMatch(attributesStr);
        QMap<QString, QString> attrMap; // Храним все атрибуты как ключ-значение

        while (attrIter.hasNext()) {
            QRegularExpressionMatch attrMatch = attrIter.next();
            QString key = attrMatch.captured(1).trimmed();
            QString value = attrMatch.captured(2).trimmed();
            // Удаляем кавычки, если они есть
            if (value.startsWith('"') && value.endsWith('"')) {
                value = value.mid(1, value.length() - 2);
            } else if (value.startsWith('\'') && value.endsWith('\'')) {
                value = value.mid(1, value.length() - 2);
            }
            attrMap[key.toLower()] = value;
            //qDebug() << "Found node attribute:" << key << "=" << value;
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
                    //qDebug() << "Error: InvalidNodeShape for" << name.trimmed();
                }
                continue;
            }
        }

        // Проверяем дополнительные атрибуты (кроме shape) для узлов
        attrMap.remove("shape"); // Убираем shape из проверки ExtraLabel
        if (!attrMap.isEmpty()) {
            for (const QString& name : nodes) {
                QString extraAttrs;
                for (auto it = attrMap.constBegin(); it != attrMap.constEnd(); ++it) {
                    extraAttrs += QString("%1=\"%2\"").arg(it.key(), it.value());
                    if (it != --attrMap.constEnd()) extraAttrs += ", ";
                }
                errors.append(Error(Error::ExtraLabel, QString("%1 %2").arg(name.trimmed(), extraAttrs)));
                //qDebug() << "Error: ExtraLabel for" << name.trimmed() << extraAttrs;
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
            //qDebug() << "Added node:" << trimmedName << "shape:" << nodeShape;
        }
    }

    if (!hasTargetNode) {
        errors.append(Error(Error::NoTargetNode));
        //qDebug() << "Error: NoTargetNode";
        return;
    }

    // Обработка направленных связей с атрибутами
    QRegularExpression edgeRegex(R"((\w+)\s*->\s*(\w+)\s*(?:\[([^\]]+)\])?\s*;)");
    QRegularExpressionMatchIterator edgeIter = edgeRegex.globalMatch(content);

    while (edgeIter.hasNext()) {
        QRegularExpressionMatch match = edgeIter.next();
        QString parentName = match.captured(1);
        QString childName = match.captured(2);
        QString edgeAttrsStr = match.captured(3); // Атрибуты ребра, если есть

        Node* parent = nodeNameMap.value(parentName);
        Node* child = nodeNameMap.value(childName);

        if (!parent || !child) {
            //qDebug() << "Node not found for edge" << parentName << "->" << childName;
            continue;
        }

        parent->children.append(child);
        amountOfParents[child] = amountOfParents.value(child, 0) + 1;
        //qDebug() << "Added directed edge:" << parentName << "->" << childName;

        // Проверяем атрибуты ребра
        if (!edgeAttrsStr.isEmpty()) {
            QRegularExpression attrRegex(R"(\s*(\w+)\s*=\s*(\w+|"[^"]*"|'[^']*')\s*(?:,|\s*$))");
            QRegularExpressionMatchIterator attrIter = attrRegex.globalMatch(edgeAttrsStr);
            while (attrIter.hasNext()) {
                QRegularExpressionMatch attrMatch = attrIter.next();
                QString key = attrMatch.captured(1).trimmed();
                QString value = attrMatch.captured(2).trimmed();
                if (value.startsWith('"') && value.endsWith('"')) {
                    value = value.mid(1, value.length() - 2);
                } else if (value.startsWith('\'') && value.endsWith('\'')) {
                    value = value.mid(1, value.length() - 2);
                }
                //qDebug() << "Found edge attribute:" << key << "=" << value;
                if (key.toLower() == "label") {
                    errors.append(Error(Error::EdgeLabel, QString("%1 и %2").arg(parentName, childName)));
                    //qDebug() << "Error: EdgeLabel for" << parentName << "->" << childName;
                }
            }
        }
    }

    // Обработка недирективных рёбер с атрибутами
    QRegularExpression undirectedEdgeRegex(R"((\w+)\s*--\s*(\w+)\s*(?:\[([^\]]+)\])?\s*;)");
    QRegularExpressionMatchIterator undirectedIter = undirectedEdgeRegex.globalMatch(content);

    bool hasUndirected = false;
    while (undirectedIter.hasNext()) {
        hasUndirected = true;
        QRegularExpressionMatch match = undirectedIter.next();
        QString node1Name = match.captured(1);
        QString node2Name = match.captured(2);
        QString edgeAttrsStr = match.captured(3); // Атрибуты ребра, если есть

        Node* node1 = nodeNameMap.value(node1Name);
        Node* node2 = nodeNameMap.value(node2Name);

        if (!node1 || !node2) {
            //qDebug() << "Node not found for undirected edge" << node1Name << "--" << node2Name;
            continue;
        }

        // Создаём двусторонние связи для недирективного ребра
        node1->children.append(node2);
        node2->children.append(node1);
        amountOfParents[node2] = amountOfParents.value(node2, 0) + 1;
        amountOfParents[node1] = amountOfParents.value(node1, 0) + 1;
        //qDebug() << "Added undirected edge:" << node1Name << "--" << node2Name;

        // Проверяем атрибуты ребра
        if (!edgeAttrsStr.isEmpty()) {
            QRegularExpression attrRegex(R"(\s*(\w+)\s*=\s*(\w+|"[^"]*"|'[^']*')\s*(?:,|\s*$))");
            QRegularExpressionMatchIterator attrIter = attrRegex.globalMatch(edgeAttrsStr);
            while (attrIter.hasNext()) {
                QRegularExpressionMatch attrMatch = attrIter.next();
                QString key = attrMatch.captured(1).trimmed();
                QString value = attrMatch.captured(2).trimmed();
                if (value.startsWith('"') && value.endsWith('"')) {
                    value = value.mid(1, value.length() - 2);
                } else if (value.startsWith('\'') && value.endsWith('\'')) {
                    value = value.mid(1, value.length() - 2);
                }
                //qDebug() << "Found edge attribute:" << key << "=" << value;
                if (key.toLower() == "label") {
                    errors.append(Error(Error::EdgeLabel, QString("%1 и %2").arg(node1Name, node2Name)));
                    //qDebug() << "Error: EdgeLabel for" << node1Name << "--" << node2Name;
                }
            }
        }
    }

    if (hasUndirected) {
        errors.append(Error(Error::UndirectedEdge));
        //qDebug() << "Error: UndirectedEdge";
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
