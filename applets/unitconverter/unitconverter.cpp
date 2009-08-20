/*****************************************************************************
 *   Copyright (C) 2008 by Gerhard Gappmeier <gerhard.gappmeier@ascolab.com> *
 *   Copyright (C) 2009 by Petri Damstén <damu@iki.fi>                       *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.                *
 *****************************************************************************/

#include "unitconverter.h"
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <KComboBox>
#include <KLineEdit>
#include <Plasma/ComboBox>
#include <Plasma/LineEdit>
#include <Plasma/Label>
#include <Plasma/Frame>
#include "conversion/converter.h"
#include "conversion/unitcategory.h"

Q_DECLARE_METATYPE(const Conversion::Unit*)
Q_DECLARE_METATYPE(Conversion::UnitCategory*)

UnitConverter::UnitConverter(QObject *parent, const QVariantList &args)
: Plasma::PopupApplet(parent, args)
, m_widget(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPopupIcon("accessories-calculator");
    resize(400, 300);
}

UnitConverter::~UnitConverter()
{
    KConfigGroup cg = config();
    cg.writeEntry("category", m_pCmbCategory->nativeWidget()->currentIndex());
    cg.writeEntry("unit1", m_pCmbUnit1->nativeWidget()->currentIndex());
    cg.writeEntry("unit2", m_pCmbUnit2->nativeWidget()->currentIndex());
    cg.writeEntry("value", m_pTxtValue1->text());
}

void UnitConverter::init()
{
}

void UnitConverter::sltCategoryChanged(int index)
{
    Q_UNUSED(index);
    Conversion::UnitCategory* category =
            m_pCmbCategory->nativeWidget()->itemData(index).value<Conversion::UnitCategory*>();
    QList<Conversion::Unit*> units = category->units();
    Conversion::Unit* defaultUnit = category->defaultUnit();
    m_pCmbUnit1->clear();
    m_pCmbUnit2->clear();
    int i = 0;
    foreach (const Conversion::Unit* unit, units) {
        m_pCmbUnit1->nativeWidget()->addItem(unit->singular(), QVariant::fromValue(unit));
        m_pCmbUnit2->nativeWidget()->addItem(unit->singular(), QVariant::fromValue(unit));
        if (unit == defaultUnit) {
            m_pCmbUnit1->nativeWidget()->setCurrentIndex(i);
            m_pCmbUnit2->nativeWidget()->setCurrentIndex(i);
        }
        ++i;
    }
    if (!category->description().isEmpty()) {
        m_pInfo->setText(QString("<a href=\"%2\">%1</a>")
                .arg(category->description()).arg(category->url().prettyUrl()));
    } else {
        m_pInfo->setText(QString());
    }
    calculate();
}

void UnitConverter::sltUnitChanged(int index)
{
    Q_UNUSED(index);
    calculate();
}

void UnitConverter::sltValueChanged(const QString &sNewValue)
{
    Q_UNUSED(sNewValue);
    calculate();
}

void UnitConverter::calculate()
{
    const Conversion::Unit* in = m_pCmbUnit1->nativeWidget()->itemData(
            m_pCmbUnit1->nativeWidget()->currentIndex()).value<const Conversion::Unit*>();
    const Conversion::Unit* out = m_pCmbUnit2->nativeWidget()->itemData(
            m_pCmbUnit2->nativeWidget()->currentIndex()).value<const Conversion::Unit*>();
    if (in && out) {
        Conversion::Value dblValueIn(m_pTxtValue1->text().toDouble(), in);
        Conversion::Value dblValueOut = in->category()->convert(dblValueIn, out->symbol());

        m_pTxtValue2->setText(QString::number(dblValueOut.number()));
    }
}

QGraphicsWidget *UnitConverter::graphicsWidget()
{
    if (!m_widget) {
        m_widget = new QGraphicsWidget(this);
        Plasma::Frame *pHeader = new Plasma::Frame(this);
        pHeader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        pHeader->setText(i18n("Unit Converter"));

        Plasma::Label *pLabel = new Plasma::Label(this);
        pLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pLabel->setText(i18n("Convert:"));
        m_pCmbCategory = new Plasma::ComboBox(this);
        m_pCmbCategory->setZValue(2);

        m_pCmbUnit1 = new Plasma::ComboBox(this);
        m_pCmbUnit2 = new Plasma::ComboBox(this);
        m_pCmbUnit1->setZValue(1);
        m_pCmbUnit2->setZValue(1);
        m_pTxtValue1 = new Plasma::LineEdit(this);
        m_pTxtValue2 = new Plasma::LineEdit(this);

        m_pInfo = new Plasma::Label(m_widget);
        m_pInfo->nativeWidget()->setOpenExternalLinks(true);

        QGraphicsGridLayout *pGridLayout = new QGraphicsGridLayout(m_widget);
        pGridLayout->addItem(pHeader, 0, 0, 1, 2);
        pGridLayout->addItem(pLabel, 1, 0);
        pGridLayout->addItem(m_pCmbCategory, 1, 1);
        pGridLayout->addItem(m_pCmbUnit1, 2, 0);
        pGridLayout->addItem(m_pCmbUnit2, 2, 1);
        pGridLayout->addItem(m_pTxtValue1, 3, 0);
        pGridLayout->addItem(m_pTxtValue2, 3, 1);
        pGridLayout->addItem(m_pInfo, 4, 0, 1, 2);
        pGridLayout->setRowStretchFactor(5, 1);

        QList<Conversion::UnitCategory*> categories = Conversion::Converter::self()->categories();
        foreach (Conversion::UnitCategory* category, categories) {
            m_pCmbCategory->nativeWidget()->addItem(category->name(), QVariant::fromValue(category));
        }
        // Load previous values
        KConfigGroup cg = config();
        int category = cg.readEntry("category", 0);
        m_pCmbCategory->nativeWidget()->setCurrentIndex(category);
        sltCategoryChanged(category);
        int unit1 = cg.readEntry("unit1", -1);
        if (unit1 >= 0) {
            m_pCmbUnit1->nativeWidget()->setCurrentIndex(unit1);
        }
        int unit2 = cg.readEntry("unit2", -1);
        if (unit2 >= 0) {
            m_pCmbUnit2->nativeWidget()->setCurrentIndex(unit2);
        }
        m_pTxtValue1->setText(cg.readEntry("value", "1.0"));
        calculate();

        connect(m_pTxtValue1->nativeWidget(), SIGNAL(textChanged(const QString&)),
                this, SLOT(sltValueChanged(const QString&)));
        connect(m_pCmbCategory->nativeWidget(), SIGNAL(currentIndexChanged(int)),
                this, SLOT(sltCategoryChanged(int)));
        connect(m_pCmbUnit1->nativeWidget(), SIGNAL(currentIndexChanged(int)),
                this, SLOT(sltUnitChanged(int)));
        connect(m_pCmbUnit2->nativeWidget(), SIGNAL(currentIndexChanged(int)),
                this, SLOT(sltUnitChanged(int)));
    }
    return m_widget;
}

#include "unitconverter.moc"

