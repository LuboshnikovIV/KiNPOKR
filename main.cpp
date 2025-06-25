#include "tests.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    Tests test;
    QTest::qExec(&test);
}
