/*
 *   Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef BLUEMARBLE_H
#define BLUEMARBLE_H

#include <plasma/glapplet.h>

class GLUquadric;
class GLTexture;
class GLShader;

class BlueMarble : public Plasma::GLApplet
{
Q_OBJECT
public:
    BlueMarble(QObject *parent, const QVariantList &args);

    ~BlueMarble();

    qreal width() const {
        return geometry().width();
    }
    qreal height() const {
        return geometry().height();
    }
    virtual void paintGLInterface(QPainter *painter,
                                  const QStyleOptionGraphicsItem *option);

    virtual void initializeGL();

protected:
//     void mousePressEvent(QGraphicsSceneMouseEvent *);
//     void mouseMoveEvent(QGraphicsSceneMouseEvent *);
    void timerEvent(QTimerEvent *);

    void generateModel();

private:
    GLUquadric* mSphere;
    GLTexture* mDayTexture;
    GLTexture* mNightTexture;
    GLShader* mShader;
    GLShader* mAtmosphereShader;

    float mRotX, mRotZ;
    float mSunAngle;
    QPointF mLastMousePos;
};

K_EXPORT_PLASMA_APPLET(bluemarble, BlueMarble)

#endif
