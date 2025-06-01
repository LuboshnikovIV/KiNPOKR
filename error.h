#ifndef ERROR_H
#define ERROR_H

#include <QString>
#include "node.h"

class Error
{
public:
    enum ErrorType {
        EmptyFile,
        NoTargetNode,
        NotATree,
        Cycle,
        DisconnectedGraph,
        MultiParents,
        InvalidNodeShape,
        UndirectedEdge,
        ExtraLabel,
        EdgeLabel
    }; // перечисление типов ошибок

    Error(ErrorType errType, const QString& errDetails = QString(), Node* errNode = nullptr);
    QString errMessage() const; // метод для получения текстового сообщения об ошибке

    ErrorType type; // тип ошибки
    QString details; // дополнительные детали ошибки
    Node* errNode; // узел связанный с ошибкой
};

#endif // ERROR_H
