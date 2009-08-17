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

// Plasma
#include <Plasma/Label>


using namespace Plasma;

ActivityWidget::ActivityWidget(QGraphicsWidget *parent)
    : Frame(parent)
{
    m_layout = new QGraphicsLinearLayout(this);

    m_image = new ContactImage(this);
    m_image->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    m_image->setMaximumSize(m_image->minimumSize());

    m_layout->addItem(m_image);

    m_messageLabel = new Plasma::Label(this);
    m_layout->addItem(m_messageLabel);
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
    m_messageLabel->setText(data.value("message").toString());
}


Plasma::DataEngine::Data ActivityWidget::activityData() const
{
    return m_atticaData;
}

#include "activitywidget.moc"
