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

#include <qmath.h>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QtGui/QGraphicsSceneHoverEvent>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QStyleOptionGraphicsItem>

#include <KIcon>

#include <Plasma/Theme>
#include <Plasma/PaintUtils>
#include <Plasma/ToolButton>
#include <Plasma/Animator>
#include <Plasma/Animation>

#include "spacer.h"
#include "gridmanager.h"

REGISTER_GROUP(GridGroup)

static const int CORNERHANDLE_WIDTH = 20;
static const int CORNERHANDLE_HEIGHT = 20;

GridGroup::GridGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
         : AbstractGroup(parent, wFlags),
           m_showGrid(false),
           m_gridManager(new GridManager(this))
{
    resize(200,200);
    setGroupType(AbstractGroup::ConstrainedGroup);

    m_gridManager->hide();
    connect(m_gridManager, SIGNAL(newClicked()), this, SLOT(addNewRowOrColumn()));
    connect(m_gridManager, SIGNAL(deleteClicked()), this, SLOT(removeRowOrColumn()));

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
    QRect rect(group.readEntry("Geometry", QRect()));
    QRectF cRect(contentsRect());
    const qreal width = cRect.width() / m_colsNumber;
    const qreal height = cRect.height() / m_rowsNumber;
    m_childrenRects.insert(child, rect);
    child->setPos(rect.x() * width + cRect.x(), rect.y() * height + cRect.y());
    child->resize(rect.width() * width, rect.height() * height);

    child->installEventFilter(this);
}

void GridGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    group.writeEntry("Geometry", m_childrenRects.value(child));
}

void GridGroup::addNewRowOrColumn()
{
    KConfigGroup cg = config();
    Plasma::Location location = m_gridManager->location();
    bool left = location == Plasma::LeftEdge;
    if (left || location == Plasma::RightEdge) {
        ++m_colsNumber;
        cg.writeEntry("ColsNumber", m_colsNumber);
        if (left) { //must move all the children one column right
            foreach (QGraphicsWidget *child, children()) {
                QRect r(m_childrenRects.value(child));
                r.translate(1, 0);
                m_childrenRects.insert(child, r);
            }
        }
    } else {
        ++m_rowsNumber;
        cg.writeEntry("RowsNumber", m_rowsNumber);
        if (location == Plasma::TopEdge) { //must move all the children one row down
            foreach (QGraphicsWidget *child, children()) {
                QRect r(m_childrenRects.value(child));
                r.translate(0, 1);
                m_childrenRects.insert(child, r);
            }
        }
    }
    saveChildren();
    emit configNeedsSaving();

    updateGeometries();
}

void GridGroup::removeRowOrColumn()
{
    KConfigGroup cg = config();
    Plasma::Location location = m_gridManager->location();
    bool left = location == Plasma::LeftEdge;
    if (left || location == Plasma::RightEdge) {
        //check we don't remove columns with children in it
        foreach (QGraphicsWidget *child, children()) {
            QRect rect = m_childrenRects.value(child);
            QPoint p;
            if (left) {
                p = QPoint(0, rect.y());
            } else {
                p = QPoint(m_colsNumber - 1, rect.y());
            }
            if (rect.contains(p)) {
                return;
            }
        }
        m_colsNumber = (m_colsNumber > 1 ? m_colsNumber - 1 : 1);
        cg.writeEntry("ColsNumber", m_colsNumber);
        if (left) { //must move all the children one column left
            foreach (QGraphicsWidget *child, children()) {
                QRect r(m_childrenRects.value(child));
                r.translate(-1, 0);
                m_childrenRects.insert(child, r);
            }
        }
    } else {
        //check we don't remove rows with children in it
        bool top = location == Plasma::TopEdge;
        foreach (QGraphicsWidget *child, children()) {
            QRectF rect = m_childrenRects.value(child);
            QPoint p;
            if (top) {
                p = QPoint(rect.x(), 0);
            } else {
                p = QPoint(rect.x(), m_rowsNumber);
            }
            if (rect.contains(p)) {
                return;
            }
        }
        m_rowsNumber = (m_rowsNumber > 1 ? m_rowsNumber - 1 : 1);
        cg.writeEntry("RowsNumber", m_rowsNumber);
        if (top) { //must move all the children one row up
            foreach (QGraphicsWidget *child, children()) {
                QRect r(m_childrenRects.value(child));
                r.translate(0, -1);
                m_childrenRects.insert(child, r);
            }
        }
    }
    saveChildren();
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
    QSizeF minSize(child->effectiveSizeHint(Qt::MinimumSize));
    while (cols * width < minSize.width()) {
        ++cols;
    }
    while (rows * height < minSize.height()) {
        ++rows;
    }
    rows = (rows > 0 ? rows : 1);
    cols = (cols > 0 ? cols : 1);

    i = (i < 0 ? 0 : i);
    j = (j < 0 ? 0 : j);
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
                    qreal dist = qSqrt((col - i) * (col - i) + (row - j) * (row - j));
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
                        qreal dist = qSqrt((col - i) * (col - i) + (row - j) * (row - j));
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

    m_childrenRects.insert(child, QRect(i, j, cols, rows));
    child->setGeometry(QRectF(i * width + rect.x(), j * height + rect.y(),
                                          width * cols, height * rows));
}

