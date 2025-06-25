/*!
* \file
* \brief Файл содержит заголовочный файл класса узла Node.
*/

#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QString>

/*!
* \brief Класс для хранения информации об узле.
*/
class Node
{
public:

    /*!
    * \brief перечисление форм узлов
    */
    enum Shape {
        Target,
        Selected,
        Base
    };

    /*!
    * \brief конструктор с передаваемыми параметрами для класс Node
    */
    Node(const QString& nodeName, Shape nodeShape);

    /*!
    * \brief Деструктор по умолчанию для класса Node
    */
    ~Node();

    QString name; //!< имя узла
    Shape shape; //!< форма узла
    QList<Node*> children; //!< список дочерних узлов

    /*!
    * \brief Перегрузка оператора равенства для Node
    */
    bool operator==(const Node& other) const{
        return name == other.name && shape == other.shape && children == other.children;
    }
};

#endif // NODE_H
