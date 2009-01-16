/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "addmacro.h"
#include <fixx11h.h>
#include <QVBoxLayout>
#include <KComboBox>
#include <QLabel>
#include <QCheckBox>
#include <KLineEdit>
#include <QSpinBox>
#include <KUrlRequester>
#include <KLocale>

AddMacro::AddMacro(QWidget* parent)
: KDialog(parent), m_params(0)
{
    setCaption(i18n("Add Macro"));
    setButtons(KDialog::Ok | KDialog::Cancel);

    m_widget = new QWidget(this);
    setMainWidget(m_widget);

    m_layout = new QVBoxLayout(m_widget);
    m_layout->setMargin(0);
    m_layout->setSpacing(spacingHint());

    m_macrosComboBox = new KComboBox(m_widget);
    const QMap<QString, QVariantList>& macros = PasteMacroExpander::instance().macros();
    foreach (const QString& macro, macros.keys()) {
        m_macrosComboBox->addItem(macros[macro][0].toString(), macro);
    }
    connect(m_macrosComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
    m_layout->addWidget(m_macrosComboBox);

    currentIndexChanged(0);
}

AddMacro::~AddMacro()
{
}

void AddMacro::currentIndexChanged(int index)
{
    delete m_params;
    m_params = new QWidget(m_widget);
    m_layout->addWidget(m_params);
    QVBoxLayout* layout = new QVBoxLayout(m_params);
    const QMap<QString, QVariantList>& macros = PasteMacroExpander::instance().macros();
    const QVariantList& params = macros[m_macrosComboBox->itemData(index).toString()];
    QWidget* w = 0;

    for (int i = 1; i < params.count(); ++i) {
        MacroParam param = params[i].value<MacroParam>();
        switch (param.type) {
            case MacroParam::Int:
            {
                QHBoxLayout* l = new QHBoxLayout(m_params);
                l->addWidget(new QLabel(param.name + ':', m_params));
                l->addWidget(w = new QSpinBox(m_params));
                layout->addItem(l);
                break;
            }
            case MacroParam::String:
                layout->addWidget(new QLabel(param.name + ':', m_params));
                layout->addWidget(w = new KLineEdit(m_params));
                break;
            case MacroParam::Url:
                layout->addWidget(new QLabel(param.name + ':'));
                layout->addWidget(w = new KUrlRequester(m_params));
                break;
            case MacroParam::Boolean:
                layout->addWidget(w = new QCheckBox(param.name, m_params));
                break;
        }
        w->setObjectName(param.name);
    }
    layout->addStretch(1.0);
}

QString AddMacro::macro()
{
    int index = m_macrosComboBox->currentIndex();
    const QMap<QString, QVariantList>& macros = PasteMacroExpander::instance().macros();
    QString macro = m_macrosComboBox->itemData(index).toString();
    const QVariantList& params = macros[m_macrosComboBox->itemData(index).toString()];
    QStringList values;

    for (int i = 1; i < params.count(); ++i) {
        MacroParam param = params[i].value<MacroParam>();
        switch (param.type) {
            case MacroParam::Int:
            {
                QSpinBox* w = m_params->findChildren<QSpinBox*>(param.name)[0];
                values.append(QString::number(w->value()));
                break;
            }
            case MacroParam::String:
            {
                KLineEdit* w = m_params->findChildren<KLineEdit*>(param.name)[0];
                values.append(w->text());
                break;
            }
            case MacroParam::Url:
            {
                KUrlRequester* w = m_params->findChildren<KUrlRequester*>(param.name)[0];
                values.append(w->url().prettyUrl());
                break;
            }
            case MacroParam::Boolean:
            {
                QCheckBox* w = m_params->findChildren<QCheckBox*>(param.name)[0];
                values.append((w->checkState() == Qt::Unchecked) ? "false" : "true");
                break;
            }
        }
    }
    return QString("%{%1(%2)}").arg(macro).arg(values.join(", "));
}

#include "addmacro.moc"
