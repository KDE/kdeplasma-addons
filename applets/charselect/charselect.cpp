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

#include "charselect.h"

#include <QClipboard>
#include <QApplication>
#include <KCharSelect>
#include <KLineEdit>
#include <KPushButton>
#include <QGridLayout>
#include <plasma/widgets/iconwidget.h>

CharSelectApplet::CharSelectApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_mainWidget(0)
{
    setPopupIcon(QLatin1String("accessories-character-map"));
}


CharSelectApplet::~CharSelectApplet()
{
    delete m_mainWidget;
}

void CharSelectApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::StartupCompletedConstraint) {
        if (size().width() < widget()->size().width() || 
             size().height() < widget()->size().height()) {
            resize(widget()->size());
            emit appletTransformedItself();
        }
    }
}

QWidget *CharSelectApplet::widget()
{
    if (!m_mainWidget) {
        m_mainWidget = new QWidget;
        m_mainWidget->setAttribute(Qt::WA_NoSystemBackground);
        QGridLayout *layout = new QGridLayout(m_mainWidget);

        m_charselect = new KCharSelect(m_mainWidget, 0, KCharSelect::BlockCombos|KCharSelect::CharacterTable|KCharSelect::FontCombo);
        m_charselect->setMinimumSize(300, 250);
        connect( m_charselect, SIGNAL(charSelected(QChar)), this, SLOT(slotCharSelect(QChar)) );
        layout->addWidget( m_charselect, 0, 0, 1, 2);

        m_lineEdit = new KLineEdit(m_mainWidget);
        m_lineEdit->setReadOnly( true );
        layout->addWidget( m_lineEdit, 1, 0 );

        m_addToClipboard = new KPushButton(m_mainWidget);
        m_addToClipboard->setText( i18n( "&Add to Clipboard" ) );
        connect( m_addToClipboard, SIGNAL(clicked()), this, SLOT(slotAddToClipboard()) );
        layout->addWidget( m_addToClipboard, 1, 1 );
    }

    return m_mainWidget;
}

void CharSelectApplet::slotAddToClipboard()
{
    const QString textLine = m_lineEdit->text();
    QClipboard *cb = QApplication::clipboard();
    cb->setText( textLine,QClipboard::Clipboard );
    cb->setText( textLine,QClipboard::Selection );
}

void CharSelectApplet::slotCharSelect( const QChar &c )
{
    m_lineEdit->setText( c );
}

#include "charselect.moc"
