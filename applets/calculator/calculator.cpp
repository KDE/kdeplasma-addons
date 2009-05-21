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

#include <QLabel>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QFontMetrics>
#include <QSizePolicy>

#include <KPushButton>
#include <KGlobal>
#include <KLocale>

#include <Plasma/Theme>
#include <Plasma/PushButton>
#include <Plasma/Label>
#include <Plasma/Frame>

CalculatorApplet::CalculatorApplet( QObject *parent, const QVariantList &args )
      : Plasma::PopupApplet( parent, args ),
      m_widget(0),
      m_layout(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    resize(300,300);
    setMinimumSize(50,50);
    setPopupIcon("accessories-calculator");
}

QGraphicsWidget *CalculatorApplet::graphicsWidget()
{
    if (!m_widget) {
        m_widget = new QGraphicsWidget(this);
        m_layout = new QGraphicsGridLayout(m_widget);

        previousAddSubOperation=calcNone;
        previousMulDivOperation=calcNone;

        int buttonX,buttonY;

        inputText = QString('0');
        sum = 0;
        factor = 0;
        waitingForDigit = true;

        Plasma::Frame *displayFrame = new Plasma::Frame(this);
        displayFrame->setFrameShadow(Plasma::Frame::Sunken);
        QGraphicsLinearLayout *frameLayout = new QGraphicsLinearLayout(displayFrame);
        mOutputDisplay = new Plasma::Label;
        mOutputDisplay->nativeWidget()->setWordWrap( true );
        frameLayout->addItem(mOutputDisplay);
        m_layout->addItem( displayFrame, 0, 0, 1, 4 );
        mOutputDisplay->nativeWidget()->setAlignment(Qt::AlignRight);
        QFont font = Plasma::Theme::defaultTheme()->font( Plasma::Theme::DefaultFont );
        font.setBold(true);
        font.setPointSize(16);
        QFontMetrics metric(font);
        mOutputDisplay->setMinimumSize(50,metric.height());
        mOutputDisplay->nativeWidget()->setFont(font);
        mOutputDisplay->nativeWidget()->setText(inputText);

        mButtonDigit[0] = new Plasma::PushButton(m_widget);
        QFontMetrics metric1(mButtonDigit[0]->font());
        buttonY=metric1.height()*1.3;
        buttonX=metric1.width("00");
        mButtonDigit[0]->setText( QString::number(0) );
        connect( mButtonDigit[0], SIGNAL( clicked() ), this, SLOT( slotDigitClicked() ) );
        mButtonDigit[0]->setMinimumSize(buttonX,buttonY);
        mButtonDigit[0]->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));
        mButtonDigit[0]->setVisible( true );
        m_layout->addItem( mButtonDigit[0], 5,0,1,2 );

        for (int i = 1; i < NumDigitButtons; i++) {
            int row = ((9 - i) / 3) + 2;
            int column = ((i - 1) % 3);
            mButtonDigit[i] = new Plasma::PushButton(m_widget);
            mButtonDigit[i]->setText( QString::number(i) );
            connect( mButtonDigit[i], SIGNAL( clicked() ), this, SLOT( slotDigitClicked() ) );
            mButtonDigit[i]->setVisible( true );
            mButtonDigit[i]->setMinimumSize(buttonX,buttonY);
            mButtonDigit[i]->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));
            m_layout->addItem( mButtonDigit[i], row, column );
        }

        mButtonDecimal = new Plasma::PushButton(m_widget);
        mButtonDecimal->setText( KGlobal::locale()->decimalSymbol() );
        m_layout->addItem( mButtonDecimal, 5, 2 );
        connect( mButtonDecimal, SIGNAL( clicked() ), this, SLOT( slotDecimalClicked() ) );
        mButtonDecimal->setVisible( true );
        mButtonDecimal->setMinimumSize(buttonX,buttonY);
        mButtonDecimal->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));

        mButtonEquals = new Plasma::PushButton(m_widget);
        mButtonEquals->setText( i18nc("The = button of the calculator", "=") );
        mButtonEquals->setMinimumSize(buttonX,buttonY);
        mButtonEquals->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));
        m_layout->addItem( mButtonEquals, 4, 3, 2,1 );

        connect( mButtonEquals, SIGNAL( clicked() ), this, SLOT( slotEqualsClicked() ) );
        mButtonEquals->setVisible( true );

        mButtonAdd = new Plasma::PushButton(m_widget);
        mButtonAdd->setText( i18nc("The + button of the calculator", "+") );
        mButtonAdd->setMinimumSize(buttonX,buttonY);
        mButtonAdd->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));

        m_layout->addItem( mButtonAdd, 3, 3 );
        connect( mButtonAdd, SIGNAL( clicked() ), this, SLOT( slotAddClicked() ) );
        mButtonAdd->setVisible( true );

        mButtonSubtract = new Plasma::PushButton(m_widget);
        mButtonSubtract->setText( i18nc("The − button of the calculator", "−") );
        mButtonSubtract->setMinimumSize(buttonX,buttonY);
        mButtonSubtract->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));
        m_layout->addItem( mButtonSubtract, 2, 3 );

        connect( mButtonSubtract, SIGNAL( clicked() ), this, SLOT( slotSubtractClicked() ) );
        mButtonSubtract->setVisible( true );

        mButtonMultiply = new Plasma::PushButton(m_widget);
        mButtonMultiply->setText( i18nc("The × button of the calculator", "×") );
        mButtonMultiply->setMinimumSize(buttonX,buttonY);
        mButtonMultiply->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));
        m_layout->addItem( mButtonMultiply, 1, 2 );

        connect( mButtonMultiply, SIGNAL( clicked() ), this, SLOT( slotMultiplyClicked() ) );
        mButtonMultiply->setVisible( true );

        mButtonDivide = new Plasma::PushButton(m_widget);
        mButtonDivide->setText( i18nc("The ∕ button of the calculator", "∕") );
        mButtonDivide->setMinimumSize(buttonX,buttonY);
        mButtonDivide->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));

        m_layout->addItem( mButtonDivide, 1, 1 );

        connect( mButtonDivide, SIGNAL( clicked() ), this, SLOT( slotDivideClicked() ) );
        mButtonDivide->setVisible( true );

        mButtonClear = new Plasma::PushButton(m_widget);
        mButtonClear->setText( i18nc("The C button of the calculator", "C") );
        mButtonClear->setMinimumSize(buttonX,buttonY);
        mButtonClear->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));

        m_layout->addItem( mButtonClear, 1, 0 );
        connect( mButtonClear, SIGNAL( clicked() ), this, SLOT( slotClearClicked() ) );
        mButtonClear->setVisible( true );

        mButtonAllClear = new Plasma::PushButton(m_widget);
        mButtonAllClear->setText( i18nc("The AC button of the calculator", "AC") );
        mButtonAllClear->setMinimumSize(buttonX,buttonY);
        mButtonAllClear->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));

        m_layout->addItem( mButtonAllClear, 1, 3);
        connect( mButtonAllClear, SIGNAL( clicked() ), this, SLOT( slotAllClearClicked() ) );
        mButtonAllClear->setVisible( true );

        // the following three lines are a complete pain and only partially work.
        // this should be moved into convenience methods in Applet, or TT needs to improve
        // how this works in QGraphicsWidget.
        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        m_widget->setMinimumSize(m_layout->sizeHint(Qt::MinimumSize) + QSizeF(left + right, top + bottom));
        m_widget->resize(300,300);

        QAction *copy = new QAction(i18n( "Copy" ), this);
        actions.append(copy);
        connect(copy, SIGNAL(triggered(bool)), this, SLOT(slotCopy()));

        QAction *paste = new QAction(i18n( "Paste" ), this);
        actions.append(paste);
        connect(paste, SIGNAL(triggered(bool)), this, SLOT(slotPaste()));
    }

    return m_widget;
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
        mButtonEquals->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Plus:
    {
        mButtonAdd->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Minus:
    {
        mButtonSubtract->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Asterisk:
    case Qt::Key_multiply:
    {
        mButtonMultiply->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Slash:
    {
        mButtonDivide->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_1:
    {
        mButtonDigit[1]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_2:
    {
        mButtonDigit[2]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_3:
    {
        mButtonDigit[3]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_4:
    {
        mButtonDigit[4]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_5:
    {
        mButtonDigit[5]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_6:
    {
        mButtonDigit[6]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_7:
    {
        mButtonDigit[7]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_8:
    {
        mButtonDigit[8]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_9:
    {
        mButtonDigit[9]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_0:
    {
        mButtonDigit[0]->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Comma:
    case Qt::Key_Period:
    {
        mButtonDecimal->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    case Qt::Key_Escape:
    {
        mButtonClear->nativeWidget()->animateClick();
        event->accept();
        break;
    }
    }

    Plasma::Applet::keyPressEvent( event );
}

void CalculatorApplet::slotDigitClicked()
{
    Plasma::PushButton *clickedButton = qobject_cast<Plasma::PushButton *>(sender());
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

    if (!inputText.contains(KGlobal::locale()->decimalSymbol())) {
      //If there is no decimal, then we need to reformat the number
      double currentValue = KGlobal::locale()->readNumber(inputText);
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

    if (!inputText.contains(KGlobal::locale()->decimalSymbol())) {
        inputText += KGlobal::locale()->decimalSymbol();
        mOutputDisplay->setText(mOutputDisplay->text()+KGlobal::locale()->decimalSymbol());
    }

    waitingForDigit = false;
}

void CalculatorApplet::slotAddClicked()
{
    double currentValue = KGlobal::locale()->readNumber(inputText);

    //We need to prevent the rounding that is occurring here!

    //kDebug() << "'+' was clixed, parsed to: " << currentValue << endl;

    if (previousMulDivOperation != calcNone) {
        calculate(currentValue, previousMulDivOperation);
        previousMulDivOperation = calcNone;
        calculate(factor, previousAddSubOperation);
        factor = 0;
    } else if (previousAddSubOperation != calcNone) {
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
    double currentValue = KGlobal::locale()->readNumber(inputText);

    if (previousMulDivOperation != calcNone) {
        calculate(currentValue, previousMulDivOperation);
        previousMulDivOperation = calcNone;
        calculate(factor, previousAddSubOperation);
        factor = 0;
    } else if (previousAddSubOperation != calcNone) {
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
    double currentValue = KGlobal::locale()->readNumber(inputText);

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
    double currentValue = KGlobal::locale()->readNumber(inputText);

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
    double currentValue = KGlobal::locale()->readNumber(inputText, &ok);

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
    inputText = KGlobal::locale()->formatNumber(sum, 6);

    int decimalIndex = inputText.indexOf(KGlobal::locale()->decimalSymbol());

    if (decimalIndex != -1) {
        while (inputText.size() > decimalIndex && inputText.endsWith('0')) {
            inputText = inputText.left(inputText.size() - 1);
        }

        if (inputText.endsWith(KGlobal::locale()->decimalSymbol())) {
            inputText = inputText.left(inputText.size() - 1);
        }
    }

    mOutputDisplay->setText(inputText);

    sum = 0.0;
    factor = 0;
    previousAddSubOperation = calcNone;
    previousMulDivOperation = calcNone;
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
            handleError(i18n("ERROR: DIV BY 0"));
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

QList<QAction*> CalculatorApplet::contextualActions()
{
    return actions;
}

void CalculatorApplet::slotCopy()
{
    QString txt = mOutputDisplay->text();
    (QApplication::clipboard())->setText(txt, QClipboard::Clipboard);
    (QApplication::clipboard())->setText(txt, QClipboard::Selection);
}

void CalculatorApplet::slotPaste()
{
    QString tmp_str = (QApplication::clipboard())->text(QClipboard::Clipboard );
    if ( tmp_str.isEmpty() )
        tmp_str = (QApplication::clipboard())->text(QClipboard::Selection);
    bool ok;
    long value = tmp_str.toLong( &ok );
    if ( ok )
        mOutputDisplay->setText( QString::number( value ) );

}

#include "calculator.moc"
