/**
 * @file variableeditor.cpp
 * @author Ondřej Turek, xtureko00
 * @brief Variable editor dialog implementation
 */

#include "variableeditor.hpp"
#include "picojson.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QTableWidget>
#include <QVBoxLayout>
#include <cstdint>
#include <qglobal.h>
#include <qstringliteral.h>
#include <string>

constexpr int kColName  = 0;
constexpr int kColType  = 1;
constexpr int kColValue = 2;

// Reuse this macro yippee
#define LOAD_INTO(into, from, key, type) do {if (from.count(key) && from.at(key).is<type>())\
                into = from.at(key).get<type>();} while(false);

// Regex for a valid C++ variable name, hate regexes but probably the easiest to do
const QRegularExpression kIdentifierRx { QStringLiteral("[a-zA-Z_][a-zA-Z0-9_]*") };

QString CppVariable::typeName(Type t)
{
    switch (t) {
    case Type::Bool:              return QStringLiteral("bool");
    case Type::Char:              return QStringLiteral("char");
    case Type::Int:               return QStringLiteral("int");
    case Type::UnsignedInt:       return QStringLiteral("unsigned int");
    case Type::Long:              return QStringLiteral("long");
    case Type::UnsignedLong:      return QStringLiteral("unsigned long");
    case Type::LongLong:          return QStringLiteral("long long");
    case Type::UnsignedLongLong:  return QStringLiteral("unsigned long long");
    case Type::Float:             return QStringLiteral("float");
    case Type::Double:            return QStringLiteral("double");
    case Type::LongDouble:        return QStringLiteral("long double");
    case Type::StdString:         return QStringLiteral("std::string");
    }
    // shut the compiler warning up again
    Q_UNREACHABLE();
}


picojson::object CppVariable::toJson() const
{
    picojson::object obj;
    obj["name"] = picojson::value(name.toStdString());
    obj["type"] = picojson::value(static_cast<int64_t>(this->type));
    obj["value"] = picojson::value(value.toStdString());
    return obj;
}

bool CppVariable::fromJson(const picojson::object &obj)
{
 
    std::string nameStr;
    int64_t type = 999;
    std::string valueStr;

    LOAD_INTO(nameStr, obj, "name", std::string);
    LOAD_INTO(type, obj, "type", int64_t);
    LOAD_INTO(valueStr, obj, "value", std::string);
 
    if (nameStr.empty() || type == 999)
        return false;
 
    CppVariable var;
    var.name  = QString::fromStdString(nameStr);
    var.type  = static_cast<Type>(type);
    var.value = QString::fromStdString(valueStr);
    
    // Check that what we just created is actually a valid variable
    if (var.name.isEmpty() || !var.isValueValid())
        return false;
    
    // Can't believe you can do this
    *this = var;
    return true;
}

QStringList CppVariable::allTypeNames()
{
    QStringList names;
    for (int i = static_cast<int>(Type::Bool); i <= static_cast<int>(Type::StdString); ++i)
        names << typeName(static_cast<Type>(i));
    return names;
}

bool CppVariable::isValueValid() const
{
    // Empty value = string
    if (value.isEmpty()) {
        return type == Type::StdString;
    }

    switch (type) {
    case Type::Bool:
        return value == QLatin1String("true") || value == QLatin1String("false")
            || value == QLatin1String("0")    || value == QLatin1String("1");

    case Type::Char: {
        if(value.length() > 2) {
            return false;
        // Allow escape sequences for chars
        } else if((value.length() == 2) && (value.at(0) == '\\')) {
            return true;
        } else if(value.length() == 1) {
            return true;
        }
        return false;
    }

    case Type::Int:
    case Type::Long:
    case Type::LongLong: {
        bool ok = false;
        value.toLongLong(&ok);
        return ok;
    }

    case Type::UnsignedInt:
    case Type::UnsignedLong:
    case Type::UnsignedLongLong: {
        bool ok = false;
        value.toULongLong(&ok);
        return ok;
    }

    case Type::Float:
    case Type::Double:
    case Type::LongDouble: {
        bool ok = false;
        value.toDouble(&ok);
        return ok;
    }

    case Type::StdString:
        return true; // any string content is valid
    }
    // shut the compiler warning up
    Q_UNREACHABLE();
}

