/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "friendmanagementwidget.h"

#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QLabel>

#include <KConfigGroup>
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

#include "contactimage.h"
#include "utils.h"


using namespace Plasma;

FriendManagementWidget::FriendManagementWidget(DataEngine* engine, QGraphicsWidget* parent)
    : Frame(parent), m_isHovered(false), m_personWatch(engine), m_engine(engine)
{
    setAcceptHoverEvents(true);
    buildDialog();
    updateActions();
    setMinimumHeight(40);
    setMinimumWidth(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    connect(&m_personWatch, SIGNAL(updated()), SLOT(updated()));
}


void FriendManagementWidget::buildDialog()
{
    int avatarSize = KIconLoader::SizeMedium;
    int actionsSize = KIconLoader::SizeSmallMedium;

    m_infoLabel = new Plasma::Label;
    m_infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_infoLabel->setMinimumWidth(avatarSize * 2);

    m_statusLabel = new Plasma::Label;
    m_statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_statusLabel->setMinimumWidth(avatarSize * 2);
    m_statusLabel->setText(i18n("<i>Accepting friendship...</i>"));

    m_avatar = new ContactImage(0);
    m_avatar->setMinimumHeight(avatarSize);
    m_avatar->setMinimumWidth(avatarSize);
    m_avatar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_actionAccept = new IconWidget;
    m_actionAccept->setIcon("dialog-ok");
    m_actionAccept->setToolTip(i18n("Accept friendship"));
    m_actionAccept->setMinimumHeight(actionsSize);
    m_actionAccept->setMaximumHeight(actionsSize);
    m_actionAccept->setMinimumWidth(actionsSize);
    m_actionAccept->setMaximumWidth(actionsSize);

    m_actionDecline = new IconWidget;
    m_actionDecline->setIcon("dialog-cancel");
    m_actionDecline->setToolTip(i18n("Decline friendship"));
    m_actionDecline->setMinimumHeight(actionsSize);
    m_actionDecline->setMaximumHeight(actionsSize);
    m_actionDecline->setMinimumWidth(actionsSize);
    m_actionDecline->setMaximumWidth(actionsSize);

    m_actions = new QGraphicsLinearLayout;
    m_actions->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_actions->addItem(m_actionAccept);
    m_actions->addItem(m_actionDecline);

    layout = new QGraphicsGridLayout;
    layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->setColumnFixedWidth(0, int(avatarSize * 1.2));
    layout->setHorizontalSpacing(4);
    layout->addItem(m_avatar, 0, 0, 2, 1, Qt::AlignTop);
    layout->addItem(m_infoLabel, 0, 1, 1, 1, Qt::AlignCenter | Qt::AlignHCenter);
    layout->addItem(m_actions, 1, 1, 1, 1, Qt::AlignBottom | Qt::AlignRight);

    setLayout(layout);

    connect(m_actionAccept, SIGNAL(clicked()), SLOT(accept()));
    connect(m_actionDecline, SIGNAL(clicked()), SLOT(accept()));
}


void FriendManagementWidget::accept()
{
    Service* service = m_engine->serviceForSource(personQuery(m_provider, m_id));
    KConfigGroup cg = service->operationDescription("approveFriendship");
    KJob *job = service->startOperationCall(cg);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
    delete service;
}


void FriendManagementWidget::decline()
{
    Service* service = m_engine->serviceForSource(personQuery(m_provider, m_id));
    KConfigGroup cg = service->operationDescription("declineFriendship");
    KJob *job = service->startOperationCall(cg);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
    delete service;
}


void FriendManagementWidget::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)
    m_isHovered = true;
    updateActions();
}


void FriendManagementWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)
    m_isHovered = false;
    updateActions();
}


void FriendManagementWidget::setId(const QString& id)
{
    m_id = id;
    m_personWatch.setId(m_id);
}


void FriendManagementWidget::setProvider(const QString& provider)
{
    m_provider = provider;
    m_personWatch.setProvider(provider);
}


void FriendManagementWidget::updateActions()
{
    m_actionAccept->setVisible(m_isHovered);
    m_actionDecline->setVisible(m_isHovered);
}


void FriendManagementWidget::updated()
{
    QString firstName = m_personWatch.data().value("FirstName").toString();
    QString lastName = m_personWatch.data().value("LastName").toString();
    if (!firstName.isEmpty() || !lastName.isEmpty()) {
        m_infoLabel->setText(i18n("<b>%1 %2 (%3)</b> wants to be your friend", firstName, lastName, m_id));
    } else {
        m_infoLabel->setText(i18n("<b>%1</b> wants to be your friend", m_id));
    }
    m_avatar->setUrl(m_personWatch.data().value("AvatarUrl").toUrl());
}


#include "friendmanagementwidget.moc"
