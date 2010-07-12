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

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <Plasma/DataEngine>
#include <Plasma/Frame>

#include "personwatch.h"


class ContactImage;
class QGraphicsGridLayout;

namespace Plasma {
    class IconWidget;
    class Label;
}

class MessageWidget : public Plasma::Frame
{
    Q_OBJECT

public:
    explicit MessageWidget(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);

    void setFolder(const QString& folderId);
    void setMessage(const QString& messageId);
    void setProvider(const QString& provider);

private Q_SLOTS:
    void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);
    void fromChanged();
    void markMessageRead();

private:
    void buildDialog();
    void setSourceParameter(QString& variable, const QString& value);

    Plasma::DataEngine* m_engine;
    QString m_folderId;
    QString m_messageId;
    QString m_provider;
    QGraphicsGridLayout* m_layout;
    ContactImage* m_image;
    Plasma::Label* m_subjectLabel;
    Plasma::Label* m_bodyLabel;
    Plasma::Label* m_fromLabel;
    Plasma::IconWidget* m_setRead;
    QString m_source;
    PersonWatch m_personWatch;
};


#endif
