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
class QPushButton;
class QLabel;

class CalculatorApplet : public Plasma::Applet
{
    Q_OBJECT

public:
    CalculatorApplet( QObject *parent, const QVariantList &args );
    ~CalculatorApplet();

    void init();

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

protected:
    virtual void keyPressEvent ( QKeyEvent * event );
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
    QPushButton *mButtonDigit[10];
    QPushButton *mButtonDecimal;
    QPushButton *mButtonAdd;
    QPushButton *mButtonSubtract;
    QPushButton *mButtonMultiply;
    QPushButton *mButtonDivide;
    QPushButton *mButtonEquals;
    QPushButton *mButtonClear;
    QPushButton *mButtonAllClear;
    QGraphicsGridLayout *m_layout;
    QGraphicsProxyWidget *m_proxy;

};

K_EXPORT_PLASMA_APPLET(calculator, CalculatorApplet)

#endif
