/*
 * Copyright 2007, 2008  Petri Damsten <damu@iki.fi>
 * Copyright 2008  Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NEWS_HEADER
#define NEWS_HEADER

//Plasma
#include <Plasma/Applet>
#include <Plasma/DataEngine>

#include <QTimer>

//Own
#include "ui_config.h"
#include "ui_feedsConfig.h"
#include "header.h"
#include "feeddata.h"


class Scroller;
class QGraphicsLinearLayout;

class News : public Plasma::Applet
{
   Q_OBJECT
   public:
        News(QObject *parent, const QVariantList &args);
        ~News();

        void init();

    public slots:
        void configChanged();
        void createConfigurationInterface(KConfigDialog *parent);
        void switchItems();

    protected:
        void constraintsEvent(Plasma::Constraints constraints);

    protected Q_SLOTS:
        void dropEvent(QGraphicsSceneDragDropEvent *event);

    private slots:
        void configAccepted();
        void addFeed();
        void removeFeed();
        void slotItemChanged();
        void slotChangeText( const QString& text );

    private:
        QStringList         m_feedlist;
        QList<Scroller *>   m_scrollerList;
        QTimer              *m_timer;

        uint                m_interval;
        uint                m_switchInterval;

        bool                m_animations;
        bool                m_logo;
        bool                m_showdroptarget;
        bool                m_collapsed;

        int                 m_maxAge;

        QGraphicsLinearLayout  *m_layout;
        Header              *m_header;
        Ui::config          ui;
        Ui::feedsConfig     feedsUi;

        QMap<QString, QString>    m_defaultFeeds;
        QMap<QString, QString> akregatorFeeds();

        void connectToEngine();
        void updateScrollers();
        QString fuzzyDate(const QDateTime& date);
};

K_EXPORT_PLASMA_APPLET(rssnow, News)

#endif
