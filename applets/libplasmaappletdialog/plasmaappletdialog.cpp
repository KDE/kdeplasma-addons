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
#include <KIconLoader>
#include <plasma/widgets/icon.h>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <KIcon>

PlasmaAppletDialog::PlasmaAppletDialog(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_icon(0),
      m_dialog(0),
      m_closePopup( false ),
      m_layout(0),
      m_proxy(0)
{
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize, iconSize);
}

PlasmaAppletDialog::~PlasmaAppletDialog()
{
    if (m_proxy) {
        m_proxy->setWidget(0);
    }
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
    //updateConstraints(Plasma::FormFactorConstraint);
}

void PlasmaAppletDialog::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        m_layout->removeAt(0);
        switch (formFactor()) {
        case Plasma::Planar:
        case Plasma::MediaCenter:
            m_closePopup = false;
            delete m_dialog;
            m_dialog = 0;

            setAspectRatioMode(Plasma::IgnoreAspectRatio);

            if (!m_proxy) {
                m_proxy = new QGraphicsProxyWidget(this);
                m_proxy->setWidget(widget());
                m_proxy->show();
            }

            m_layout->addItem(m_proxy);
            setMinimumSize( m_minimumSize.width(), m_minimumSize.height());
            break;
        case Plasma::Horizontal:
        case Plasma::Vertical:
            m_closePopup = true;
            setAspectRatioMode(Plasma::Square);

            if (m_proxy) {
                m_proxy->setWidget(0); // prevent it from deleting our widget!
                delete m_proxy;
                m_proxy = 0;
            }

            if (!m_dialog) {
                m_dialog = new Plasma::Dialog();
                m_dialog->setWindowFlags(Qt::Popup);
                QVBoxLayout *l_layout = new QVBoxLayout(m_dialog);
                l_layout->setSpacing(0);
                l_layout->setMargin(0);
                l_layout->addWidget(widget());
                m_dialog->adjustSize();
            }

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
