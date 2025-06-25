/*!
* \file
* \brief Файл содержит заголовочный файл класса ошибки Error.
*/

#ifndef ERROR_H
#define ERROR_H

#include <QString>
#include "node.h"

/*!
* \brief Класс для хранения информации об ошибках.
*/
class Error
{
public:
    /*!
    * \brief Перечисление типов ошибок
    */
    enum ErrorType {
        EmptyFile,
        NoTargetNode,
        Cycle,
        DisconnectedGraph,
        MultiParents,
        InvalidNodeShape,
        UndirectedEdge,
        ExtraLabel,
        EdgeLabel
    };

    /*!
    * \brief Конструктор с передаваемыми параметрами для класса Error
    */
    Error(ErrorType errType, const QString& errDetails = QString(), Node* errNode = nullptr);

    ErrorType type; //!< тип ошибки
    QString details; //!< дополнительные детали ошибки
    Node* errNode; //!< узел связанный с ошибкой

    /*!
    * \brief Метод для получения текстового сообщения об ошибке
    */
    QString errMessage() const;

    /*!
    * \brief Перегрузка оператора равенства для Error
    */
    bool operator==(const Error& other) const {
        return type == other.type;
    }
};

#endif // ERROR_H
