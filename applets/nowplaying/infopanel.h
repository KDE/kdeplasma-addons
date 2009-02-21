/*
 * Copyright 2008  Alex Merry <alex.merry@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <QGraphicsWidget>
#include <QMap>
#include <QString>

namespace Plasma {
    class Label;
}
class QGraphicsLayoutItem;
class QGraphicsLinearLayout;
class QGraphicsGridLayout;

class AlbumArt;

class InfoPanel : public QGraphicsWidget
{
    Q_OBJECT

public:
    InfoPanel(QGraphicsWidget *parent = 0);
    ~InfoPanel();

public slots:
    void updateMetadata(const QMap<QString,QString>& metadata);
    void updateArtwork(const QPixmap &artwork);

private:
    void updateLabels();

    AlbumArt* m_artwork;
    Plasma::Label* m_artistLabel;
    Plasma::Label* m_titleLabel;
    Plasma::Label* m_albumLabel;
    Plasma::Label* m_timeLabel;

    Plasma::Label* m_artistText;
    Plasma::Label* m_titleText;
    Plasma::Label* m_albumText;
    Plasma::Label* m_timeText;

    QGraphicsLinearLayout *m_barLayout;
    QGraphicsGridLayout* m_layout;
    QMap<QString,QString> m_metadata;
};


#endif // INFOPANEL_H
