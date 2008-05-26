/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "plasmaappletdialog.h"


#include <QVBoxLayout>
#include <KStandardDirs>
#include <KIconLoader>
#include <plasma/widgets/icon.h>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <KIcon>

PlasmaAppletDialog::PlasmaAppletDialog(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , m_icon( 0 )
    , m_closePopup( false )
    , m_layout( 0 )
    , m_proxy(0)
{
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize, iconSize);
}

PlasmaAppletDialog::~PlasmaAppletDialog()
{
    delete m_dialog;
    delete m_icon;
}

void PlasmaAppletDialog::initMinimumSize()
{
    m_minimumSize = QSize( 300, 300 );
}

void PlasmaAppletDialog::initialize()
{
    m_icon = new Plasma::Icon(KIcon("icons"), QString(), this);
}

void PlasmaAppletDialog::init()
{
    initialize();
    initMinimumSize();
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMaximumSize(INT_MAX, INT_MAX);
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_layout->setMaximumSize(INT_MAX, INT_MAX);
    m_layout->setOrientation(Qt::Horizontal);
    setLayout(m_layout);

    connect(m_icon, SIGNAL(clicked()), this, SLOT(slotOpenDialog()));

    m_dialog = new Plasma::Dialog();

    m_proxy = new QGraphicsProxyWidget(this);

    QVBoxLayout *l_layout = new QVBoxLayout();
    l_layout->setSpacing(0);
    l_layout->setMargin(0);


    l_layout->addWidget( widget() );

    m_dialog->setLayout( l_layout );
    m_dialog->adjustSize();

    constraintsUpdated(Plasma::FormFactorConstraint);
}

void PlasmaAppletDialog::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        // Plasma::Dialog already has standard background
        setBackgroundHints(NoBackground);
        m_layout->removeAt(0);
        switch (formFactor()) {
        case Plasma::Planar:
        case Plasma::MediaCenter:
            m_closePopup = false;
            setAspectRatioMode(Plasma::IgnoreAspectRatio);
            m_dialog->setWindowFlags(Qt::Widget);
            m_proxy->setWidget(m_dialog);
            m_proxy->show();
            m_layout->addItem(m_proxy);
            setMinimumSize( m_minimumSize.width(), m_minimumSize.height());
            break;
        case Plasma::Horizontal:
        case Plasma::Vertical:
            m_closePopup = true;
            setAspectRatioMode(Plasma::Square);
            m_dialog->setWindowFlags(Qt::Popup);
            m_proxy->setWidget(0);
            m_proxy->hide();
            m_layout->addItem(m_icon);
            break;
        }
    }
}


void PlasmaAppletDialog::slotOpenDialog()
{
    if (m_dialog->isVisible()) {
        m_dialog->hide();
    } else {
        m_dialog->move(popupPosition(m_dialog->sizeHint()));
        m_dialog->show();
    }

    m_dialog->clearFocus();
}

#include "plasmaappletdialog.moc"
