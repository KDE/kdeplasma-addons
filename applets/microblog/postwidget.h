/*
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef POSTWIDGET_H
#define POSTWIDGET_H

#include <KTimeZone>

#include <Plasma/DataEngine>
#include <Plasma/Frame>

class KColorScheme;

namespace Plasma
{
class Label;
class IconWidget;
class TextBrowser;
class ToolButton;
}

class PostWidget : public Plasma::Frame
{
    Q_OBJECT
public:
    PostWidget(QGraphicsWidget *parent);
    ~PostWidget();

    void setData(const Plasma::DataEngine::Data &data);
    void setPicture(const QPixmap &picture);
    void setColorScheme(KColorScheme *scheme);
    void setActionsShown(bool show);

protected:
    QString timeDescription(const QDateTime &dt);

protected Q_SLOTS:
    void askReply();
    void askForward();
    void askFavorite();
    void askProfile();

Q_SIGNALS:
    void reply(const QString messageId, const QString &reply);
    void forward(const QString &messageId);
    void favorite(const QString &messageId, const bool favorite);
    void openProfile(const QString &profile);

private:
    QString m_messageId;
    Plasma::Label *m_author;
    Plasma::IconWidget *m_picture;
    Plasma::Label *m_from;
    Plasma::TextBrowser *m_text;
    Plasma::ToolButton *m_replyButton;
    Plasma::ToolButton *m_forwardButton;
    Plasma::ToolButton *m_favoriteButton;
    bool m_isFavorite;
    KTimeZone m_tz;
    KColorScheme *m_colorScheme;
};

#endif
