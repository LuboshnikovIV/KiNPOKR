#include "node.h"

Node::Node(const QString& nodeName, Shape nodeShape)
    : name(nodeName), shape(nodeShape), visited(false) {}

Node::~Node()
{
}
