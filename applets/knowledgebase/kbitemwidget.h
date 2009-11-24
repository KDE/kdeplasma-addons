/***************************************************************************
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
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

#ifndef KBITEMWIDGET_H
#define KBITEMWIDGET_H

//Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>

#include <Plasma/PopupApplet>
#include <Plasma/Frame>
#include <Plasma/DataEngine>

namespace Plasma
{
    class IconWidget;
    class Label;
}

class KBItemWidget : public Plasma::Frame
{
    Q_OBJECT

public:
    KBItemWidget(QGraphicsWidget *parent = 0);
    virtual ~KBItemWidget();

    void setAtticaData(const Plasma::DataEngine::Data &data);

    void setPixmap(const QPixmap &pixmap);
    Plasma::DataEngine::Data data();

    void setDetailsShown(const bool show);

    Plasma::PopupApplet* m_applet;

    QGraphicsWidget *dragTitle() const;

public Q_SLOTS:
    void openBrowser();
    void openProfile();
    void toggleDetails();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

Q_SIGNALS:
    void detailsVisibilityChanged(KBItemWidget *item, bool shown);

private :
    void updateActions();

    Plasma::DataEngine::Data m_ocsData;
    QString m_stylesheet;
    bool m_isHovered;
    int m_currentPage;
    int m_totalPages;

    Plasma::Label *m_title;
    Plasma::Label* m_category;
    QGraphicsLinearLayout *m_layout;
    QPixmap m_pixmap;
    QGraphicsWidget *m_detailsWidget;
    Plasma::IconWidget *m_openBrowser;
};

#endif

