QT += core testlib  # testlib для QTest, core для QObject
CONFIG += c++17 qttest  # qttest для корректной работы Qt Test

TARGET = TestApp
SOURCES += \
    error.cpp \
    main.cpp \
    node.cpp \
    tests.cpp \
    treecoverageanalyzer.cpp

HEADERS += \
    error.h \
    node.h \
    tests.h \
    treecoverageanalyzer.h
