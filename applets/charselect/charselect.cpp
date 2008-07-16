/***************************************************************************
 *   Copyright 2008 by Laurent Montel <montel@kde.org>                     *
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


#include <QClipboard>
#include <QApplication>
#include <KCharSelect>
#include <KLineEdit>
#include <KPushButton>
#include <QGridLayout>
#include <plasma/widgets/icon.h>
#include "charselect.h"

CharSelectApplet::CharSelectApplet(QObject *parent, const QVariantList &args)
    : PlasmaAppletDialog(parent, args)
{
}


CharSelectApplet::~CharSelectApplet()
{
}

void CharSelectApplet::initMinimumSize()
{
    m_minimumSize = QSize( 400, 350 );
}

QWidget *CharSelectApplet::widget()
{
    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;
    widget->setLayout( layout );

    m_charselect = new KCharSelect(0L, KCharSelect::CharacterTable|KCharSelect::FontCombo);
    connect( m_charselect, SIGNAL( charSelected(const QChar &) ), this, SLOT( slotCharSelect( const QChar& ) ) );
    layout->addWidget( m_charselect, 0, 0, 1, 2);

    m_lineEdit = new KLineEdit;
    m_lineEdit->setReadOnly( true );
    layout->addWidget( m_lineEdit, 1, 0 );

    m_addToClipboard = new KPushButton;
    m_addToClipboard->setText( i18n( "&Add to Clipboard" ) );
    connect( m_addToClipboard, SIGNAL( clicked() ), this, SLOT( slotAddToClipboard() ) );
    layout->addWidget( m_addToClipboard, 1, 1 );
    return widget;
}

void CharSelectApplet::initialize()
{
    m_icon = new Plasma::Icon(KIcon("accessories-character-map"), QString(), this);
}

void CharSelectApplet::slotAddToClipboard()
{
    QString textLine = m_lineEdit->text();
    QClipboard *cb = QApplication::clipboard();
    cb->setText( textLine,QClipboard::Clipboard );
    cb->setText( textLine,QClipboard::Selection );
}

void CharSelectApplet::slotCharSelect( const QChar &c )
{
    m_lineEdit->setText( c );
}

#include "charselect.moc"
