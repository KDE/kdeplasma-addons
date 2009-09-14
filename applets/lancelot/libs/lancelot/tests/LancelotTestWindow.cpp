/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotTestWindow.h"

#include <KIcon>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <KLineEdit>
#include <Plasma/LineEdit>

#include "../widgets/Widget.h"
#include "../widgets/ExtenderButton.h"
#include "../widgets/Panel.h"
#include "../widgets/ResizeBordersPanel.h"
#include "../widgets/ScrollBar.h"
#include "../widgets/ScrollPane.h"

#include "../layouts/FullBorderLayout.h"
#include "../layouts/FlipLayout.h"
#include "../layouts/NodeLayout.h"
#include "../layouts/CardLayout.h"

using namespace Lancelot;

class ScrollableWidget: public Lancelot::Widget, public Lancelot::Scrollable {
public:
    ScrollableWidget(QGraphicsItem * parent = 0)
        : Lancelot::Widget(parent)
    {
        //
        setGeometry(QRectF(QPointF(), fullSize()));
    }

    L_Override void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0)
    {
        painter->fillRect(
                QRectF(QPointF(), size()),
                QBrush(QColor(250, 100, 100))
                );
        painter->fillRect(
                QRectF(QPointF(5, 5), size() - QSizeF(10, 10)),
                QBrush(QColor(100, 100, 100))
                );

        painter->drawText(
                QPointF(30, 50),
                "Rect: " + QString::number(view.left()) +
                ", " + QString::number(view.top()) +
                ": " + QString::number(view.width()) +
                " x " + QString::number(view.height()) +
                ""
                );
        painter->drawText(
                QPointF(30, 80) - geometry().topLeft(),
                "Rect: " + QString::number(view.left()) +
                ", " + QString::number(view.top()) +
                ": " + QString::number(view.width()) +
                " x " + QString::number(view.height()) +
                ""
                );
    }

    L_Override QSizeF fullSize() const
    {
        // return QSizeF(384, 354); // perfect fit - no scrolls
        // return QSizeF(392, 362); // perfect fit - no scrolls
        // return QSizeF(300, 600); // vertical
        // return QSizeF(600, 300); // horizontal
        return QSizeF(600, 600); // both
    }

    L_Override void viewportChanged(QRectF viewport)
    {
        view = viewport;
        setGeometry(QRectF(- viewport.topLeft(), fullSize()));
        update();
    }

    L_Override qreal scrollUnit(Qt::Orientation direction)
    {
        Q_UNUSED(direction);
        return 10;
    }

private:
    QRectF view;
};

LancelotTestWindow::LancelotTestWindow()
    : QGraphicsView()
{

    // Initialization
    setFocusPolicy(Qt::WheelFocus);
    m_corona = new Plasma::Corona(this);
    setScene(m_corona);

    // Test area - begin ####################################
    QGraphicsWidget * m_root = new QGraphicsWidget();

    m_root->setGeometry(QRectF(0, 0, 400, 400));
    m_corona->addItem(m_root);

    BasicWidget * wgt;

    wgt = new BasicWidget(m_root);
    wgt->setGroupByName("RootPanel");
    wgt->setTitle("Ivan");
    wgt->setIcon(KIcon("lancelot"));
    wgt->setDescription("Cukic je genije");
    wgt->setGeometry(QRectF(QPointF(0, 0), wgt->preferredSize()));

    wgt = new BasicWidget(m_root);
    wgt->setGroupByName("RootPanel");
    wgt->setTitle("Ivan");
    wgt->setIcon(KIcon("lancelot"));
    wgt->setDescription("Pie iesu domine, dona e ist requiem");
    wgt->setGeometry(QRectF(QPointF(0, 100), wgt->preferredSize()));

    wgt = new BasicWidget(m_root);
    wgt->setGroupByName("RootPanel");
    wgt->setTitle("Ivan Restrer oret");
    wgt->setIcon(KIcon("lancelot"));
    wgt->setDescription("Cukic je genije");
    wgt->setGeometry(QRectF(QPointF(0, 200), wgt->preferredSize()));

    wgt = new BasicWidget(m_root);
    wgt->setInnerOrientation(Qt::Vertical);
    wgt->setGroupByName("RootPanel");
    wgt->setTitle("Ivan");
    wgt->setIcon(KIcon("lancelot"));
    wgt->setDescription("Cukic je genije");
    wgt->setGeometry(QRectF(QPointF(200, 0), wgt->preferredSize()));

    wgt = new BasicWidget(m_root);
    wgt->setInnerOrientation(Qt::Vertical);
    wgt->setGroupByName("RootPanel");
    wgt->setTitle("Ivan");
    wgt->setIcon(KIcon("lancelot"));
    wgt->setDescription("Pie iesu domine, dona e ist requiem");
    wgt->setGeometry(QRectF(QPointF(200, 100), wgt->preferredSize()));

    wgt = new BasicWidget(m_root);
    wgt->setInnerOrientation(Qt::Vertical);
    wgt->setGroupByName("RootPanel");
    wgt->setTitle("Ivan Restrer oret");
    wgt->setIcon(KIcon("lancelot"));
    wgt->setDescription("Cukic je genije");
    wgt->setGeometry(QRectF(QPointF(200, 200), wgt->preferredSize()));
    // Test area - end   ####################################

    // Starting...
}

LancelotTestWindow::~LancelotTestWindow()
{
}

#include "LancelotTestWindow.moc"

