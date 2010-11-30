/*
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "gridmanager.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QPainter>

#include <KIcon>
#include <KLocale>

#include <Plasma/Theme>
#include <Plasma/PaintUtils>
#include <Plasma/Animator>
#include <Plasma/ToolButton>
#include <Plasma/Animation>

static const int SIZE = 30;

GridManager::GridManager(QGraphicsItem *parent)
           : QGraphicsWidget(parent),
             m_layout(new QGraphicsLinearLayout(this)),
             m_location(Plasma::Floating), //it isn't really floating, just not on any edge
             m_newRowCol(new Plasma::ToolButton(this)),
             m_newRowCol2(new Plasma::ToolButton(this)),
             m_delRowCol(new Plasma::ToolButton(this)),
             m_delRowCol2(new Plasma::ToolButton(this))
{
    setLayout(m_layout);
    setZValue(100000000);
    m_layout->addItem(m_newRowCol);
    m_layout->addItem(m_delRowCol);
    m_layout->addStretch();
    m_layout->addItem(m_delRowCol2);
    m_layout->addItem(m_newRowCol2);

    m_newRowCol->setIcon(KIcon("list-add"));
    m_newRowCol->setMinimumSize(QSizeF(0, 0));
    m_delRowCol->setIcon(KIcon("list-remove"));
    m_delRowCol->setMinimumSize(QSizeF(0, 0));
    m_newRowCol2->setIcon(KIcon("list-add"));
    m_newRowCol2->setMinimumSize(QSizeF(0, 0));
    m_delRowCol2->setIcon(KIcon("list-remove"));
    m_delRowCol2->setMinimumSize(QSizeF(0, 0));

    m_newRowCol->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    m_newRowCol2->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    m_delRowCol->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    m_delRowCol2->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

    //FIXME: QGraphicsLayout is bugged and it won't lay out the items until a resize is called.
    resize(50, 50);

    m_fadeAnim = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    m_fadeAnim->setTargetWidget(this);
    m_fadeAnim->setProperty("startOpacity", 0);
    m_fadeAnim->setProperty("targetOpacity", 1);
    connect(m_fadeAnim, SIGNAL(finished()), this, SLOT(animationFinished()));

    connect(m_newRowCol, SIGNAL(clicked()), this, SIGNAL(newClicked()));
    connect(m_delRowCol, SIGNAL(clicked()), this, SIGNAL(deleteClicked()));
    connect(m_newRowCol2, SIGNAL(clicked()), this, SIGNAL(newClicked()));
    connect(m_delRowCol2, SIGNAL(clicked()), this, SIGNAL(deleteClicked()));
}

GridManager::~GridManager()
{

}

void GridManager::checkLocation(const QPointF &pos)
{
    if (isVisible() && geometry().contains(pos)) {
        return;
    }

    QRectF cRect(parentWidget()->contentsRect());

    const QRectF leftEdge(cRect.x(), cRect.y(), SIZE, cRect.height());
    const QRectF rightEdge(cRect.right() - SIZE, cRect.y(), SIZE, cRect.height());
    const QRectF topEdge(cRect.x(), cRect.y(), cRect.width(), SIZE);
    const QRectF bottomEdge(cRect.x(), cRect.bottom() - SIZE, cRect.width(), SIZE);
    QRectF geom;
    if (leftEdge.contains(pos)) {
        setLocation(Plasma::LeftEdge);
    } else if (rightEdge.contains(pos)) {
        setLocation(Plasma::RightEdge);
    } else if (topEdge.contains(pos)) {
        setLocation(Plasma::TopEdge);
    } else if (bottomEdge.contains(pos)) {
        setLocation(Plasma::BottomEdge);
    } else {
        hideAnimated();
    }
}

void GridManager::setLocation(Plasma::Location location)
{
    if (location == m_location) {
        return;
    }

    m_location = location;

    if (isVisible()) {
        m_replace = true;
        m_fadeAnim->setDirection(QAbstractAnimation::Backward);
        m_fadeAnim->start();
    } else {
        place();
    }
}

Plasma::Location GridManager::location() const
{
    return m_location;
}

void GridManager::place()
{
    const QRectF cRect(parentWidget()->contentsRect());
    QRectF geom;
    switch (m_location) {
        case Plasma::LeftEdge:
            geom = QRectF(cRect.x(), cRect.y(), SIZE, cRect.height());
        break;

        case Plasma::RightEdge:
            geom = QRectF(cRect.right() - SIZE, cRect.y(), SIZE, cRect.height());
        break;

        case Plasma::TopEdge:
            geom = QRectF(cRect.x(), cRect.y(), cRect.width(), SIZE);
        break;

        case Plasma::BottomEdge:
            geom = QRectF(cRect.x(), cRect.bottom() - SIZE, cRect.width(), SIZE);
        break;

        default:
            return;
    }

    if (m_location == Plasma::RightEdge || m_location == Plasma::LeftEdge) {
        m_newRowCol->setToolTip(i18n("Add a new column"));
        m_delRowCol->setToolTip(i18n("Remove a column"));
        m_newRowCol2->setToolTip(i18n("Add a new column"));
        m_delRowCol2->setToolTip(i18n("Remove a column"));
        m_layout->setOrientation(Qt::Vertical);
    } else {
        m_newRowCol->setToolTip(i18n("Add a new row"));
        m_delRowCol->setToolTip(i18n("Remove a row"));
        m_newRowCol2->setToolTip(i18n("Add a new row"));
        m_delRowCol2->setToolTip(i18n("Remove a row"));
        m_layout->setOrientation(Qt::Horizontal);
    }

    setGeometry(geom);
    showAnimated();
}

void GridManager::showAnimated()
{
    show();

    m_fadeAnim->setDirection(QAbstractAnimation::Forward);
    m_fadeAnim->start();
}

void GridManager::hideAnimated()
{
    m_location = Plasma::Floating;
    m_replace = false;
    m_fadeAnim->setDirection(QAbstractAnimation::Backward);
    m_fadeAnim->start();
}

void GridManager::animationFinished()
{
    if (m_fadeAnim->direction() == QAbstractAnimation::Backward) {
        hide();

        if (m_replace) {
            place();
        }
    }
}

void GridManager::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect(), 4);
    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    c.setAlphaF(0.3);

    painter->fillPath(p, c);
}

#include "gridmanager.moc"
