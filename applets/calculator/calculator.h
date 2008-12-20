/***************************************************************************
 *   Copyright (C) 2007 by Henry Stanaland <stanaland@gmail.com>           *
 *   Copyright (C) 2008 by Laurent Montel  <montel@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <plasma/popupapplet.h>

class QAction;
class QGraphicsGridLayout;

namespace Plasma
{
    class PushButton;
    class Label;
}

class CalculatorApplet : public Plasma::PopupApplet
{
    Q_OBJECT

public:
    CalculatorApplet( QObject *parent, const QVariantList &args );
    ~CalculatorApplet();

    virtual QList<QAction*> contextualActions();

private Q_SLOTS:
    void handleError(const QString &errorMessage);
    void slotDigitClicked();
    void slotDecimalClicked();
    void slotAddClicked();
    void slotSubtractClicked();
    void slotMultiplyClicked();
    void slotDivideClicked();
    void slotEqualsClicked();
    void slotClearClicked();
    void slotAllClearClicked();
    void slotCopy();
    void slotPaste();

protected:
    virtual void keyPressEvent ( QKeyEvent * event );

private:
    double sum;
    double factor;
    enum {NumDigitButtons = 10, MaxInputLength=10};
    enum calcOperator { calcNone, calcPlus, calcMinus, calcMult, calcDiv};

    virtual QGraphicsWidget *graphicsWidget();
    bool calculate(double newValue, calcOperator oldOperator);

    QString inputText;
    calcOperator previousAddSubOperation;
    calcOperator previousMulDivOperation;
    bool waitingForDigit;

    Plasma::Label      *mOutputDisplay;
    Plasma::PushButton *mButtonDigit[10];
    Plasma::PushButton *mButtonDecimal;
    Plasma::PushButton *mButtonAdd;
    Plasma::PushButton *mButtonSubtract;
    Plasma::PushButton *mButtonMultiply;
    Plasma::PushButton *mButtonDivide;
    Plasma::PushButton *mButtonEquals;
    Plasma::PushButton *mButtonClear;
    Plasma::PushButton *mButtonAllClear;
    QGraphicsWidget *m_widget;
    QGraphicsGridLayout *m_layout;
    QList<QAction *> actions;
};

K_EXPORT_PLASMA_APPLET(calculator, CalculatorApplet)

#endif
