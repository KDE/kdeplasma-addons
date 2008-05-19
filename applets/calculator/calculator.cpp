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

#include "calculator.h"
#include <QtGui/QPainter>
#include <QLabel>
#include <QPushButton>

#include <kglobal.h>
#include <klocale.h>
#include <Plasma/Theme>


CalculatorApplet::CalculatorApplet( QObject *parent, const QVariantList &args )
    : Plasma::Applet( parent, args )
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_layout = 0;
    resize(300,300);
    setMinimumSize(300,300);
}

void CalculatorApplet::init()
{
    m_proxy = new QGraphicsProxyWidget(this);
    m_layout = new QGraphicsGridLayout;

    inputText = QString('0');
    sum = 0;
    factor = 0;
    waitingForDigit = true;

    mOutputDisplay = new QLabel;
    mOutputDisplay->setWordWrap( true );
    QGraphicsProxyWidget *w = new QGraphicsProxyWidget;
    w->setWidget( mOutputDisplay );
    m_layout->addItem( w, 0, 0,1,5 );
    mOutputDisplay->setAlignment(Qt::AlignRight);
    QFont font = Plasma::Theme::defaultTheme()->font( Plasma::Theme::DefaultFont );
    font.setBold(true);
    font.setPointSize(16);
    mOutputDisplay->setFont(font);
    mOutputDisplay->setText(inputText);
    mOutputDisplay->setVisible(true);

    mButtonDigit[0] = new QPushButton( QString::number(0) );
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonDigit[0] );
    connect( mButtonDigit[0], SIGNAL( clicked() ), this, SLOT( slotDigitClicked() ) );
    mButtonDigit[0]->setVisible( true );
    m_layout->addItem( w, 5,1,1,2 );


    for (int i = 1; i < NumDigitButtons; i++) {
        int row = ((9 - i) / 3) + 2;
        int column = ((i - 1) % 3) + 1;
        mButtonDigit[i] = new QPushButton( QString::number(i) );
        w = new QGraphicsProxyWidget;
        w->setWidget( mButtonDigit[i] );
        connect( mButtonDigit[i], SIGNAL( clicked() ), this, SLOT( slotDigitClicked() ) );
        mButtonDigit[i]->setVisible( true );
        m_layout->addItem( w, row,column );
    }


    mButtonDecimal = new QPushButton( KGlobal::locale()->decimalSymbol());
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonDecimal );
    m_layout->addItem( w, 5, 3 );
    connect( mButtonDecimal, SIGNAL( clicked() ), this, SLOT( slotDecimalClicked() ) );
    mButtonDecimal->setVisible( true );

    mButtonEquals = new QPushButton( "=" );
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonEquals );
    m_layout->addItem( w, 4, 4,2,1 );

    connect( mButtonEquals, SIGNAL( clicked() ), this, SLOT( slotEqualsClicked() ) );
    mButtonEquals->setVisible( true );

    mButtonAdd = new QPushButton( "+" );
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonAdd );
    m_layout->addItem( w, 3, 4 );
    connect( mButtonAdd, SIGNAL( clicked() ), this, SLOT( slotAddClicked() ) );
    mButtonAdd->setVisible( true );

    mButtonSubtract = new QPushButton( "-" );
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonSubtract );
    m_layout->addItem( w, 2, 4 );

    connect( mButtonSubtract, SIGNAL( clicked() ), this, SLOT( slotSubtractClicked() ) );
    mButtonSubtract->setVisible( true );

    mButtonMultiply = new QPushButton( "X" );
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonMultiply );
    m_layout->addItem( w, 1, 3 );

    connect( mButtonMultiply, SIGNAL( clicked() ), this, SLOT( slotMultiplyClicked() ) );
    mButtonMultiply->setVisible( true );

    mButtonDivide = new QPushButton( "/" );
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonDivide );

    m_layout->addItem( w, 1, 2 );

    connect( mButtonDivide, SIGNAL( clicked() ), this, SLOT( slotDivideClicked() ) );
    mButtonDivide->setVisible( true );

    mButtonClear = new QPushButton( "C" );
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonClear );

    m_layout->addItem( w, 1, 1 );
    connect( mButtonClear, SIGNAL( clicked() ), this, SLOT( slotClearClicked() ) );
    mButtonClear->setVisible( true );

    mButtonAllClear = new QPushButton( "AC" );
    w = new QGraphicsProxyWidget;
    w->setWidget( mButtonAllClear );

    m_layout->addItem( w, 1, 4);
    connect( mButtonAllClear, SIGNAL( clicked() ), this, SLOT( slotAllClearClicked() ) );
    mButtonAllClear->setVisible( true );

    m_proxy->show();
    setLayout(m_layout);
}

