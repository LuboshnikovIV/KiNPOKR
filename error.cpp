/*!
* \file
* \brief Файл содержит реализацию функций класса Error.
*/

#include "error.h"
#include <QDebug>

Error::Error(ErrorType errType, const QString& errDetails, Node* errNode)
    : type(errType), details(errDetails), errNode(errNode) {}

QString Error::errMessage() const
{
    switch (type) {
    case EmptyFile:
        return "Файл пустой.";
    case NoTargetNode:
        return "Некорректная ситуация, нет узла для которого определяем покрытие.";
    case Cycle:
        return details.isEmpty() ? "Граф не является деревом. В графе присутствует цикл." :
                   QString("Граф не является деревом. В графе присутствует цикл %1").arg(details);
    case DisconnectedGraph:
        return "Граф не является деревом. Граф не связан.";
    case MultiParents:
        return details.isEmpty() ? "Граф не является деревом. Найдены узлы с несколькими родителями." :
                   QString("Граф не является деревом. У узла %1 более одного родителя.").arg(details);
    case InvalidNodeShape:
        return details.isEmpty() ? "Форма узла не соответствует требованиям." :
                   QString("Форма отмеченного узла %1 не соответствует требованиям.").arg(details);
    case UndirectedEdge:
        return "В деревьях связь между узлами должна быть направленная.";
    case ExtraLabel:
        return details.isEmpty() ? "Для узла использована дополнительная метка." :
                   QString("Для узла %1 была использована метка %2, стоит убрать метку и оставить только название.").arg(details);
    case EdgeLabel:
        return details.isEmpty() ? "У связи между узлами есть метка." :
                   QString("У связи между узлами %1 есть метка, которая ухудшает читаемость графа, стоит убрать ее.").arg(details);
    default:
        return "Неизвестная ошибка.";
    }
}

QDebug operator<<(QDebug debug, const Error& error) {
    debug << "Error(" << error.type << ", \"" << ")";
    return debug;
}
