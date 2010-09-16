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

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QtGui/QGraphicsSceneHoverEvent>
#include <QtGui/QGraphicsLinearLayout>

#include <Plasma/Theme>
#include <Plasma/PaintUtils>
#include <Plasma/PushButton>

#include "spacer.h"

REGISTER_GROUP(GridGroup)

GridGroup::GridGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
         : AbstractGroup(parent, wFlags),
           m_showGrid(false),
           m_gridManager(new Spacer(this)),
           m_newRowCol(new Plasma::PushButton(m_gridManager)),
           m_delRowCol(new Plasma::PushButton(m_gridManager))
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
    m_newRowCol->resize(20, 20);
    m_newRowCol->setMaximumSize(20, 20);
    m_delRowCol->setIcon(KIcon("list-remove"));
    m_delRowCol->resize(20, 20);
    m_delRowCol->setMaximumSize(20, 20);

    connect(m_newRowCol, SIGNAL(clicked()), this, SLOT(addNewRowOrColumn()));
    connect(m_delRowCol, SIGNAL(clicked()), this, SLOT(removeRowOrColumn()));
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
        m_rowsNumber = contentsRect().width() / 50;

        cg.writeEntry("RowsNumber", m_rowsNumber);
    }
    if (m_colsNumber == 0) {
        m_colsNumber = contentsRect().height() / 50;

        cg.writeEntry("ColsNumber", m_colsNumber);
    }
}

bool GridGroup::showDropZone(const QPointF &pos)
{
    m_showGrid = !pos.isNull();
    update();
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
        m_colsNumber = (m_colsNumber > 1 ? m_colsNumber - 1 : 1);
        cg.writeEntry("ColsNumber", m_colsNumber);
    } else {
        m_rowsNumber = (m_rowsNumber > 1 ? m_rowsNumber - 1 : 1);
        cg.writeEntry("RowsNumber", m_rowsNumber);
    }
    emit configNeedsSaving();

    updateGeometries();
}

void GridGroup::updateChild(QGraphicsWidget *child)
{
    QPointF pos(child->pos());
    QRectF rect(contentsRect());
    if (pos.x() < rect.left()) {
        pos.setX(rect.left());
    }
    if (pos.y() < rect.top()) {
        pos.setY(rect.top());
    }
    QRectF geom(child->geometry());
    geom.setTopLeft(pos);
    const qreal width = rect.width() / m_colsNumber;
    const qreal height = rect.height() / m_rowsNumber;
    for (int i = 0; i < m_colsNumber; ++i) {
        for (int j = 0; j < m_rowsNumber; ++j) {
            QRectF r(i * width + rect.x() - width / 2., j * height + rect.y() - height / 2., width, height);
            if (r.contains(pos)) {
                int cols = qRound(geom.width() / width);
                int rows = qRound(geom.height() / height);
                rows = (rows > 0 ? rows : 1);
                cols = (cols > 0 ? cols : 1);
                if (i + cols > m_colsNumber) {
                    cols = m_colsNumber - i;
                }
                if (j + rows > m_rowsNumber) {
                    rows = m_rowsNumber - j;
                }
                child->setData(0, QRectF(i, j, cols, rows));
                child->setGeometry(QRectF(i * width + rect.x(), j * height + rect.y(),
                                          width * cols, height * rows));

                return;
            }
        }
    }
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

    updateGeometries();
}

void GridGroup::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF pos(event->pos());
    if (m_gridManager->geometry().contains(pos)) {
        return;
    }
    QRectF bRect(boundingRect());
    QRectF cRect(contentsRect());
    if (QRectF(0, 0, 40, bRect.height()).contains(pos)) {
        m_gridManagerLayout->setOrientation(Qt::Vertical);
        m_gridManager->setGeometry(cRect.x(), cRect.y(), 20, cRect.height());
        m_gridManager->show();
    } else if (QRectF(bRect.width() - 40, 0, 40, bRect.height()).contains(pos)) {
        m_gridManagerLayout->setOrientation(Qt::Vertical);
        m_gridManager->setGeometry(cRect.right() - 20, cRect.y(), 20, cRect.height());
        m_gridManager->show();
    } else if (QRectF(0, 0, bRect.width(), 20).contains(pos)) {
        m_gridManagerLayout->setOrientation(Qt::Horizontal);
        m_gridManager->setGeometry(cRect.x(), cRect.y(), cRect.width(), 20);
        m_gridManager->show();
    } else if (QRectF(0, bRect.height() - 40, bRect.width(), 40).contains(pos)) {
        m_gridManagerLayout->setOrientation(Qt::Horizontal);
        m_gridManager->setGeometry(cRect.x(), cRect.bottom() - 20, cRect.width(), 20);
        m_gridManager->show();
    } else {
        m_gridManager->hide();
    }
}

void GridGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_gridManager->hide();
    if (m_spacer) {
        m_spacer.data()->hide();
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
                if (!m_spacer) {
                    m_spacer = new Spacer(this);
                    m_spacer.data()->hide();
                    m_spacer.data()->resize(20, 20);
                    m_spacer.data()->installEventFilter(this);
                }
                m_spacer.data()->setParentItem(widget);

                QPointF pos(e->pos());
                QRectF rect(widget->boundingRect());
                const qreal width = rect.width() / 2.;
                const qreal height = rect.height() / 2.;
                QRectF topLeft(0, 0, width, height);
                QRectF topRight(width, 0, width, height);
                QRectF bottomRight(width, height, width, height);
                if (topLeft.contains(pos)) {
                    m_spacer.data()->setGeometry(0, 0, 20, 20);
                    m_spacerCorner = Qt::TopLeftCorner;
                } else if (topRight.contains(pos)) {
                    m_spacer.data()->setGeometry(rect.width() - 20, 0, 20, 20);
                    m_spacerCorner = Qt::TopRightCorner;
                } else if (bottomRight.contains(pos)) {
                    m_spacer.data()->setGeometry(rect.width() - 20, rect.height() - 20, 20, 20);
                    m_spacerCorner = Qt::BottomRightCorner;
                } else {
                    m_spacer.data()->setGeometry(0, rect.height() - 20, 20, 20);
                    m_spacerCorner = Qt::BottomLeftCorner;
                }

                m_spacer.data()->show();
            }
        }
        break;

        case QEvent::GraphicsSceneHoverLeave:
            if (m_spacer) {
                m_spacer.data()->setParentItem(this);
                m_spacer.data()->hide();
            }
        break;

        case QEvent::GraphicsSceneMousePress:
            if (obj == m_spacer.data() && static_cast<QGraphicsSceneMouseEvent *>(event)->button() == Qt::LeftButton) {
                event->accept();
                m_showGrid = true;
                update();
                return true;
            }
        break;

        case QEvent::GraphicsSceneMouseMove:
            if (obj == m_spacer.data()) {
                QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent *>(event);
                QGraphicsWidget *child = m_spacer.data()->parentWidget();
                QRectF geom(child->geometry());
                const QPointF delta = e->pos() - e->lastPos();
                switch (m_spacerCorner) {
                    case Qt::TopLeftCorner:
                        geom.setTopLeft(geom.topLeft() + delta);
                        break;
                    case Qt::TopRightCorner:
                        geom.setTopRight(geom.topRight() + delta);
                        m_spacer.data()->setPos(m_spacer.data()->pos() + QPointF(delta.x(), 0));
                        break;
                    case Qt::BottomRightCorner:
                        geom.setBottomRight(geom.bottomRight() + delta);
                        m_spacer.data()->setPos(m_spacer.data()->pos() + delta);
                        break;
                    case Qt::BottomLeftCorner:
                        geom.setBottomLeft(geom.bottomLeft() + delta);
                        m_spacer.data()->setPos(m_spacer.data()->pos() + QPointF(0, delta.y()));
                        break;
                }
                child->setGeometry(geom);
                event->accept();
            }
        break;

        case QEvent::GraphicsSceneMouseRelease:
            if (obj == m_spacer.data()) {
                QGraphicsWidget *child = m_spacer.data()->parentWidget();
                updateChild(child);
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
