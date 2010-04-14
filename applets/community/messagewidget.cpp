/***************************************************************************
 *   Copyright (c) 2009 by Eckhart Wörner <ewoerner@kde.org>               *
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

#include "messagewidget.h"

#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsLinearLayout>

#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>

#include "contactimage.h"
#include "utils.h"


using namespace Plasma;

MessageWidget::MessageWidget(DataEngine* engine, QGraphicsWidget* parent)
    : Frame(parent),
      m_engine(engine),
      m_personWatch(engine)
{
    buildDialog();
    setMinimumHeight(40);
    setMinimumWidth(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    connect(&m_personWatch, SIGNAL(updated()), SLOT(fromChanged()));
}


void MessageWidget::fromChanged()
{
    QString id = m_personWatch.data().value("Id").toString();
    QString firstName = m_personWatch.data().value("FirstName").toString();
    QString lastName = m_personWatch.data().value("LastName").toString();
    QString avatarUrl = m_personWatch.data().value("AvatarUrl").toString();
    if (!firstName.isEmpty() || !lastName.isEmpty()) {
        m_fromLabel->setText(i18n("<i>From %1 %2 (%3)</i>", firstName, lastName, id));
    } else {
        m_fromLabel->setText(i18n("<i>From %1</i>", id));
    }
    m_image->setUrl(avatarUrl);
}


void MessageWidget::setFolder(const QString& folderId)
{
    setSourceParameter(m_folderId, folderId);
}


void MessageWidget::setMessage(const QString& messageId)
{
    setSourceParameter(m_messageId, messageId);
}


void MessageWidget::setProvider(const QString& provider)
{
    setSourceParameter(m_provider, provider);
    m_personWatch.setProvider(m_provider);
}


void MessageWidget::setSourceParameter(QString& variable, const QString& value)
{
    if (!m_source.isEmpty()) {
        m_engine->disconnectSource(m_source, this);
    }
    variable = value;
    m_source = messageSummaryQuery(m_provider, m_folderId, m_messageId);
    if (!m_source.isEmpty()) {
        m_engine->connectSource(m_source, this);
    }
}


void MessageWidget::dataUpdated(const QString& source, const DataEngine::Data& data)
{
    if (source != m_source) {
        return;
    }

    DataEngine::Data m_ocsData = data.value(messageAddPrefix(m_messageId)).value<DataEngine::Data>();
    m_subjectLabel->setText(QString("<b>%1</b>").arg(m_ocsData.value("Subject").toString()));
    m_personWatch.setId(m_ocsData.value("From-Id").toString());
    m_bodyLabel->setText(m_ocsData.value("Body").toString());

    m_image->setUrl(m_ocsData.value("AvatarUrl").toUrl());
    m_setRead->setVisible(m_ocsData.value("Status").toString() == "unread");
}


void MessageWidget::buildDialog()
{
    int avatarSize = KIconLoader::SizeMedium;
    int actionSize = KIconLoader::SizeSmallMedium;


    m_image = new ContactImage(m_engine);
    m_image->setMinimumHeight(avatarSize);
    m_image->setMinimumWidth(avatarSize);
    m_image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_subjectLabel = new Label;
    m_subjectLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_subjectLabel->setMinimumWidth(avatarSize * 2);

    m_fromLabel = new Label;
    m_fromLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_bodyLabel = new Label;
    m_bodyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_setRead = new Plasma::IconWidget;
    m_setRead->setIcon("mail-unread-new");
    m_setRead->setToolTip(i18n("Mail is unread, mark as read"));
    m_setRead->setMinimumHeight(actionSize);
    m_setRead->setMaximumHeight(actionSize);
    m_setRead->setMinimumWidth(actionSize);
    m_setRead->setMaximumWidth(actionSize);
    m_setRead->setVisible(false);

    m_layout = new QGraphicsGridLayout;
    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->setColumnFixedWidth(0, int(avatarSize * 1.2));
    m_layout->setHorizontalSpacing(4);
    m_layout->addItem(m_image, 0, 0, 3, 1, Qt::AlignTop);
    m_layout->addItem(m_setRead, 0, 1, 1, 1, Qt::AlignTop);
    m_layout->addItem(m_subjectLabel, 0, 2, 1, 2, Qt::AlignTop);
    m_layout->addItem(m_fromLabel, 1, 2, 1, 1, Qt::AlignTop);
    m_layout->addItem(m_bodyLabel, 2, 2, 1, 2, Qt::AlignTop);

    setLayout(m_layout);

    connect(m_setRead, SIGNAL(clicked()), SLOT(markMessageRead()));
}

void MessageWidget::markMessageRead()
{
    m_engine->query(messageQuery(m_provider, m_folderId, m_messageId));
}


#include "messagewidget.moc"
