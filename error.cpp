#include "error.h"

Error::Error([[maybe_unused]] ErrorType errType, [[maybe_unused]] const QString& errDetails, [[maybe_unused]] Node* errNode){}

QString Error::errMessage() const{
    return QString("HI");
}