QGraphicsWidget *GridGroup::childAt(int column, int row)
{
    foreach (QGraphicsWidget *c, children()) {
        QRectF rect = m_childrenRects.value(c);
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
        QRectF r(m_childrenRects.value(child));
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

    m_gridManager->hideAnimated();
    updateGeometries();
}

void GridGroup::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (immutability() == Plasma::Mutable) {
        m_gridManager->checkLocation(event->pos());
    }
}

void GridGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_gridManager->hideAnimated();
    if (m_cornerHandle) {
        m_cornerHandle.data()->hide();
    }
}

void GridGroup::checkCorner(const QPointF &pos, QGraphicsWidget *widget)
{
    const QRectF rect(widget->contentsRect());
    QRectF topLeft(rect.left() - CORNERHANDLE_WIDTH / 2., rect.top() - CORNERHANDLE_HEIGHT / 2.,
                   CORNERHANDLE_WIDTH, CORNERHANDLE_HEIGHT);
    QRectF topRight(rect.right() - CORNERHANDLE_WIDTH / 2., rect.top() - CORNERHANDLE_HEIGHT / 2.,
                    CORNERHANDLE_WIDTH, CORNERHANDLE_HEIGHT);
    QRectF bottomRight(rect.right() - CORNERHANDLE_WIDTH / 2., rect.bottom() - CORNERHANDLE_HEIGHT / 2.,
                       CORNERHANDLE_WIDTH, CORNERHANDLE_HEIGHT);
    QRectF bottomLeft(rect.left() - CORNERHANDLE_WIDTH / 2., rect.bottom() - CORNERHANDLE_HEIGHT / 2.,
                      CORNERHANDLE_WIDTH, CORNERHANDLE_HEIGHT);

    //it would be better to check if pos is inside one of those rects above, but that way
    //the corner handle wouldn't show up when hovering the analog clock of other applets with
    //shape != boundingRect because they can be outside its shape.
    const QRectF bRect(widget->boundingRect());
    QRectF activeRect(bRect.topLeft(), bRect.size() / 2.);
    m_cornerHandle.data()->show();
    qreal shiftWidth = bRect.width() / 2.;
    qreal shiftHeight = bRect.height() / 2.;
    if (activeRect.contains(pos)) {
        m_cornerHandle.data()->setGeometry(topLeft);
        m_handleCorner = Qt::TopLeftCorner;
    } else if (activeRect.translated(shiftWidth, 0).contains(pos)) {
        m_cornerHandle.data()->setGeometry(topRight);
        m_handleCorner = Qt::TopRightCorner;
    } else if (activeRect.translated(shiftWidth, shiftHeight).contains(pos)) {
        m_cornerHandle.data()->setGeometry(bottomRight);
        m_handleCorner = Qt::BottomRightCorner;
    } else if (activeRect.translated(0, shiftHeight).contains(pos)) {
        m_cornerHandle.data()->setGeometry(bottomLeft);
        m_handleCorner = Qt::BottomLeftCorner;
    } else {
        m_cornerHandle.data()->setParentItem(this);
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
                    m_cornerHandle.data()->resize(CORNERHANDLE_WIDTH, CORNERHANDLE_HEIGHT);
                    m_cornerHandle.data()->installEventFilter(this);
                }
                m_cornerHandle.data()->setParentItem(widget);

                checkCorner(e->pos(), widget);
            }
        }
        break;

        case QEvent::GraphicsSceneHoverLeave:
            if (m_cornerHandle) {
                QGraphicsItem *child = m_cornerHandle.data()->parentItem();
                if (!child->boundingRect().contains(static_cast<QGraphicsSceneHoverEvent *>(event)->pos())) {
                    m_cornerHandle.data()->setParentItem(this);
                    m_cornerHandle.data()->hide();
                }
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
                QSizeF size(child->effectiveSizeHint(Qt::MinimumSize));
                const QPointF delta = mapFromScene(e->scenePos()) - m_resizeStartPos;
                switch (m_handleCorner) {
                    case Qt::TopLeftCorner: {
                            const qreal top = geom.top() + delta.y();
                            const qreal left = geom.left() + delta.x();
                            geom.setTop(geom.bottom() - top >= size.height() ? top : geom.bottom() - size.height());
                            geom.setLeft(geom.right() - left >= size.width() ? left : geom.right() - size.width());
                        }
                        break;
                    case Qt::TopRightCorner: {
                            const qreal top = geom.top() + delta.y();
                            const qreal right = geom.right() + delta.x();
                            geom.setTop(geom.bottom() - top >= size.height() ? top : geom.bottom() - size.height());
                            geom.setRight(right - geom.left() >= size.width() ? right : geom.left() + size.width());
                            qreal l, t, r, b;
                            child->getContentsMargins(&l, &t, &r, &b);
                            pos = QPointF(geom.width() - r - CORNERHANDLE_WIDTH / 2., t - CORNERHANDLE_HEIGHT / 2.);
                        }
                        break;
                    case Qt::BottomRightCorner: {
                            const qreal bottom = geom.bottom() + delta.y();
                            const qreal right = geom.right() + delta.x();
                            geom.setBottom(bottom - geom.top() >= size.height() ? bottom : geom.top() + size.height());
                            geom.setRight(right - geom.left() >= size.width() ? right : geom.left() + size.width());
                            qreal l, t, r, b;
                            child->getContentsMargins(&l, &t, &r, &b);
                            pos = QPointF(geom.width() - r - CORNERHANDLE_WIDTH / 2., geom.height() - b - CORNERHANDLE_HEIGHT / 2.);
                        }
                        break;
                    case Qt::BottomLeftCorner: {
                            const qreal bottom = geom.bottom() + delta.y();
                            const qreal left = geom.left() + delta.x();
                            geom.setBottom(bottom - geom.top() >= size.height() ? bottom : geom.top() + size.height());
                            geom.setLeft(geom.right() - left >= size.width() ? left : geom.right() - size.width());
                            qreal l, t, r, b;
                            child->getContentsMargins(&l, &t, &r, &b);
                            pos = QPointF(l - CORNERHANDLE_WIDTH / 2., geom.height() - b - CORNERHANDLE_HEIGHT / 2.);
                        }
                        break;
                }
                child->setGeometry(geom);
                m_cornerHandle.data()->setPos(pos);

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
                QSizeF minSize(child->effectiveSizeHint(Qt::MinimumSize));
                while (cols * width < minSize.width()) {
                    ++cols;
                }
                while (rows * height < minSize.height()) {
                    ++rows;
                }
                i = (i > 0 ? i : 0);
                j = (j > 0 ? j : 0);

                QRect newRect(i, j, cols, rows);

                bool intersects = false;
                foreach (QGraphicsWidget *c, children()) {
                    if (c != child) {
                        QRect r(m_childrenRects.value(c));
                        if (r.intersects(newRect)) {
                            intersects = true;
                            break;
                        }
                    }
                }

                if (intersects) {
                    newRect = m_childrenRects.value(child);
                } else {
                    m_childrenRects.insert(child, newRect);
                }
                child->setGeometry(QRectF(newRect.x() * width + rect.x(), newRect.y() * height + rect.y(),
                                          width * newRect.width(), height * newRect.height()));

                QPointF p(child->mapFromScene(static_cast<QGraphicsSceneMouseEvent *>(event)->scenePos()));
                checkCorner(p, child);

                Plasma::Animation *anim = Plasma::Animator::create(Plasma::Animator::GeometryAnimation);
                if (anim) {
                    child->removeEventFilter(this);
                    anim->setTargetWidget(child);
                    anim->setProperty("startGeometry", geom);
                    anim->setProperty("targetGeometry", child->geometry());
                    anim->start(QAbstractAnimation::DeleteWhenStopped);
                    connect(anim, SIGNAL(finished()), this, SLOT(resizeDone()));
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

void GridGroup::resizeDone()
{
    Plasma::Animation *anim = static_cast<Plasma::Animation *>(sender());
    anim->targetWidget()->installEventFilter(this);
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
                if (r.intersects(option->exposedRect)) {
                    QPainterPath p = Plasma::PaintUtils::roundedRectangle(r.adjusted(2, 2, -2, -2), 4);
                    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
                    c.setAlphaF(0.3);
                    painter->fillPath(p, c);
                }
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
