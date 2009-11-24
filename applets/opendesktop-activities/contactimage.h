/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
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

#ifndef CONTACTIMAGE_H
#define CONTACTIMAGE_H

//Qt
#include <QtCore/QUrl>
#include <QtGui/QGraphicsWidget>

#include <Plasma/DataEngine>


class ContactImage : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit ContactImage(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);
    void setUrl(const QUrl& url);

private Q_SLOTS:
    void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    void pixmapUpdated();

    Plasma::DataEngine* m_engine;

    int border;
    QColor fg;
    QColor bg;

    QPixmap m_pixmap;
    QPixmap m_scaledPixmap;

    QString m_source;
};


#endif
