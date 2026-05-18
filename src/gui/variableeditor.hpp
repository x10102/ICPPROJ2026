/**
 * @file variableeditor.cpp
 * @author Ondřej Turek, xtureko00
 * @brief Variable editor dialog interface
 */

#ifndef _VAREDITOR_H
#define _VAREDITOR_H
#include "picojson.h"
#include <QDialog>
#include <QMap>
#include <QString>
#include <QStringList>

class QTableWidget;
class QComboBox;
class QLineEdit;
class QPushButton;

struct CppVariable
{
    enum class Type {
        Bool,
        Char,
        Int,
        UnsignedInt,
        Long,
        UnsignedLong,
        LongLong,
        UnsignedLongLong,
        Float,
        Double,
        LongDouble,
        StdString,
    };

    QString name;
    Type    type  { Type::Int };
    QString value;

    /**
     * @brief Checks that the internal value is valid for the type
     */
    bool isValueValid() const;

    /**
     * @brief Returns the keyword used to declare a c++ variable of this type
     */
    static QString typeName(Type t);

    /**
     * @brief Returns the full declaration of the variable 
     */
    QString toCppDeclaration() const;

    /**
     * @brief Returns a list of all available variable names
     */
    static QStringList allTypeNames();

    picojson::object toJson() const;
    bool fromJson(const picojson::object &obj);
};

// ouuughh modern c++ so coool
// also ouughhh Qt has its own map so cool we should have been using this the entire time
using VariableMap = QMap<QString, CppVariable>;

class VariableEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VariableEditorDialog(VariableMap &variables, QWidget *parent = nullptr);

private slots:
    void addVariable();
    void removeSelectedVariable();
    void onCellChanged(int row, int column);

private:
    // Populates the table from the current state of m_variables.
    void loadVariables();

    // Reads one row back into m_variables; returns false on validation error.
    bool commitRow(int row);

    VariableMap   &m_variables;
    QTableWidget  *m_table;
    QPushButton   *m_addBtn;
    QPushButton   *m_removeBtn;
    QPushButton   *m_okBtn;
    QPushButton   *m_cancelBtn;

    bool m_ignoreChanges { false };
};

// Use a free function here, we don't need to be creating / storing instances of the editor
bool showVariableEditor(VariableMap &variables, QWidget *parent = nullptr);
#endif