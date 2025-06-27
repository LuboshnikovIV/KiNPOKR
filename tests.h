/*!
* \file
* \brief Заголовочный файл класса Tests для тестирования функций программы GetConclusionAboutNodeCoverage.
*/

#ifndef TESTS_H
#define TESTS_H

#include <QObject>
#include <QtTest/QtTest>
#include "node.h"
#include "error.h"
#include "treecoverageanalyzer.h"

/*!
 * \brief Класс для тестирования функций
 */
class Tests : public QObject
{
    Q_OBJECT

private:

    /*!
    * \brief Функция позволяющая создать узел внутри теста
    * \param[in] name - имя узла
    * \param[in] shape - форма узла
    * \return Объект класса Node
    */
    Node* createNode(const QString& name, Node::Shape shape = Node::Shape::Base) {
        return new Node(name, shape);
    }

    /*!
    * \brief Функция позволяющая имитировать связь между узлами внутри теста
    * \param[in] parent - узел-родитель
    * \param[in] child - узел-ребенок
    * \param[in,out] amountOfParents - таблица узел - количество родителей узла
    */
    void addEdge(Node* parent, Node* child, QHash<Node*, int>& amountOfParents) {
        parent->children.append(child);
        amountOfParents[child] = amountOfParents.value(child, 0) + 1;
    }

    /*!
    * \brief Функция сравнивает внутренности контейнера (QSet<Node*>) и выводит их на экран в удобном для чтения виде
    * \param[in] actual - контейнер который был получен в результате выполнения программы
    * \param[in] expected - контейнер который ожидаем поучить
    */
    void printNodeSetDifference(const QSet<Node*>& actual, const QSet<Node*>& expected);

    /*!
    * \brief Функция сравнивает внутренности контейнера (QSet<QList<Node*>>) и выводит их на экран в удобном для чтения виде
    * \param[in] actual - контейнер который был получен в результате выполнения программы
    * \param[in] expected - контейнер который ожидаем поучить
    */
    void printNodeSetDifferenceForCycles(const QSet<QList<Node*>>& actual, const QSet<QList<Node*>>& expected);

    /*!
    * \brief Функция сравнивает внутренности контейнера (QSet<QPair<Node*, Node*>>) и выводит их на экран в удобном для чтения виде
    * \param[in] actual - контейнер который был получен в результате выполнения программы
    * \param[in] expected - контейнер который ожидаем поучить
    */
    void printNodeSetDifferenceForRedundant(const QSet<QPair<Node*, Node*>>& actual, const QSet<QPair<Node*, Node*>>& expected);

    void verifyNodes(const QSet<QString>& actualNames, const QSet<QString>& expectedNames, const QList<Node*>& actualTreeMap, const QList<Node*>& expectedTreeMap);

    /*!
    * \brief Функция сравнивает два узла по форме, количеству детей и структуре детей, сохраняя порядок и учитывая цикл
    * \param[in] pair - первый элемент пары: полученный узел от parseDOT, второй элемент: ожидаемый узел
    * \param[in,out] visited - контейнер уже посещенных пар-узлов
    * \return true - если форма, количество и структура детей совпадают, false - в противном случае
    */
    bool compareNodes(QPair<const Node*, const Node*> pair, QSet<QPair<const Node*, const Node*>>& visited);

private slots:
    void parseDOT_test();
    void parseDOT_test_data();

    void treeGraphTakeErrors_test();
    void treeGraphTakeErrors_test_data();

    void hasCycles_test();
    void hasCycles_test_data();

    void analyzeZoneWithExtraNodes_test();
    void analyzeZoneWithExtraNodes_test_data();

    void analyzeZoneWithMissingNodes_test();
    void analyzeZoneWithMissingNodes_test_data();

    void analyzeZoneWithRedundant_test();
    void analyzeZoneWithRedundant_test_data();
};

#endif // TESTS_H
