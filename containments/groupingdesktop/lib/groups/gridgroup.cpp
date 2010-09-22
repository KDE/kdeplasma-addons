/*
 *   Copyright 2009-2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#include "gridgroup.h"

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QtGui/QGraphicsSceneHoverEvent>
#include <QtGui/QGraphicsLinearLayout>

#include <KIcon>

#include <Plasma/Theme>
#include <Plasma/PaintUtils>
#include <Plasma/ToolButton>
#include <Plasma/Animator>
#include <Plasma/Animation>

#include "spacer.h"

REGISTER_GROUP(GridGroup)

GridGroup::GridGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
         : AbstractGroup(parent, wFlags),
           m_showGrid(false),
           m_gridManager(new Spacer(this)),
           m_newRowCol(new Plasma::ToolButton(m_gridManager)),
           m_delRowCol(new Plasma::ToolButton(m_gridManager)),
           m_managerAnim(0)
{
    resize(200,200);
    setGroupType(AbstractGroup::ConstrainedGroup);
    setUseSimplerBackgroundForChildren(true);

    m_gridManager->hide();

    m_gridManagerLayout = new QGraphicsLinearLayout();
    m_gridManager->setLayout(m_gridManagerLayout);
    m_gridManager->setZValue(100000000);
    m_gridManagerLayout->addItem(m_newRowCol);
    m_gridManagerLayout->addStretch();
    m_gridManagerLayout->addItem(m_delRowCol);

    m_newRowCol->setIcon(KIcon("list-add"));
    m_newRowCol->setMinimumSize(QSizeF());
    m_delRowCol->setIcon(KIcon("list-remove"));
    m_delRowCol->setMinimumSize(QSizeF());

    m_managerAnim = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    m_managerAnim->setTargetWidget(m_gridManager);
    m_managerAnim->setProperty("startOpacity", 0);
    m_managerAnim->setProperty("targetOpacity", 1);
    connect(m_managerAnim, SIGNAL(finished()), this, SLOT(animationFinished()));

    connect(m_newRowCol, SIGNAL(clicked()), this, SLOT(addNewRowOrColumn()));
    connect(m_delRowCol, SIGNAL(clicked()), this, SLOT(removeRowOrColumn()));

    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(subGroupRemoved(AbstractGroup*)));
}

GridGroup::~GridGroup()
{

}

void GridGroup::init()
{
    KConfigGroup cg = config();
    m_rowsNumber = cg.readEntry("RowsNumber", 0);
    m_colsNumber = cg.readEntry("ColsNumber", 0);

    if (m_rowsNumber == 0) {
        m_rowsNumber = contentsRect().height() / 50;

        cg.writeEntry("RowsNumber", m_rowsNumber);
    }
    if (m_colsNumber == 0) {
        m_colsNumber = contentsRect().width() / 50;

        cg.writeEntry("ColsNumber", m_colsNumber);
    }
}

bool GridGroup::showDropZone(const QPointF &pos)
{
    bool showGrid = !pos.isNull();
    if (showGrid != m_showGrid) {
        m_showGrid = showGrid;
        update();
    }

    return m_showGrid;
}

QString GridGroup::pluginName() const
{
    return QString("grid");
}

void GridGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    QRectF rect(group.readEntry("Geometry", QRectF()));
    QRectF cRect(contentsRect());
    const qreal width = cRect.width() / m_colsNumber;
    const qreal height = cRect.height() / m_rowsNumber;
    child->setData(0, rect);
    child->setPos(rect.x() * width + cRect.x(), rect.y() * height + cRect.y());
    child->resize(rect.width() * width, rect.height() * height);

    child->installEventFilter(this);
}

void GridGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    group.writeEntry("Geometry", child->data(0).toRectF());
}

void GridGroup::addNewRowOrColumn()
{
    KConfigGroup cg = config();
    if (m_gridManagerLayout->orientation() == Qt::Vertical) {
        ++m_colsNumber;
        cg.writeEntry("ColsNumber", m_colsNumber);
    } else {
        ++m_rowsNumber;
        cg.writeEntry("RowsNumber", m_rowsNumber);
    }
    emit configNeedsSaving();

    updateGeometries();
}

void GridGroup::removeRowOrColumn()
{
    KConfigGroup cg = config();
    if (m_gridManagerLayout->orientation() == Qt::Vertical) {
        //check we don't remove columns with children in it
        foreach (QGraphicsWidget *child, children()) {
            QRectF rect = child->data(0).toRectF();
            if (rect.contains(m_colsNumber, rect.y())) {
                return;
            }
        }
        m_colsNumber = (m_colsNumber > 1 ? m_colsNumber - 1 : 1);
        cg.writeEntry("ColsNumber", m_colsNumber);
    } else {
        //check we don't remove rows with children in it
        foreach (QGraphicsWidget *child, children()) {
            QRectF rect = child->data(0).toRectF();
            if (rect.contains(rect.x(), m_rowsNumber)) {
                return;
            }
        }
        m_rowsNumber = (m_rowsNumber > 1 ? m_rowsNumber - 1 : 1);
        cg.writeEntry("RowsNumber", m_rowsNumber);
    }
    emit configNeedsSaving();

    updateGeometries();
}

void GridGroup::appletRemoved(Plasma::Applet *)
{
    if (m_cornerHandle && m_cornerHandle.data()->isVisible()) {
        m_cornerHandle.data()->hide();
        m_cornerHandle.data()->setParentItem(this);
    }
}

void GridGroup::subGroupRemoved(AbstractGroup *)
{
    appletRemoved(0);
}
void GridGroup::animationFinished()
{
    if (m_managerAnim->direction() == QAbstractAnimation::Backward) {
        m_gridManager->hide();
    }
}

void GridGroup::updateChild(QGraphicsWidget *child)
{
    QPointF pos(child->pos());
    QRectF rect(contentsRect());
    QRectF geom(child->geometry());

    const qreal width = rect.width() / m_colsNumber;
    const qreal height = rect.height() / m_rowsNumber;
    int i = qRound(pos.x() / width);
    int j = qRound(pos.y() / height);
    int cols = qRound(geom.width() / width);
    int rows = qRound(geom.height() / height);
    rows = (rows > 0 ? rows : 1);
    cols = (cols > 0 ? cols : 1);

    i = (i < 0 ? 0 : i);
    QGraphicsWidget *w = childAt(i, j);
    //find the nearest hole in which the child can be put
    if (w && w != child) {
        bool done = false;
        int ray = 1;
        bool firstPhase = true;
        while (!done && (ray < m_colsNumber || ray < m_rowsNumber)) {
            for (int k = ray; k > -ray; --k) {
                int col = k + i;
                int row = (k >= 0 ? ray - k : ray + k) + j;
                if (firstPhase) {
                    qreal dist = sqrt((col - i) * (col - i) + (row - j) * (row - j));
                    if (dist == ray) {
                        continue;
                    }
                }
                if (col >= 0 && row >= 0 && col < m_colsNumber && row < m_rowsNumber) {
                    QGraphicsWidget *c = childAt(col, row);
                    if (!c || c == child) {
                        i = col;
                        j = row;
                        done = true;
                        break;
                    }
                }
            }
            if (!done) {
                for (int k = -ray; k < ray; ++k) {
                    int col = k + i;
                    int row = (k >= 0 ? -ray + k : -ray - k) + j;
                    if (firstPhase) {
                        qreal dist = sqrt((col - i) * (col - i) + (row - j) * (row - j));
                        if (dist == ray) {
                            continue;
                        }
                    }
                    if (col >= 0 && row >= 0 && col < m_colsNumber && row < m_rowsNumber) {
                        QGraphicsWidget *c = childAt(col, row);
                        if (!c || c == child) {
                            i = col;
                            j = row;
                            done = true;
                            break;
                        }
                    }
                }
            }
            if (!firstPhase) {
                ++ray;
            }
            firstPhase = !firstPhase;
        }

        if (!done) { //couldn't find an empty cell
            w->hide(); //FIXME: do something, maybe should destroy it?
        }
    }

    int colsNumber = m_colsNumber;
    for (int col = i + 1; col < m_colsNumber; ++col) {
        bool occupied = false;
        for (int row = j; row < j + rows; ++row) {
            QGraphicsWidget *c = childAt(col, row);
            if (c && c != child) {
                occupied = true;
                break;
            }
        }
        if (occupied) {
            colsNumber = col;
            break;
        }
    }
    if (i + cols > colsNumber) {
        cols = colsNumber - i;
    }

    int rowsNumber = m_rowsNumber;
    for (int row = j + 1; row < m_rowsNumber; ++row) {
        bool occupied = false;
        for (int col = i; col < i + cols; ++col) {
            QGraphicsWidget *c = childAt(col, row);
            if (c && c != child) {
                occupied = true;
                break;
            }
        }
        if (occupied) {
            rowsNumber = row;
            break;
        }
    }
    if (j + rows > rowsNumber) {
        rows = rowsNumber - j;
    }

    child->setData(0, QRectF(i, j, cols, rows));
    child->setGeometry(QRectF(i * width + rect.x(), j * height + rect.y(),
                                          width * cols, height * rows));
}

QGraphicsWidget *GridGroup::childAt(int column, int row)
{
    foreach (QGraphicsWidget *c, children()) {
        QRectF rect = c->data(0).toRectF();
        if (rect.contains(column, row) && column < rect.right() && row < rect.bottom()) {
            return c;
        }
    }

    return 0;
}

void GridGroup::updateGeometries()
{
    QRectF rect(contentsRect());
    const qreal width = rect.width() / m_colsNumber;
    const qreal height = rect.height() / m_rowsNumber;
    foreach (QGraphicsWidget *child, children()) {
        QRectF r(child->data(0).toRectF());
        child->setPos(r.x() * width + rect.x(), r.y() * height + rect.y());
        child->resize(r.width() * width, r.height() * height);
    }
}

void GridGroup::layoutChild(QGraphicsWidget *child, const QPointF &)
{
    updateChild(child);

    child->installEventFilter(this);

    m_showGrid = false;
    update();
}

void GridGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    AbstractGroup::resizeEvent(event);

    if (m_managerAnim && m_gridManager->isVisible()) {
        m_managerAnim->setDirection(QAbstractAnimation::Backward);
        m_managerAnim->start();
    }
    updateGeometries();
}

void GridGroup::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF pos(event->pos());
    if ((m_gridManager->isVisible() && m_gridManager->geometry().contains(pos)) || immutability() != Plasma::Mutable) {
        return;
    }
    QRectF bRect(boundingRect());
    QRectF cRect(contentsRect());
    const qreal size = 30;
    bool vertical;
    QRectF geom;
    if (QRectF(0, 0, 2 * size, bRect.height()).contains(pos)) {
        vertical = true;
        geom = QRect(cRect.x(), cRect.y(), size, cRect.height());
    } else if (QRectF(bRect.width() - 2 * size, 0, 2 * size, bRect.height()).contains(pos)) {
        vertical = true;
        geom = QRectF(cRect.right() - size, cRect.y(), size, cRect.height());
    } else if (QRectF(0, 0, bRect.width(), size).contains(pos)) {
        vertical = false;
        geom = QRectF(cRect.x(), cRect.y(), cRect.width(), size);
    } else if (QRectF(0, bRect.height() - 2 * size, bRect.width(), 2 * size).contains(pos)) {
        vertical = false;
        geom = QRectF(cRect.x(), cRect.bottom() - size, cRect.width(), size);
    } else {
        m_managerAnim->setDirection(QAbstractAnimation::Backward);
        m_managerAnim->start();
        return;
    }

    if (vertical) {
        m_newRowCol->setToolTip(i18n("Add a new column"));
        m_delRowCol->setToolTip(i18n("Remove a column"));
        m_gridManagerLayout->setOrientation(Qt::Vertical);
    } else {
        m_newRowCol->setToolTip(i18n("Add a new row"));
        m_delRowCol->setToolTip(i18n("Remove a row"));
        m_gridManagerLayout->setOrientation(Qt::Horizontal);
    }
    m_gridManager->setGeometry(geom);
    if (!m_gridManager->isVisible()) {
        m_gridManager->show();
        m_managerAnim->setDirection(QAbstractAnimation::Forward);
        m_managerAnim->start();
    }
}

void GridGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_managerAnim->setDirection(QAbstractAnimation::Backward);
    m_managerAnim->start();
    if (m_cornerHandle) {
        m_cornerHandle.data()->hide();
    }
}

bool GridGroup::eventFilter(QObject *obj, QEvent *event)
{
    if (immutability() != Plasma::Mutable) {
        return AbstractGroup::eventFilter(obj, event);
    }

    switch (event->type()) {
        case QEvent::GraphicsSceneHoverEnter:
        case QEvent::GraphicsSceneHoverMove: {
            QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(obj);
            QGraphicsSceneHoverEvent *e = static_cast<QGraphicsSceneHoverEvent *>(event);
            if (widget && children().contains(widget)) {
                if (!m_cornerHandle) {
                    m_cornerHandle = new Spacer(this);
                    m_cornerHandle.data()->hide();
                    m_cornerHandle.data()->resize(20, 20);
                    m_cornerHandle.data()->installEventFilter(this);
                }
                m_cornerHandle.data()->setParentItem(widget);

                QPointF pos(e->pos());
                QRectF rect(widget->boundingRect());
                const qreal width = rect.width() / 2.;
                const qreal height = rect.height() / 2.;
                QRectF topLeft(0, 0, width, height);
                QRectF topRight(width, 0, width, height);
                QRectF bottomRight(width, height, width, height);
                if (topLeft.contains(pos)) {
                    m_cornerHandle.data()->setGeometry(0, 0, 20, 20);
                    m_handleCorner = Qt::TopLeftCorner;
                } else if (topRight.contains(pos)) {
                    m_cornerHandle.data()->setGeometry(rect.width() - 20, 0, 20, 20);
                    m_handleCorner = Qt::TopRightCorner;
                } else if (bottomRight.contains(pos)) {
                    m_cornerHandle.data()->setGeometry(rect.width() - 20, rect.height() - 20, 20, 20);
                    m_handleCorner = Qt::BottomRightCorner;
                } else {
                    m_cornerHandle.data()->setGeometry(0, rect.height() - 20, 20, 20);
                    m_handleCorner = Qt::BottomLeftCorner;
                }

                m_cornerHandle.data()->show();
            }
        }
        break;

        case QEvent::GraphicsSceneHoverLeave:
            if (m_cornerHandle) {
                m_cornerHandle.data()->setParentItem(this);
                m_cornerHandle.data()->hide();
            }
        break;

        case QEvent::GraphicsSceneMousePress:
            if (obj == m_cornerHandle.data()) {
                QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent *>(event);
                if (e->button() == Qt::LeftButton) {
                    event->accept();
                    m_showGrid = true;
                    m_resizeStartPos = mapFromScene(e->scenePos());
                    m_resizeStartGeom = m_cornerHandle.data()->parentWidget()->geometry();
                    update();
                    return true;
                }
            }
        break;

        case QEvent::GraphicsSceneMouseMove:
            if (obj == m_cornerHandle.data()) {
                QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent *>(event);
                QGraphicsWidget *child = m_cornerHandle.data()->parentWidget();
                QRectF geom(m_resizeStartGeom);
                QPointF pos(m_cornerHandle.data()->pos());
                QSizeF size(child->minimumSize());
                const QPointF delta = mapFromScene(e->scenePos()) - m_resizeStartPos;
                switch (m_handleCorner) {
                    case Qt::TopLeftCorner:
                        geom.setTopLeft(geom.topLeft() + delta);
                        break;
                    case Qt::TopRightCorner:
                        geom.setTopRight(geom.topRight() + delta);
                        pos = QPointF(geom.width() - 20, 0);
                        break;
                    case Qt::BottomRightCorner:
                        geom.setBottomRight(geom.bottomRight() + delta);
                        pos = QPointF(geom.width() - 20, geom.height() - 20);
                        break;
                    case Qt::BottomLeftCorner:
                        geom.setBottomLeft(geom.bottomLeft() + delta);
                        pos = QPointF(0, geom.height() - 20);
                        break;
                }
                if (geom.width() >= size.width() && geom.height() >= size.height()) {
                    child->setGeometry(geom);
                    m_cornerHandle.data()->setPos(pos);
                }
                event->accept();
            }
        break;

        case QEvent::GraphicsSceneMouseRelease:
            if (obj == m_cornerHandle.data()) {
                QGraphicsWidget *child = m_cornerHandle.data()->parentWidget();
                QRectF geom(child->geometry());

                QPointF pos(child->pos());
                QRectF rect(contentsRect());

                const qreal width = rect.width() / m_colsNumber;
                const qreal height = rect.height() / m_rowsNumber;
                int i = qRound(pos.x() / width);
                int j = qRound(pos.y() / height);
                int cols = qRound(geom.width() / width);
                int rows = qRound(geom.height() / height);
                rows = (rows > 0 ? rows : 1);
                cols = (cols > 0 ? cols : 1);
                i = (i > 0 ? i : 0);
                j = (j > 0 ? j : 0);

                QRectF newRect = QRectF(i, j, cols, rows);

                bool intersects = false;
                foreach (QGraphicsWidget *c, children()) {
                    if (c != child) {
                        QRectF r(c->data(0).toRectF());
                        if (r.intersects(newRect)) {
                            intersects = true;
                            break;
                        }
                    }
                }

                if (intersects) {
                    newRect = child->data(0).toRectF();
                } else {
                    child->setData(0, newRect);
                }
                child->setGeometry(QRectF(newRect.x() * width + rect.x(), newRect.y() * height + rect.y(),
                                          width * newRect.width(), height * newRect.height()));

                Plasma::Animation *anim = Plasma::Animator::create(Plasma::Animator::GeometryAnimation);
                if (anim) {
                    anim->setTargetWidget(child);
                    anim->setProperty("startGeometry", geom);
                    anim->setProperty("targetGeometry", child->geometry());
                    anim->start(QAbstractAnimation::DeleteWhenStopped);
                }

                m_showGrid = false;
                update();

                saveChildren();
            }
            break;

        default:
            break;
    }

    return AbstractGroup::eventFilter(obj, event);
}

void GridGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractGroup::paint(painter, option, widget);

    if (m_showGrid) {
        painter->setRenderHint(QPainter::Antialiasing);
        QRectF rect(contentsRect());
        const qreal width = rect.width() / m_colsNumber;
        const qreal height = rect.height() / m_rowsNumber;
        for (int i = 0; i < m_colsNumber; ++i) {
            for (int j = 0; j < m_rowsNumber; ++j) {
                QRectF r(i * width + rect.x(), j * height + rect.y(), width, height);
                QPainterPath p = Plasma::PaintUtils::roundedRectangle(r.adjusted(2, 2, -2, -2), 4);
                QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
                c.setAlphaF(0.3);
                painter->fillPath(p, c);
            }
        }
    }
}

GroupInfo GridGroup::groupInfo()
{
    GroupInfo gi("grid", i18n("Grid Group"));
    gi.setIcon("view-grid");

    return gi;
}

#include "gridgroup.moc"
