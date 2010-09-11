/*
    Copyright 2009 by Marco Martin <notmart@gmail.com>
    Copyright 2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "activitywidget.h"
#include "contactimage.h"

//Qt
#include <QtCore/QUrl>
#include <QGraphicsLinearLayout>

//KDE
#include <KDebug>
#include <KIconLoader>
#include <KToolInvocation>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Applet>

using namespace Plasma;

ActivityWidget::ActivityWidget(DataEngine* engine, QGraphicsWidget* parent)
    : Frame(parent),
      m_link(0),
      m_isHovered(false)
{
    setAcceptHoverEvents(true);

    m_layout = new QGraphicsLinearLayout(this);
    
    m_image = new ContactImage(engine, this);
    m_image->setMinimumSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    m_image->setMaximumWidth(m_image->minimumSize().width());
    m_image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    m_layout->addItem(m_image);

    m_messageLabel = new Plasma::Label(this);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_layout->addItem(m_messageLabel);

    int s = KIconLoader::SizeSmallMedium; // The size for the action icons
    
    Applet *parentApplet = qobject_cast<Applet *>(parent);
    m_link = new Plasma::IconWidget(this);

    if (parentApplet && parentApplet->hasAuthorization("LaunchApp"))
    {
        m_link->setIcon("go-jump");
        m_link->setToolTip(i18n("More information"));
        m_link->setMinimumHeight(s);
        m_link->setMaximumHeight(s);
        m_link->setMinimumWidth(s);
        m_link->setMaximumWidth(s);
        m_layout->addItem(m_link);
        m_link->setVisible(true);
        connect(m_link, SIGNAL(clicked()), this, SLOT(followLink()));
    }
    updateActions();
}

ActivityWidget::~ActivityWidget()
{
}

void ActivityWidget::setActivityData(Plasma::DataEngine::Data data)
{
    m_atticaData = data;
    const QString user = data.value("user").toString();
    const QString message = data.value("message").toString();
    m_timestamp = data.value("timestamp").toDateTime();
    
    if (!message.startsWith(user)) {
        m_messageLabel->setText(i18n("%1: <i>%2</i>", user, message));
    } else {
        m_messageLabel->setText(message);
    }
    m_image->setUrl(data.value("user-AvatarUrl").toUrl());

    updateActions();
}

QString ActivityWidget::message() const
{
    return m_messageLabel->text();
}

QDateTime ActivityWidget::timestamp() const
{
    return m_timestamp;
}

Plasma::DataEngine::Data ActivityWidget::activityData() const
{
    return m_atticaData;
}


void ActivityWidget::followLink()
{
    KToolInvocation::invokeBrowser(m_atticaData["link"].value<QUrl>().toString());
}


void ActivityWidget::updateActions()
{
    if (m_link) {
        m_link->setVisible(m_isHovered && m_atticaData.value("link").value<QUrl>().isValid());
    }
}


void ActivityWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_isHovered = true;
    updateActions();
}


void ActivityWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_isHovered = false;
    updateActions();
}


#include "activitywidget.moc"
