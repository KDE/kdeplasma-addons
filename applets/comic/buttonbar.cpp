/***************************************************************************
 *   Copyright (C) 2008-2012 Matthias Fuchs <mat69@gmx.net>                *
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

#include "buttonbar.h"

#include <KIconLoader>
#include <KLocale>
#include <KPushButton>

#include <Plasma/Frame>
#include <Plasma/PushButton>

#include <QtGui/QGraphicsLinearLayout>
#include <QtCore/QPropertyAnimation>

ButtonBar::ButtonBar(QGraphicsWidget *parent)
  : QObject(parent)
{
    mFrame = new Plasma::Frame(parent);
    mFrame->setZValue(10);
    QGraphicsLinearLayout *l = new QGraphicsLinearLayout();
    mPrev = new Plasma::PushButton(mFrame);
    mPrev->nativeWidget()->setIcon(KIcon("arrow-left"));
    mPrev->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mPrev->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    connect(mPrev, SIGNAL(clicked()), this , SIGNAL(prevClicked()));
    l->addItem(mPrev);

    mZoom = new Plasma::PushButton(mFrame);
    mZoom->nativeWidget()->setIcon(KIcon("zoom-original"));
    mZoom->nativeWidget()->setToolTip(i18n("Show at actual size in a different view.  Alternatively, click with the middle mouse button on the comic."));
    mZoom->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mZoom->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    connect(mZoom, SIGNAL(clicked()), this, SIGNAL(zoomClicked()));
    l->addItem(mZoom);

    mNext = new Plasma::PushButton(mFrame);
    mNext->nativeWidget()->setIcon(KIcon("arrow-right"));
    mNext->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mNext->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    connect(mNext, SIGNAL(clicked()), this , SIGNAL(nextClicked()));
    l->addItem(mNext);
    mFrame->setLayout(l);
    mFrame->setFrameShadow(Plasma::Frame::Raised);
    // To get correct frame size in constraintsEvent
    l->activate();
    mFrame->setOpacity(0.0);

    mFrameAnim = new QPropertyAnimation(mFrame, "opacity", mFrame);
    mFrameAnim->setDuration(100);
    mFrameAnim->setStartValue(0.0);
    mFrameAnim->setEndValue(1.0);
}

ButtonBar::~ButtonBar()
{
}

QSizeF ButtonBar::size() const
{
    return mFrame->size();
}

void ButtonBar::setPos(const QPointF &pos)
{
    mFrame->setPos(pos);
}

void ButtonBar::show()
{
    mFrameAnim->setDirection(QAbstractAnimation::Forward);
    mFrameAnim->start();
}

void ButtonBar::hide()
{
    mFrameAnim->setDirection(QAbstractAnimation::Backward);
    mFrameAnim->start();
}


void ButtonBar::setNextEnabled(bool enabled)
{
    mNext->setEnabled(enabled);
}

void ButtonBar::setPrevEnabled(bool enabled)
{
    mPrev->setEnabled(enabled);
}

#include "buttonbar.moc"
