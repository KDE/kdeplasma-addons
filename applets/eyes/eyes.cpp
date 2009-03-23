/***************************************************************************
 *   Copyright 2008-2009 by Olivier Goffart <ogoffart@kde.org>             *
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

#include "eyes.h"

#include <QPainter>
#include <QPixmap>
#include <QGraphicsView>

#include <Plasma/Theme>
#include <KDialog>
#include <KDebug>

#include <math.h>

Eyes::Eyes(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args) , timerInterval(50),  previousMousePos(-1,-1)
{
    resize(192, 128);

    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("widgets/eyes");
    m_svg->setContainsMultipleImages(true);

    rightPupil = new Plasma::SvgWidget(this);
    rightPupil->setSvg(m_svg);
    rightPupil->setElementID("rightPupil");

    leftPupil = new Plasma::SvgWidget(this);
    leftPupil->setSvg(m_svg);
    leftPupil->setElementID("leftPupil");

    timerId = startTimer(50);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(false);
}

Eyes::~Eyes()
{

}

void Eyes::constraintsEvent(Plasma::Constraints constraints)
{
    Q_UNUSED(constraints)

    if (constraints & Plasma::FormFactorConstraint) {
        setBackgroundHints(NoBackground);
    }

    if (constraints & Plasma::SizeConstraint) {
        if(formFactor() == Plasma::Vertical) {
            setMinimumSize(QSizeF(0, boundingRect().width()/1.5));
            setMaximumSize(QSizeF(-1, boundingRect().width()/1.5));
        } else if(formFactor() == Plasma::Horizontal) {
            setMinimumSize(QSizeF(boundingRect().height()*1.5,0));
            setMaximumSize(QSizeF(boundingRect().height()*1.5,-1));
        } else { 
            setMinimumSize(QSizeF());
            setMaximumSize(QSizeF());
        }

        double pupilSize = qMin( qMin(boundingRect().width()/2, boundingRect().height()) / 5,
                                 (boundingRect().width()/2 + boundingRect().height()) / 12);

        leftPupil->resize(pupilSize, pupilSize);
        rightPupil->resize(pupilSize, pupilSize);
    }
    previousMousePos = QPoint(-1,-1);
}

void Eyes::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect)
{
    Q_UNUSED(option)
    QRect rect = contentsRect;
    rect.setWidth(rect.width()/2 - 2);
    m_svg->paint(p, rect, "leftEye");
    rect.translate(rect.width() + 2*2 , 0);
    m_svg->paint(p, rect, "rightEye");
}



static QPointF pupilPos( const QRectF &eyesRect, const QPointF &mousePos)
{
    const QPointF vect = mousePos - eyesRect.center();     //cursor position relative to the center of the eyes
    const qreal abs_vect = vect.x() * vect.x() + vect.y() * vect.y();

    if (qFuzzyCompare(vect.x() + 1 , qreal(1.0))) {
        if (vect.y() > eyesRect.height()/2) {
            return eyesRect.center() + QPoint( 0, eyesRect.height()/2);
        } else if (vect.y() < -eyesRect.height()/2) {
            return eyesRect.center() + QPoint( 0, -eyesRect.height()/2);
        } else {
            return mousePos;
        }
    }

    const qreal a = eyesRect.width() / 2;
    const qreal b = eyesRect.height() / 2;
    const qreal tan_alpha = vect.y() / vect.x();

    /* 
        the pupil need to be on the intersection between the line
           y = x * tan_alpha
        and the ellipse
           x^2/a^2 + y^2/b^2 
    */

    qreal x = a*b / sqrt(b*b + a*a * tan_alpha*tan_alpha);
    if (vect.x() < 0) {
        x = -x;
    }
    const qreal y = x*tan_alpha;

    if (abs_vect < (x * x) + (y * y)) {
        return mousePos;
    }

    return eyesRect.center() + QPointF(x, y);
}

void Eyes::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId) {
        Plasma::Applet::timerEvent(e);
        return;
    }

    QPoint absMousePos = QCursor::pos();

    if (absMousePos == previousMousePos) {
        if (timerInterval > 300)
            return;
        timerInterval += 50;
        killTimer(timerId);
        timerId = startTimer(timerInterval);
        return;
    }

    if (timerInterval != 50) {
        timerInterval = 50;
        killTimer(timerId);
        timerId = startTimer(timerInterval);
    }

    QGraphicsView *myview = view();
    if (!myview) {
        return;
    }

    previousMousePos = absMousePos;
    //cursor position relative to the item coordonate
    QPointF mousePos = mapFromScene( myview->mapToScene( myview->mapFromGlobal( absMousePos ) ) );

    const QRectF bounding = boundingRect();
    const qreal paddingX = bounding.width() / 9;
    const qreal paddingY = bounding.height() / 5;

    QRectF eyesRect = boundingRect();
    // left pupil
    eyesRect.setWidth(eyesRect.width()/2 - 2);
    leftPupil->setPos(pupilPos(eyesRect.adjusted(paddingX,paddingY,-paddingX,-paddingY), mousePos)
                           - leftPupil->boundingRect().center());

    //right pupil
    eyesRect.translate(eyesRect.width() + 2*2 , 0);
    rightPupil->setPos(pupilPos(eyesRect.adjusted(paddingX,paddingY,-paddingX,-paddingY), mousePos)
                             - rightPupil->boundingRect().center());
}


#include "eyes.moc"
