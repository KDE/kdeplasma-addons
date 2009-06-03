/***************************************************************************
 *   Copyright  2008 by Thomas Coopman <thomas.coopman@gmail.com>          *
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

#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QString>
#include <QStringList>
#include <QImage>
#include <QObject>

#include <KUrl>

#include <Plasma/DataEngine>

class QTimer;

class SlideShow : public QObject
{
    Q_OBJECT

public:
    explicit SlideShow(QObject *parent);
    ~SlideShow();

    void setDirs(const QStringList &slideShowPaths, bool recursive = false);
    void setImage(const QString &imagePath);
    void setRandom(bool);
    QImage image();
    KUrl currentUrl() const;

    void setUpdateInterval(int msec);

public Q_SLOTS:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

Q_SIGNALS:
    void pictureUpdated();

private:
    void addImage(const QString &imagePath);
    void addDir(const QString &path);
    void addRecursiveDir(const QString &path);
    KUrl url();

private Q_SLOTS:
    void updatePicture();
    void clearPicture();

private:
    QStringList m_picturePaths;
    QStringList m_filters;
    int m_slideNumber;
    bool m_useRandom;

    QList<int> m_indexList;
    int m_randomInt;
    KUrl m_currentUrl;
    QTimer *m_timer;
    QImage m_picture;
};

#endif /*SLIDESHOW_H*/
