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
    //class DataEngine;
}

class Frame : public Plasma::Applet
{
    Q_OBJECT
    public:
        Frame(QObject *parent, const QVariantList &args);
        ~Frame();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
	QRectF boundingRect() const;
        QPainterPath shape() const;
        void constraintsUpdated(Plasma::Constraints constraints);

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void showConfigurationInterface();

    protected Q_SLOTS:
	void dropEvent(QGraphicsSceneDragDropEvent *event);
        void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
	void resizeEvent( QResizeEvent * );
	void configAccepted();
	void choosePicture(const KUrl& currentUrl);
	void setSlideShow();
	void composePicture(QPainter* painter = NULL);
        void updateSizes();
        
    protected:
       

    private:
	
	/// The current color of the frame
        QColor m_frameColor;
        KDialog *m_dialog; //should we move this into another class?
	/// true if the user wants a frame. If false, there's only the black border around the picture
	bool m_frame;
	/// If true, the frame will have squared corners
	bool m_squareCorners;
	/// If true, the frame will have round corners 
	bool m_roundCorners;
	/// If true, the picture will have a drop shadow.
	bool m_shadow;
	/// true if the user wants Wikipedia Picture of the Day (unused)
	bool m_potd;
	/// Stores the current picture URL when slideShow is false. Wikipedia Picture of the Day is default.
	KUrl m_currentUrl;
	/// The current slideshow folder
	KUrl m_slideShowUrl;
        bool m_slideShow;
        unsigned int m_slideNumber;
        QTimer *slideShowTimer;
	int m_slideshowTime;
	/// The current picture
	QImage myPicture;
	/// Off-Screen pixmap
	QPixmap *m_cmpPicture;
        ///Unused (for now)
	//Plasma::Layout *m_layout;
	/// The size of the picture, it's max(width, height) that is set
	int m_pixelSize;
        /// Pixmap rect
        QRect m_pixmapRect;
        /// Pixmap rect + frame/shadow outline
        QRect m_pixmapOutlineRect;
        /// Plasmoid Bounding Rect
        QRect m_boundingRect;
        /// Frame & shadow outline thickness
        int m_frameOutline;
        int m_swOutline;
        /// Max Picture Dimension
        static const int m_maxDimension=800;
        /// Utility functions
        double angleForPos(QPointF in);
        double distanceForPos(QPointF in);
        QPolygon mapToPicture(const QPolygon in) const;
        /// Used by mousemove method to skip some paint events.
        int m_skipPaint;
	/// Designer Config file
	Ui::config ui;
        /// Default image
        QImage loadDefaultImage(QString message);
       /*/Debug timer
        QTime debugt;
        int debugtp;
        void cT(QString step);*/
};

/*
inline 
void Frame::cT (QString step) {
    if (step == "start") {
        debugtp = 0;
        kDebug() << "Start measurement-----\n";
        debugt.start();
    }
    else {
        int t = debugt.restart();
        debugtp+=t;
        kDebug() << "Time at step " << step << ":\t" << debugtp << "\t" <<t << "\n";
    }
}
-------------------- */
K_EXPORT_PLASMA_APPLET(frame, Frame)

#endif