CalculatorApplet::~CalculatorApplet()
{
}

void CalculatorApplet::keyPressEvent ( QKeyEvent * event )
{
    switch( event->key() )
    {
    case Qt::Key_Equal:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    {
        mButtonEquals->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Plus:
    {
        mButtonAdd->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Minus:
    {
        mButtonSubtract->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_multiply:
    {
        mButtonMultiply->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Slash:
    {
        mButtonDivide->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_1:
    {
        mButtonDigit[1]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_2:
    {
        mButtonDigit[2]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_3:
    {
        mButtonDigit[3]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_4:
    {
        mButtonDigit[4]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_5:
    {
        mButtonDigit[5]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_6:
    {
        mButtonDigit[6]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_7:
    {
        mButtonDigit[7]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_8:
    {
        mButtonDigit[8]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_9:
    {
        mButtonDigit[9]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_0:
    {
        mButtonDigit[0]->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Comma:
    {
        mButtonDecimal->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Escape:
    {
        mButtonClear->animateClick();
        event->accept();
        break;
    }
    }

    Plasma::Applet::keyPressEvent( event );
}

void CalculatorApplet::slotDigitClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    int newDigit = clickedButton->text().toInt();

    if (inputText == "0" && newDigit == 0.0)
        return;

    if (inputText.length() >= MaxInputLength)
        return;

    if (waitingForDigit) {
        inputText.clear();
        waitingForDigit = false;
    }

    inputText += QString::number(newDigit);

    if (!inputText.contains('.')) {
      //If there is no decimal, then we need to reformat the number
      double currentValue = inputText.toDouble();
      QString localizedString = KGlobal::locale()->formatNumber(currentValue, 0);
      mOutputDisplay->setText(localizedString);
    } else {
      //Once we have a decimal, then all we have to do is append the number
      mOutputDisplay->setText(mOutputDisplay->text()+QString::number(newDigit));
    }
}

/*
 * We should probably take into account localization.  If dots are used for
 */
void CalculatorApplet::slotDecimalClicked()
{

    if (waitingForDigit) {
        inputText = '0';
	mOutputDisplay->setText(inputText);
    }
    //TODO use locale comma
    if (!inputText.contains('.')) {
        inputText += '.';
	mOutputDisplay->setText(mOutputDisplay->text()+'.');
    }

    waitingForDigit = false;
}

void CalculatorApplet::slotAddClicked()
{
    double currentValue = inputText.toDouble();

    //We need to prevent the rounding that is occuring here!

    //kDebug() << "'+' was clixed, parsed to: " << currentValue << endl;

    if (previousAddSubOperation!=calcNone) {
        calculate(currentValue, previousAddSubOperation);
    } else {
        sum = currentValue;
    }

    inputText = '0';  //this is so if you click "1 + =" you'll get 1 instead of 2
    previousAddSubOperation=calcPlus;
    waitingForDigit = true;
}

void CalculatorApplet::slotSubtractClicked()
{
    double currentValue = inputText.toDouble();

    if (previousAddSubOperation!=calcNone) {
        calculate(currentValue, previousAddSubOperation);
    } else {
        sum = currentValue;
    }

    inputText = '0';  //this is so if you click "1 - =" you'll get 1 instead of 0
    previousAddSubOperation = calcMinus;
    waitingForDigit = true;
}

void CalculatorApplet::slotMultiplyClicked()
{
    double currentValue = inputText.toDouble();

    if (previousMulDivOperation!=calcNone) {
        calculate(currentValue, previousMulDivOperation);
    } else {
        factor = currentValue;
    }

    inputText = '0';  //this is so if you click "6 * =" you'll get 6 instead of 36
    previousMulDivOperation=calcMult;
    waitingForDigit = true;
}

void CalculatorApplet::slotDivideClicked()
{
    double currentValue = inputText.toDouble();

    if (previousMulDivOperation!=calcNone) {
        calculate(currentValue, previousMulDivOperation);
    } else {
        factor = currentValue;
    }

    inputText = '0';  //this is if you click "6 / =" you'll get 6 instead of 1
    previousMulDivOperation = calcDiv;
    waitingForDigit = true;
}

void CalculatorApplet::slotEqualsClicked()
{
    bool ok;
    double currentValue = inputText.toDouble(&ok);

    if (ok == false) {
        handleError(i18n("ERROR"));
        return;
    }

    //kDebug() << "'=' was clicked, parsed to: " << currentValue << endl;

    if (previousMulDivOperation!=calcNone) {
        if (!calculate(currentValue, previousMulDivOperation))
            return;
        currentValue = factor;
    }

    if (previousAddSubOperation!=calcNone) {
        calculate(currentValue, previousAddSubOperation);
    } else {
        sum = currentValue;
    }

    //We use the 'g' formatted to figure out whether this if an integer
    inputText = QString::number(sum, 'f', 6);

    QString leftSide  = inputText.left(inputText.indexOf('.'));
    if (leftSide.size()>3)
      leftSide = KGlobal::locale()->formatNumber(leftSide.toDouble(), 0);

    QString rightSide = inputText.right(inputText.size()-inputText.indexOf('.'));
    while (rightSide.size() > 1 && rightSide.endsWith('0')) {
      rightSide = rightSide.left(rightSide.size()-1);
    }
    if (rightSide.endsWith('.'))
      rightSide.clear();
    mOutputDisplay->setText(leftSide+rightSide);

    sum = 0.0;
    factor = 0;
    previousAddSubOperation=calcNone;
    previousMulDivOperation=calcNone;
    waitingForDigit = true;

}

void CalculatorApplet::slotClearClicked()
{
    inputText = '0';
    waitingForDigit = true;
    mOutputDisplay->setText(inputText);
}

void CalculatorApplet::slotAllClearClicked()
{
    sum = 0;
    factor = 0;
    previousAddSubOperation=calcNone;
    previousMulDivOperation=calcNone;
    inputText = '0';
    waitingForDigit = true;
    mOutputDisplay->setText(inputText);
}

bool CalculatorApplet::calculate(double newValue, calcOperator oldOperator)
{
    switch( oldOperator )
    {
    case calcPlus:
        sum += newValue;
        break;
    case calcMinus:
        sum -= newValue;
        break;
    case calcMult:
        factor *= newValue;
        break;
    case calcDiv:
    {
        if (newValue != 0.0)
            factor /= newValue;
        else {
            handleError(i18n("ERROR: DIV BY O"));
            return false;
        }
        break;
    }
    case calcNone:
    default:
        break;
    }
    return true;
}

void CalculatorApplet::handleError(const QString &errorMessage)
{
    sum = 0;
    factor = 0;
    previousAddSubOperation=calcNone;
    previousMulDivOperation=calcNone;
    mOutputDisplay->setText(errorMessage);
    inputText = '0';
    waitingForDigit = true;

}


#include "calculator.moc"
