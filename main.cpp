/*!
* \file
* \brief Данный файл содержит главную функцию программы TreeCoverageAnalyzerApp.
*
* \mainpage Документация для программы "TreeCoverageAnalyzerApp"
Программа предназначена для анализа покрытия дерева, представленного в формате DOT.
Для функционирования программы необходима операционная система Windows 10 или выше.
Программа разработана на языке C++ с использованием стандартных библиотек C++, библиотеки Qt и фреймворка QtCreator.
Программа должна получать два параметра командной строки: имя входного файла с описанием графа в формате DOT и имя выходного файла для записи результатов анализа покрытия.
Если аргументы командной строки не переданы, программа запускает модульные тесты.

Пример команды запуска программы:
* \code
TreeCoverageAnalyzerApp.exe input.dot output.txt
* \endcode

* \author Лубошников Иван
* \date 25 Июня 2025
* \version 1.0
*/

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "treecoverageanalyzer.h"
#include "tests.h"
#include <clocale>

int runTests() {
    Tests testSuite; // Используем класс Tests из tests.h
    QTest::qExec(&testSuite); // Запускаем все тесты, определённые в Tests
    return 0;
}

/*!
 * \brief Главная функция программы TreeCoverageAnalyzerApp
 * \param [in] argc - количество переданных аргументов командной строки
 * \param [in] argv - переданные аргументы командной строки
 * \param [in] argv[0] - аргумент запуска программы
 * \param [in] argv[1] - путь к входному DOT-файлу
 * \param [in] argv[2] - путь к выходному текстовому файлу с результатами (игнорируется, результат записывается в coverage_result.txt)
 * \return 0 - программа завершилась успешно; 1 - была найдена ошибка
 */
int main(int argc, char* argv[]) {
    if (argc == 1) {
        // Если аргументов нет - запускаем тесты
        return runTests();
    }

    // Основная логика программы
    setlocale(LC_ALL, "Russian_Russia.1251");
    QCoreApplication app(argc, argv);

    // 1. Проверка аргументов командной строки
    if (argc != 3) {
        qCritical() << "Ошибка: Неверное количество аргументов";
        qCritical() << "Использование:" << argv[0] << "<input.dot> <output.txt>";
        qWarning() << "Примечание: второй аргумент игнорируется, результат записывается в coverage_result.txt";
        return 1;
    }

    const QString inputFile = argv[1];

    // 2. Чтение входного DOT-файла
    qDebug() << "Чтение файла:" << inputFile;
    QFile dotFile(inputFile);
    if (!dotFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Ошибка при открытии файла для чтения:" << inputFile;
        return 1;
    }

    QString dotContent = QString::fromUtf8(dotFile.readAll());
    dotFile.close();

    // 3. Создание анализатора покрытия дерева
    TreeCoverageAnalyzer analyzer;

    // 4. Парсинг DOT-контента
    qDebug() << "Парсинг DOT-файла...";
    analyzer.parseDOT(dotContent);

    // 5. Проверка ошибок парсинга
    analyzer.checkErrorsAfterParseDOT();

    // 6. Заполнение хэш-таблицы и проверка графа
    analyzer.fillHash(analyzer.treeMap, analyzer.amountOfParents);
    analyzer.checkErrorsAfterTreeGraphTakeErrors();

    // 7. Анализ покрытия дерева
    qDebug() << "Анализ покрытия дерева...";
    analyzer.analyzeTreeCoverage();

    // 8. Результат уже записан в coverage_result.txt методом getResult
    qDebug() << "Результат сохранен в: coverage_result.txt";

    // 9. Программа завершена успешно
    qDebug() << "Программа завершена успешно.";
    return 0;
}