QString CppVariable::toCppDeclaration() const
{
    Q_ASSERT(isValueValid());
    Q_ASSERT(!name.isEmpty());

    const QString tName = typeName(type);

    if (type == Type::StdString) {
        // Wrap the value in double quotes, escape embedded quotes
        QString escaped = value;
        escaped.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
        escaped.replace(QLatin1Char('"'),  QStringLiteral("\\\""));
        return QStringLiteral("%1 %2 = \"%3\";").arg(tName, name, escaped);
    }

    if (type == Type::Char) {
        return QStringLiteral("%1 %2 = '%3';").arg(tName, name, value);
    }

    // We have to add this or we might get a narrowing warning from the compiler
    QString suffix;
    switch (type) {
    case Type::UnsignedInt:
    case Type::UnsignedLong:
    case Type::UnsignedLongLong: suffix = QStringLiteral("u"); break;
    case Type::Long:             suffix = QStringLiteral("l"); break;
    case Type::LongLong:         suffix = QStringLiteral("ll"); break;
    case Type::Float:            suffix = QStringLiteral("f"); break;
    default: break;
    }

    return QStringLiteral("%1 %2 = %3%4;").arg(tName, name, value, suffix);
}


VariableEditorDialog::VariableEditorDialog(VariableMap &variables, QWidget *parent): QDialog(parent), m_variables(variables)
{
    setWindowTitle(tr("Editor Proměnných"));
    setMinimumSize(560, 340);

    // The table unnnghhhh this took so much time to get right
    m_table = new QTableWidget(0, 3, this);
    m_table->setHorizontalHeaderLabels({ tr("Jméno"), tr("Typ"), tr("Hodnota") });
    m_table->horizontalHeader()->setSectionResizeMode(kColName,  QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(kColType,  QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColValue, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    // Buttons
    m_addBtn    = new QPushButton("Přidat", this);
    m_removeBtn = new QPushButton("Odstranit", this);
    m_removeBtn->setEnabled(false);

    auto *editLayout = new QHBoxLayout;
    editLayout->addWidget(m_addBtn);
    editLayout->addWidget(m_removeBtn);
    editLayout->addStretch();

    // Confirmation popup
    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_okBtn     = buttonBox->button(QDialogButtonBox::Ok);
    m_cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);

    // Add it all to a layout
    auto *root = new QVBoxLayout(this);
    root->addWidget(m_table);
    root->addLayout(editLayout);
    root->addWidget(buttonBox);

    // connect everything
    connect(m_addBtn,    &QPushButton::clicked,
            this,        &VariableEditorDialog::addVariable);
    connect(m_removeBtn, &QPushButton::clicked,
            this,        &VariableEditorDialog::removeSelectedVariable);

    connect(m_table, &QTableWidget::cellChanged,
            this,    &VariableEditorDialog::onCellChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_removeBtn->setEnabled(!m_table->selectedItems().isEmpty());
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        // Validate all rows before accepting.
        for (int row = 0; row < m_table->rowCount(); ++row) {
            if (!commitRow(row)) return; // commitRow shows the error message
        }

        // This is a QDialog, so we just call accept() and and the main window knows we're good
        // as with many things, we should have been using this all along
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    loadVariables();
}

void VariableEditorDialog::loadVariables()
{
    m_ignoreChanges = true;

    // Clear everything
    m_table->setRowCount(0);

    for (const CppVariable &var : m_variables) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);


        auto *nameItem = new QTableWidgetItem(var.name);
        m_table->setItem(row, kColName, nameItem);

        auto *typeCombo = new QComboBox(m_table);
        typeCombo->addItems(CppVariable::allTypeNames());
        typeCombo->setCurrentText(CppVariable::typeName(var.type));
        m_table->setCellWidget(row, kColType, typeCombo);

        auto *valueItem = new QTableWidgetItem(var.value);
        m_table->setItem(row, kColValue, valueItem);

    }

    m_ignoreChanges = false;
}


