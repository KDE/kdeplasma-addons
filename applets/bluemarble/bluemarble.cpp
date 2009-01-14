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

#include "bluemarble.h"

// has to be first
#include "kwinglutils.h"

#include <QGraphicsSceneMouseEvent>

#include <kdebug.h>
#include <kstandarddirs.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <math.h>



BlueMarble::BlueMarble(QObject *parent, const QVariantList &args)
    : Plasma::GLApplet(parent, args)
{
    setHasConfigurationInterface(false);
    setAcceptsHoverEvents(true);

    mDayTexture = 0;
    mNightTexture = 0;
    mShader = 0;
    mAtmosphereShader = 0;

    mRotX = 30;
    mRotZ = 0;
    mSunAngle = 0;

    resize(400, 400);
    initializeGL();
}

BlueMarble::~BlueMarble()
{
    delete mDayTexture;
    delete mNightTexture;
    delete mShader;
    delete mAtmosphereShader;
}

void BlueMarble::generateModel()
{
    mSphere = gluNewQuadric();
    gluQuadricTexture(mSphere, GL_TRUE);
}

/*
void BlueMarble::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if (!(e->buttons() & Qt::LeftButton)) {
        return;
    }
    mLastMousePos = e->pos();
}

void BlueMarble::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (!(e->buttons() & Qt::LeftButton)) {
        return;
    }

    mRotZ += mLastMousePos.x() - e->pos().x();
    mRotX -= mLastMousePos.y() - e->pos().y();

    mLastMousePos = e->pos();

    update();
}*/

void BlueMarble::timerEvent(QTimerEvent *)
{
    mRotZ += 0.04;
    mSunAngle += 0.02;
    update();
}

void BlueMarble::initializeGL()
{
    kDebug() ;
    initGL();
    if (!GLShader::fragmentShaderSupported()) {
        setFailedToLaunch(true, i18n("OpenGL Shaders not supported"));
        return;
    }

    generateModel();
    mDayTexture = new GLTexture(KGlobal::dirs()->findResource("data", "plasma-bluemarble/earth.png"));
    mNightTexture = new GLTexture(KGlobal::dirs()->findResource("data", "plasma-bluemarble/earth-night.png"));

    kDebug() << "Loading shader";
    mShader = new GLShader(KGlobal::dirs()->findResource("data", "plasma-bluemarble/earth.vert"),
                           KGlobal::dirs()->findResource("data", "plasma-bluemarble/earth.frag"));
    kDebug() << "Initing shader";
    mShader->bind();
    mShader->setUniform("dayTexture", 0);
    mShader->setUniform("nightTexture", 1);
    mShader->unbind();

    kDebug() << "Loading atmosphere shader";
    mAtmosphereShader = new GLShader(KGlobal::dirs()->findResource("data", "plasma-bluemarble/atmosphere.vert"),
                                     KGlobal::dirs()->findResource("data", "plasma-bluemarble/atmosphere.frag"));

    startTimer(500);
}

void BlueMarble::paintGLInterface(QPainter *, const QStyleOptionGraphicsItem *)
{
    checkGLError("paint 1");
    glViewport(0, 0, int(width()), int(height()));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, width() / height(), 1, 32);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, -10, 0,  0, 0, 0,  0, 0, 1);
    glRotatef(mRotX, 1, 0, 0);
    glRotatef(mRotZ, 0, 0, 1);


    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const int earthDetailLevel = 60;
    const float earthRadius = 3.0f;

    // Render atmosphere
    mAtmosphereShader->bind();
    mAtmosphereShader->setUniform("sunDir", 0.707 * cos(mSunAngle), 0.707 * sin(mSunAngle), 0.5);

    glCullFace(GL_FRONT);
    gluSphere(mSphere, earthRadius * 3, earthDetailLevel, earthDetailLevel);

    mAtmosphereShader->unbind();
    glDisable(GL_BLEND);


    // Render Earth
    checkGLError("paint 4");
    mShader->bind();
    mShader->setUniform("sunDir", 0.707 * cos(mSunAngle), 0.707 * sin(mSunAngle), 0.5);
    glActiveTexture(GL_TEXTURE0);
    mDayTexture->bind();
    glActiveTexture(GL_TEXTURE1);
    mNightTexture->bind();

    glCullFace(GL_BACK);
    checkGLError("paint 5");
   gluSphere(mSphere, earthRadius, earthDetailLevel, earthDetailLevel);

    mShader->unbind();
}

#include "bluemarble.moc"
