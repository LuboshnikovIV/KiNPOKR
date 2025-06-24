#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QString>


class Node
{
public:
    enum Shape {
        Target,
        Selected,
        Base
    }; // перечисление форм узлов

    Node(const QString& nodeName, Shape nodeShape);
    ~Node();

    QString name; // имя узла
    Shape shape; // форма узла
    QList<Node*> children; // список дочерних узлов
    bool visited = false; // переменная для обхода

    bool operator==(const Node& other) const{
        return name == other.name && shape == other.shape && children == other.children;
    }
};

#endif // NODE_H
