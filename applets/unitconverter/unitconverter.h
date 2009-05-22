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

#ifndef UNITCONVERTER_H
#define UNITCONVERTER_H

#include <Plasma/PopupApplet>

namespace Plasma {
    class ComboBox;
    class LineEdit;
    class Label;
}

// Define our plasma Applet
class UnitConverter : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        // Basic Create/Destroy
        UnitConverter(QObject *parent, const QVariantList &args);
        ~UnitConverter();

        virtual void init();
        virtual QGraphicsWidget *graphicsWidget();

    private slots:
        void sltValueChanged(const QString &sNewValue);
        void sltUnitChanged(int index);
        void sltCategoryChanged(int index);

    private:
        void calculate();

    private:
        QGraphicsWidget  *m_widget;
        Plasma::ComboBox *m_pCmbCategory;
        Plasma::ComboBox *m_pCmbUnit1;
        Plasma::ComboBox *m_pCmbUnit2;
        Plasma::LineEdit *m_pTxtValue1;
        Plasma::LineEdit *m_pTxtValue2;
        Plasma::Label    *m_pInfo;
        int               m_iCategory;
};

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(unitconverter, UnitConverter)

#endif // _UNITCONVERTER_H_
