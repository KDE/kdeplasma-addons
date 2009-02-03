/*
 * Copyright (C) 2007, 2008 Petri Damsten <damu@iki.fi>
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

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>
#include "ui_config.h"
#include "ui_feedsConfig.h"

namespace Plasma {
  class WebView;
}
class QGraphicsLinearLayout;

class News : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        News(QObject *parent, const QVariantList &args);
        ~News();

        QGraphicsWidget *graphicsWidget();

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

    protected:
        virtual void createConfigurationInterface(KConfigDialog *parent);
        void connectToEngine();
        QMap<QString, QString> akregatorFeeds();

    protected slots:
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void configAccepted();
        void linkActivated(const QUrl& link);
        void addFeed();
        void removeFeed();
        void feedTextChanged(const QString& text);
        void makeStylesheet();
	void updateSpinBoxSuffix(int interval);

    private:
        QStringList m_feeds;
        QString m_feedstring;
        uint m_interval;
        bool m_showTimestamps;
        bool m_showTitles;
        bool m_showDescriptions;
        Ui::config ui;
        Ui::feedsConfig feedsUi;
        QMap<QString, QString> m_defaultFeeds;
        Plasma::WebView* m_news;
        QGraphicsLinearLayout* m_layout;
        QGraphicsWidget *m_graphicsWidget;
        QString m_cssDir;
        Plasma::DataEngine::Data m_dataCache;
};

K_EXPORT_PLASMA_APPLET(news, News)

#endif
