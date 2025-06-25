/*!
* \file
* \brief Файл содержит реализацию функций класса Node.
*/

#include "node.h"

Node::Node(const QString& nodeName, Shape nodeShape)
    : name(nodeName), shape(nodeShape) {}

Node::~Node()
{
}