void VariableEditorDialog::addVariable()
{
    m_ignoreChanges = true;

    const int row = m_table->rowCount();
    m_table->insertRow(row);

    auto *nameItem  = new QTableWidgetItem(tr("variable%1").arg(row + 1));
    auto *valueItem = new QTableWidgetItem(QStringLiteral("0"));

    m_table->setItem(row, kColName,  nameItem);
    m_table->setItem(row, kColValue, valueItem);

    auto *typeCombo = new QComboBox(m_table);
    typeCombo->addItems(CppVariable::allTypeNames());
    m_table->setCellWidget(row, kColType, typeCombo);

    m_ignoreChanges = false;

    m_table->selectRow(row);
    m_table->scrollToItem(nameItem);
    m_table->editItem(nameItem);
}

void VariableEditorDialog::removeSelectedVariable()
{
    const int row = m_table->currentRow();
    if (row < 0) return;

    // Also remove from the map if the row was previously committed.
    if (auto *nameItem = m_table->item(row, kColName))
        m_variables.remove(nameItem->text());

    m_table->removeRow(row);
}

void VariableEditorDialog::onCellChanged(int row, int /*column*/)
{
    if (m_ignoreChanges) return;
    commitRow(row);
}

bool VariableEditorDialog::commitRow(int row)
{
    auto *nameItem  = m_table->item(row, kColName);
    auto *valueItem = m_table->item(row, kColValue);
    auto *typeCombo = qobject_cast<QComboBox *>(m_table->cellWidget(row, kColType));

    if (!nameItem || !valueItem || !typeCombo) return false;

    // Check name is valid
    const QString name = nameItem->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Neplatná hodnota",
            QStringLiteral("Hodnota na řádku %1 nesmí být prázdná").arg(row + 1));
        m_table->setCurrentCell(row, kColName);
        return false;
    }
    QRegularExpressionMatch m = kIdentifierRx.match(name);
    if (!m.hasMatch() || m.captured() != name) {
        QMessageBox::warning(this, "Neplatná hodnota",
            QStringLiteral("\"%1\" není platný název proměnné").arg(name));
        m_table->setCurrentCell(row, kColName);
        return false;
    }

    // Check name is not duplicate
    for (int r = 0; r < m_table->rowCount(); ++r) {
        if (r == row) continue;
        if (auto *ni = m_table->item(r, kColName); ni && ni->text() == name) {
            QMessageBox::warning(this, "Neplatná hodnota",
                QStringLiteral("Redefinice proměnné \"%1\"").arg(name));
            m_table->setCurrentCell(row, kColName);
            return false;
        }
    }

    CppVariable var;
    var.name  = name;
    var.type  = static_cast<CppVariable::Type>(typeCombo->currentIndex());
    var.value = valueItem->text();

    if (!var.isValueValid()) {
        QMessageBox::warning(this, tr("Neplatná hodnota"),
            QStringLiteral("\"%1\" není platná hodnota pro typ %2.")
                .arg(var.value).arg(CppVariable::typeName(var.type)));
        m_table->setCurrentCell(row, kColValue);
        return false;
    }

    m_variables.insert(name, var);

    return true;
}

bool showVariableEditor(VariableMap &variables, QWidget *parent)
{
    // Work on a copy so that rejection leaves the original map intact.
    VariableMap workingCopy = variables;

    VariableEditorDialog dlg(workingCopy, parent);
    if (dlg.exec() != QDialog::Accepted)
        return false;

    variables = std::move(workingCopy);
    return true;
}
