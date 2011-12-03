/***************************************************************************
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>                 *
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

#ifndef PICTURE_H
#define PICTURE_H


#include <KUrl>
#include <KIO/StoredTransferJob>
#include <KDirWatch>

/**
 * @brief Picture choice
 * @author Anne-Marie Mahfouf <annma@kde.org>
 *
 * This class handles the choice of the picture and
 * makes it ready for the Frame class to paint this picture.
 */

class Picture : public QObject
{
    Q_OBJECT

public:
    explicit Picture(QObject *parent);
    ~Picture();
    /**
    * Set Default picture with written message @p message if no picture or folder was chosen
    * by the user
    **/
    QImage defaultPicture(const QString &message);
    /**
    * Set picture from location @p currentUrl
    **/
    void setPicture(const KUrl &currentUrl);
    KUrl url();
    QString message();
    void setMessage(const QString &message);
    void setAllowNullImages(bool allowNull);
    bool allowNullImages() const;

Q_SIGNALS:
    void pictureLoaded(QImage image);

private Q_SLOTS:
    void slotFinished(KJob *job);
    void reload();
    void checkImageLoaded(const QImage &newImage);
    void customizeEmptyMessage();

private:
    void setPath(const QString &path);
    KUrl m_currentUrl;
    QString m_path; // The local path of the image on disk
    KDirWatch *m_fileWatch;
    QString m_message;
    QString m_defaultImage;
    bool m_checkDir;
    bool m_allowNullImages;
};


#endif

