/*
 * Copyright 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef FIFTEENIMAGEPROVIDER_H
#define FIFTEENIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QString>
#include <QVector>

class FifteenImageProvider : public QQuickImageProvider
{
public:
    FifteenImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    // Update our pixmaps, called when sizes change or boardSize changes
    void updatePixmaps();

    QString m_imagePath;
    QPixmap m_pixmap;
    int m_boardSize;
    int m_pieceWidth;
    int m_pieceHeight;

    QVector<QPixmap> m_pixmaps;
};

#endif

