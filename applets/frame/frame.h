/***************************************************************************
 *   Copyright  2007 by Anne-Marie Mahfouf <annma@kde.org>              *
 *   Copyright  2007 by Antonio Vinci <mercurio@personellarete.it>      *
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

#ifndef FRAME_H
#define FRAME_H

#include <QPaintDevice>
#include <QLabel>
#include <QPixmap>
#include <QPaintEvent>
#include <QPainter>
#include <QGraphicsItem>
#include <QColor>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/phase.h>

#include "ui_config.h"

class KDialog;
class QGraphicsSceneDragDropEvent;

namespace Plasma
{
    class Layout;
}

class Frame : public Plasma::Applet
{
    Q_OBJECT
    public:
        Frame(QObject *parent, const QVariantList &args);

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            const QRect &contentsRect);
        void init();
        void constraintsUpdated(Plasma::Constraints constraints);

    public slots:
        void showConfigurationInterface();

    protected Q_SLOTS:
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
        void configAccepted();
        void choosePicture(const KUrl& currentUrl);
        void setSlideShow();

    private Q_SLOTS:
        void addDir();
        void removeDir();

    private:
        void paintCache(const QStyleOptionGraphicsItem *option,
                        const QSize &contentsSize);

        /// The current color of the frame
        QColor m_frameColor;
        KDialog *m_dialog; // should we move this into another class?
        /// true if the user wants a frame. If false, there's only the black border around the picture
        bool m_frame;
        /// If true, the frame will have rounded corners
        bool m_roundCorners;
        /// If true, the picture will have a drop shadow.
        bool m_shadow;
        /// true if the user wants Wikipedia Picture of the Day (unused)
        bool m_potd;
        /// Stores the current picture URL when slideShow is false. Wikipedia Picture of the Day is default.
        KUrl m_currentUrl;
        /// The current slideshow folder
        QStringList m_slideShowPaths;
        bool m_slideShow;
        unsigned int m_slideNumber;
        QTimer *m_slideShowTimer;
        int m_slideshowTime;
        /// The current picture
        QImage m_picture;
        /// Off-Screen pixmap
        QPixmap m_pixmapCache;
        /// Frame & shadow outline thickness
        int m_frameOutline;
        int m_swOutline;
        /// Max Picture Dimension
        static const int m_maxDimension=800;
        /// Designer Config file
        Ui::config ui;
        /// Default image
        QImage loadDefaultImage(QString message);
};

K_EXPORT_PLASMA_APPLET(frame, Frame)

#endif
