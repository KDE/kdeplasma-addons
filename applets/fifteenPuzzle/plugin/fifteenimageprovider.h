/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

