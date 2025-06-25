/*!
* \file
* \brief Файл содержит заголовочный файл класса TreeCoverageAnalyzer, которй используются в ходе работы программы GetConclusionAboutNodeCoverage.
*/
#ifndef TREECOVERAGEANALYZER_H
#define TREECOVERAGEANALYZER_H

#include <QRegularExpression>
#include <QHash>
#include <QSet>
#include <QList>
#include <QPair>
#include "Node.h"
#include "Error.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

/*!
* \brief Класс для хранения информации о выводе программы, а также загаловки функций выполняющих основную работу всей программы.
*/
class TreeCoverageAnalyzer
{
public:
    /*!
    * \brief перечисление типов покрытия узлов
    */
    enum CoverageStatus {
        FullyCovered,
        PartiallyCovered,
        NotCovered
    };

    /*!
    * \brief конструктор по умолчанию для класса TreeCoverageAnalyzer
    */
    TreeCoverageAnalyzer();

    /*!
    * \brief деконструктор по умолчанию для класса TreeCoverageAnalyzer
    */
    ~TreeCoverageAnalyzer();

    QSet<Node*> rootNodes; //!< список корней графа
    QSet<QList<Node*>> cycles; //!< список циклов
    QSet<Node*> multiParents; //!< список узлов у которых есть множество родителей
    QHash<Node*, int> amountOfParents; //!< таблица узел - количество родителей узла
    bool isConnected; //!< поле указывает связан ли граф
    QList<Node*> treeMap; //!< контейнер всех найденных узлов
    QSet<Node*> visitedNodes; //!< список посещенных узлов
    QSet<Node*> missingNodes; //!< список узлов которых не хватает для покрытия
    QSet<Node*> extraNodes; //!< список лишних узлов
    QSet<QPair<Node*, Node*>> redundantNodes; //!< список избыточных узлов, представляет собой пару, где первый элемент это узел который был отмечен, а второй избыточный
    QList<Error> errors; //!< список найденных ошибок

    /*!
    * \brief Функция позволяющая записать найденные ошибки в отдельный файл и завершить выполнение программы
    * \param[in,out] filename - имя файла
    */
    void writeErrorsToFileAndExit(const QString& filename);

    /*!
    * \brief Функция проверяет найденные ошибки после парсинга файла
    * \param[out] filename - имя файла с которым будет создоваться вывод об ошибках
    */
    void checkErrorsAfterParseDOT();

    /*!
    * \brief Преобразует файл написанный на языке DOT в данные о графе-дереве
    * \param [in] content – содержимое входного файла в формате DOT
    * \param [out] treeMap – список узлов встреченных в файле
    * \param [out] Error::ErrorType – enum тип ошибки для дальнейшего сбора ошибок
    * \param [out] Error::errNode – узел связанный с ошибкой (если ошибка связана с узлом)
    */
    void parseDOT(const QString& content);

    /*!
    * \brief Очисщает поля класса TreeCoverageAnalyzer
    */
    void clearData();

    /*!
    * \brief Заполнение таблицы узел – количество родителей и вызов валидации графа
    * \param [in] treeMap – список всех узлов которые нашлись при считывании .dot файла
    * \param[in,out] amountOfParents – таблица узел - количество родителей узла
    */
    void fillHash(QList<Node*>& treeMap, QHash<Node*, int>& amountOfParents);

    /*!
    * \brief Проверяет граф на то, что он является деревом, этой проверкой функция собирает все ошибки которые могут быть, а именно: множество родителей у узла (>=2); цикличность внутри графа; связность графа
    * \param [in] amountOfParents – заполненная таблица узел-количество родителей
    * \param[out] isConnected – переменная показывает связан ли граф
    * \param[out] multiParents – список узлов с количеством родителей >= 2
    * \param[out] rootNodes – список корней найденных в графе
    * \param [out] Error::ErrorType – enum тип ошибки для дальнейшего сбора ошибок
    */
    void treeGraphTakeErrors(QHash<Node*, int>& amountOfParents);

    /*!
    * \brief Проверяет наличие циклов в графе
    * \param [in] node - текущий узел дерева (корень)
    * \param [out] cycles - контейнер для хранения найденных циклов
    * \param [out] currentPath - текущий путь при обходе графа (для отслеживания циклов)
    * \param [out] visitedNodes – список посещенных узлов у данного
    * \param [out] Error::ErrorType – enum тип ошибки для дальнейшего сбора ошибок
    */
    void hasCycles(Node* node, QList<Node*>& currentPath);

    /*!
    * \brief Проверяет наличие циклов в графе
    */
    void analyzeTreeCoverage();

    /*!
    * \brief Анализирует покрытие зоны в которой возможно находятся лишние узлы
    * \param [in] node - текущий узел для анализа (изначально корень дерева)
    * \param [out] extraNodes – контейнер с лишними узлами
    */
    void analyzeZoneWithExtraNodes(Node* node);

    /*!
    * \brief Анализирует покрытие зоны в которой возможно находятся узлы которые нужно отметить
    * \param [in] node - текущий узел для анализа
    * \param [out] missingNodes – контейнер с узлами которых не хватает для покрытия
    * \return одно из enum значений (FullyCovered, PartiallyCovered, NotCovered), где FullyCovered – узел имеет полное покрытие,  PartiallyCovered - узел имее частичное покрытие, NotCovered – узел полностью не покрыт
    */
    CoverageStatus analyzeZoneWithMissingNodes(Node* node);

    /*!
    * \brief Анализирует покрытие зоны в которой возможно находятся избыточные узлы
    * \param [in] node - текущий узел для анализа
    * \param [in] selectedNode – узел который был отмечен до перехода в эту зону
    * \param [in/out] redundant – контейнер с избыточными узлами
    */
    void analyzeZoneWithRedundantNodes(Node* node, Node* selectedNode);

    /*!
    * \brief Функция получения результата анализа
    * \param [out] file – файл формата .txt в котором будет составлен вывод о покрытии узла
    */
    void getResult() const;
};

#endif // TREECOVERAGEANALYZER_H
