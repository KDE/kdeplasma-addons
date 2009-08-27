/*
    Copyright 2009 by Marco Martin <notmart@gmail.com>

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
#include <QGraphicsLinearLayout>

//KDE
#include <KDebug>
#include <KIconLoader>
#include <KToolInvocation>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Label>


using namespace Plasma;

ActivityWidget::ActivityWidget(QGraphicsWidget *parent)
    : Frame(parent),
      m_isHovered(false)
{
    setAcceptHoverEvents(true);

    m_layout = new QGraphicsLinearLayout(this);

    m_image = new ContactImage(this);
    m_image->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    m_image->setMaximumSize(m_image->minimumSize());

    m_layout->addItem(m_image);

    m_messageLabel = new Plasma::Label(this);
    m_layout->addItem(m_messageLabel);

    int s = KIconLoader::SizeSmallMedium; // The size for the action icons

    m_link = new Plasma::IconWidget(this);
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

ActivityWidget::~ActivityWidget()
{
}

void ActivityWidget::setPixmap(const QPixmap &pixmap)
{
    m_image->setPixmap(pixmap);
}

void ActivityWidget::setActivityData(Plasma::DataEngine::Data data)
{
    m_atticaData = data;
    m_messageLabel->setText(i18n("<b>%1:</b><br />%2")
        .arg(data.value("user").toString())
        .arg(data.value("message").toString()));
    updateActions();
}


Plasma::DataEngine::Data ActivityWidget::activityData() const
{
    return m_atticaData;
}


void ActivityWidget::followLink()
{
    KToolInvocation::invokeBrowser(m_atticaData["link"].toString());
}


void ActivityWidget::updateActions()
{
    m_link->setVisible(m_isHovered && !m_atticaData.value("link").toString().isEmpty());
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
