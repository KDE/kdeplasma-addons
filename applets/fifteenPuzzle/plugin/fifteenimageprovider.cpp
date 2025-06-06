/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fifteenimageprovider.h"
#include "debug_p.h"

#include <QDebug>

FifteenImageProvider::FifteenImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
    , m_boardSize(4)
    , m_pieceWidth(30)
    , m_pieceHeight(30)
{
}

QPixmap FifteenImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize); // wanted sizes are actually encoded in the id

    // id format is boardSize-imagenumber-pieceWidth-pieceHeight-imagePath
    qDebug(PLASMA_FIFTEENPUZZLE) << "pixmap requested with id " << id;

    // MaxSplit used as the path can contains '-' but is the last argument
    auto maxSplit = [](const QString &toSplit, QLatin1Char separator) {
        QStringList parts = toSplit.split(separator);
        QStringList res = parts.mid(0, 4);

        QString last = parts.mid(4).join('-');
        res.append(last);

        return res;
    };
    QStringList idParts = maxSplit(id, QLatin1Char('-'));
    if (idParts.size() < 4) {
        *size = QSize();
        return QPixmap();
    }

    bool update = false;
    int boardSize = idParts.at(0).toInt();
    int pieceWidth = idParts.at(2).toInt();
    int pieceHeight = idParts.at(3).toInt();
    QString path = idParts.at(4);
    if (path != m_imagePath && !path.isEmpty()) {
        m_imagePath = path;
        qDebug(PLASMA_FIFTEENPUZZLE) << "loading pixmap from file " << path << m_pixmap.load(path);
        update = true;
    }

    if (pieceWidth != m_pieceWidth || pieceHeight != m_pieceHeight || m_boardSize != boardSize) {
        m_pieceWidth = pieceWidth;
        m_pieceHeight = pieceHeight;
        m_boardSize = boardSize;
        update = true;
    }

    if (update) {
        updatePixmaps();
    }

    if (idParts.at(1) == QLatin1String("all")) {
        return m_pixmap;
    } else {
        // The applet use 1 based id for pieces
        int number = idParts.at(1).toInt() - 1;

        qDebug(PLASMA_FIFTEENPUZZLE) << "pixmap for piece " << number << " requested";
        if (number >= 0 && number < m_pixmaps.size()) {
            *size = QSize(m_pieceWidth, m_pieceHeight);
            return m_pixmaps.at(number);
        }
    }

    *size = QSize();
    return QPixmap();
}

void FifteenImageProvider::updatePixmaps()
{
    QSize size(m_pieceWidth * m_boardSize, m_pieceHeight * m_boardSize);
    QPixmap copyPixmap = m_pixmap.scaled(size);

    m_pixmaps.clear();
    m_pixmaps.resize(m_boardSize * m_boardSize);

    for (int i = 0; i < m_boardSize * m_boardSize; i++) {
        int posX = (i % m_boardSize) * m_pieceWidth;
        int posY = (i / m_boardSize) * m_pieceHeight;

        m_pixmaps[i] = copyPixmap.copy(posX, posY, m_pieceWidth, m_pieceHeight);
    }
}
