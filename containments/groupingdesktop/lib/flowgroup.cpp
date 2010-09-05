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

#include "flowgroup.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QPainter>

#include <Plasma/Theme>
#include <Plasma/PaintUtils>

REGISTER_GROUP(flow, FlowGroup)

class Spacer : public QGraphicsWidget
{
    public:
        Spacer(QGraphicsWidget *parent)
        : QGraphicsWidget(parent)
        {}

        ~Spacer()
        {}

        int index;

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0)
        {
            Q_UNUSED(option)
            Q_UNUSED(widget)

            //TODO: make this a pretty gradient?
            painter->setRenderHint(QPainter::Antialiasing);
            QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect().adjusted(1, 1, -2, -2), 4);
            QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
            c.setAlphaF(0.3);

            painter->fillPath(p, c);
        }
};


FlowGroup::FlowGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
          : AbstractGroup(parent, wFlags),
            m_layout(new QGraphicsLinearLayout(Qt::Horizontal)),
            m_spacer(new Spacer(this))
{
    setLayout(m_layout);

    m_spacer->hide();

    resize(200,200);
    setGroupType(AbstractGroup::ConstrainedGroup);
    setUseSimplerBackgroundForChildren(true);
}

FlowGroup::~FlowGroup()
{

}

QString FlowGroup::pluginName() const
{
    return QString("flow");
}

void FlowGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    m_layout->insertItem(group.readEntry("Position", -1), child);
}

void FlowGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    int pos = 0;
    for (int i = 0; i < m_layout->count(); ++i) {
        if (child == m_layout->itemAt(i)) {
            pos = i;
            break;
        }
    }

    group.writeEntry("Position", pos);
}

bool FlowGroup::showDropZone(const QPointF &pos)
{
    if (pos == QPointF()) {
        m_layout->removeItem(m_spacer);
        m_spacer->hide();

        return false;
    }

    //lucky case: the spacer is already in the right position
//     if (m_spacer->geometry().contains(pos)) {
//         return true;
//     }

//     Plasma::FormFactor f = formFactor();
    int insertIndex = m_layout->count();

    //FIXME: needed in two places, make it a function?
    for (int i = 0; i < m_layout->count(); ++i) {
        QRectF siblingGeometry = m_layout->itemAt(i)->geometry();

//         if (f == Plasma::Horizontal) {
            qreal middle = siblingGeometry.left() + (siblingGeometry.width() / 2.0);
            if (pos.x() < middle) {
                insertIndex = i;
                break;
            } else if (pos.x() <= siblingGeometry.right()) {
                insertIndex = i + 1;
                break;
            }
//         } else { // Plasma::Vertical
//             qreal middle = siblingGeometry.top() + (siblingGeometry.height() / 2.0);
//             if (pos.y() < middle) {
//                 insertIndex = i;
//                 break;
//             } else if (pos.y() <= siblingGeometry.bottom()) {
//                 insertIndex = i + 1;
//                 break;
//             }
//         }
    }

    m_spacer->index = insertIndex;
    if (insertIndex != -1) {
        m_spacer->show();
        m_layout->insertItem(insertIndex, m_spacer);

        return true;
    }

    return false;
}

void FlowGroup::layoutChild(QGraphicsWidget *child, const QPointF &)
{
    m_spacer->hide();
    m_layout->removeItem(m_spacer);
    m_layout->insertItem(m_spacer->index, child);
}

QString FlowGroup::prettyName()
{
    return i18n("Flow Group");
}

QSet<Plasma::FormFactor> FlowGroup::availableOnFormFactors()
{
    QSet<Plasma::FormFactor> set;
    set.insert(Plasma::Planar);
    set.insert(Plasma::MediaCenter);
    set.insert(Plasma::Vertical);
    set.insert(Plasma::Horizontal);

    return set;
}

#include "flowgroup.moc"
