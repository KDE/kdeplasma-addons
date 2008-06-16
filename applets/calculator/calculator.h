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

#include <QGraphicsGridLayout>
#include <QGraphicsProxyWidget>

#include <plasma/applet.h>

class QLabel;
class QAction;

namespace Plasma
{
    class PushButton;
}

class CalculatorApplet : public Plasma::Applet
{
    Q_OBJECT

public:
    CalculatorApplet( QObject *parent, const QVariantList &args );
    ~CalculatorApplet();

    void init();

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
    void constraintsEvent(Plasma::Constraints constraints);

private:
    double sum;
    double factor;
    enum {NumDigitButtons = 10, MaxInputLength=10};
    enum calcOperator { calcNone, calcPlus, calcMinus, calcMult, calcDiv};

    bool calculate(double newValue, calcOperator oldOperator);

    QString inputText;
    calcOperator previousAddSubOperation;
    calcOperator previousMulDivOperation;
    bool waitingForDigit;

    QLabel      *mOutputDisplay;
    Plasma::PushButton *mButtonDigit[10];
    Plasma::PushButton *mButtonDecimal;
    Plasma::PushButton *mButtonAdd;
    Plasma::PushButton *mButtonSubtract;
    Plasma::PushButton *mButtonMultiply;
    Plasma::PushButton *mButtonDivide;
    Plasma::PushButton *mButtonEquals;
    Plasma::PushButton *mButtonClear;
    Plasma::PushButton *mButtonAllClear;
    QGraphicsGridLayout *m_layout;
    QGraphicsProxyWidget *m_proxy;
    QList<QAction *> actions;
};

K_EXPORT_PLASMA_APPLET(calculator, CalculatorApplet)

#endif
