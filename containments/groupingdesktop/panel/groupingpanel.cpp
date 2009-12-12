/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
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

#include "groupingpanel.h"

#include <limits>

#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QComboBox>
#include <QDesktopWidget>
#include <QGraphicsLinearLayout>
#include <QGridLayout>
#include <QGraphicsLayout>
#include <QGraphicsSceneDragDropEvent>
#include <QLabel>
#include <QMenu>
#include <QTimer>
#include <QPainter>
#include <QSignalMapper>

#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KIntNumInput>
#include <KMessageBox>

#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/AbstractToolBox>
#include <Plasma/View>
#include <Plasma/PaintUtils>

#include <kephal/screens.h>

#include "../lib/abstractgroup.h"

using namespace Plasma;

class Spacer : public QGraphicsWidget
{
public:
    Spacer(QGraphicsWidget *parent)
         : QGraphicsWidget(parent),
           m_visible(true)
    {
        setAcceptDrops(true);
    }

    ~Spacer()
    {}

    GroupingPanel *panel;
    bool m_visible;

protected:
    void dropEvent(QGraphicsSceneDragDropEvent *event)
    {
        event->setPos(mapToParent(event->pos()));
        panel->dropEvent(event);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        if (!m_visible) {
            return;
        }

        //TODO: make this a pretty gradient?
        painter->setRenderHint(QPainter::Antialiasing);
        QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect().adjusted(1, 1, -2, -2), 4);
        QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        c.setAlphaF(0.3);

        painter->fillPath(p, c);
    }
};

GroupingPanel::GroupingPanel(QObject *parent, const QVariantList &args)
    : GroupingContainment(parent, args),
      m_configureAction(0),
      m_currentSize(QSize(Kephal::ScreenUtils::screenSize(screen()).width(), 35)),
      m_maskDirty(true),
      m_canResize(true),
      m_spacerIndex(-1),
      m_spacer(0),
      m_lastSpace(0),
      m_layout(0)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/panel-background");
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    resize(m_currentSize);
    setMinimumSize(m_currentSize);
    setMaximumSize(m_currentSize);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));

    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));
    connect(this, SIGNAL(groupRemoved(AbstractGroup*)),
            this, SLOT(groupRemoved(AbstractGroup*)));
}

GroupingPanel::~GroupingPanel()
{
}

void GroupingPanel::init()
{
    setContainmentType(Containment::PanelContainment);

    GroupingContainment::init();
    //FIXME: This should be enabled, but in that case proxywidgets won't get rendered
    //setFlag(ItemClipsChildrenToShape, true);

    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setLayout(m_layout);
    updateBorders(geometry().toRect());
    m_layout->setMaximumSize(size());

    KConfigGroup cg = config("Configuration");

    m_currentSize = cg.readEntry("minimumSize", m_currentSize);
    if (formFactor() == Plasma::Vertical) {
        m_currentSize.expandedTo(QSize(0, 35));
    } else {
        m_currentSize.expandedTo(QSize(35, 0));
    }

    setMinimumSize(cg.readEntry("minimumSize", m_currentSize));
    setMaximumSize(cg.readEntry("maximumSize", m_currentSize));
    setDrawWallpaper(false);

    m_lastSpaceTimer = new QTimer(this);
    m_lastSpaceTimer->setSingleShot(true);
    connect(m_lastSpaceTimer, SIGNAL(timeout()), this, SLOT(adjustLastSpace()));
}

QList<QAction*> GroupingPanel::contextualActions()
{
    if (!m_configureAction) {
        m_configureAction = new QAction(i18n("Panel Settings"), this);
        m_configureAction->setIcon(KIcon("configure"));
        connect(m_configureAction, SIGNAL(triggered()), this, SIGNAL(toolBoxToggled()));

        constraintsEvent(Plasma::ImmutableConstraint);
    }

    QList<QAction*> actions;

    actions.append(m_configureAction);

    return GroupingContainment::contextualActions();
}

void GroupingPanel::backgroundChanged()
{
    constraintsEvent(Plasma::LocationConstraint);
}

void GroupingPanel::adjustLastSpace()
{
    if (!m_layout) {
        return;
    }

    bool useSpacer = true;

    if (formFactor() == Plasma::Vertical) {
        foreach (Applet *applet, applets()) {
            if (applet->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag) {
                useSpacer = false;
                break;
            }
        }
    } else {
        foreach (Applet *applet, applets()) {
            if (applet->sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag) {
                useSpacer = false;
                break;
            }
        }
    }

    if (useSpacer) {
        if (!m_lastSpace) {
            m_lastSpace = new Spacer(this);
            m_lastSpace->panel = this;
            m_lastSpace->m_visible = false;
            m_lastSpace->setPreferredSize(0,0);
            m_lastSpace->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            m_layout->addItem(m_lastSpace);
        }
    } else {
        m_layout->removeItem(m_lastSpace);
        delete m_lastSpace;
        m_lastSpace = 0;
    }
}

void GroupingPanel::enableUpdateSize()
{
    m_canResize = true;
}

void GroupingPanel::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    layoutWidget(applet, pos);
}

void GroupingPanel::layoutGroup(AbstractGroup *group, const QPointF &pos)
{
    layoutWidget(group, pos);
}

void GroupingPanel::layoutWidget(QGraphicsWidget *widget, const QPointF &pos)
{
    // this gets called whenever an applet is added, and we add it to our layout

    if (!m_layout) {
        return;
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = -1;

    //Enlarge the panel if possible and needed
    QSizeF widgetHint = widget->preferredSize();
    QSizeF panelHint = layout()->preferredSize();
    if (f == Plasma::Horizontal) {
        if (panelHint.width() + widgetHint.width() > size().width()) {
            resize(panelHint.width() + widgetHint.width(), size().height());
        }
    } else {
        if (panelHint.height() + widgetHint.height() > size().height()) {
            resize(size().width(), panelHint.height() + widgetHint.height());
        }
    }
    m_layout->setMaximumSize(size());

    //if pos is (-1,-1) insert at the end of the panel
    if (pos != QPoint(-1, -1)) {
        for (int i = 0; i < m_layout->count(); ++i) {
            QRectF siblingGeometry = m_layout->itemAt(i)->geometry();
            if (f == Plasma::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (QApplication::layoutDirection() == Qt::RightToLeft) {
                    if (pos.x() > middle) {
                        insertIndex = i;
                    } else if (pos.x() >= siblingGeometry.left()) {
                        insertIndex = i + 1;
                    }
                } else if (pos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { // Plasma::Vertical
                qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.y() <= siblingGeometry.bottom()) {
                    insertIndex = i + 1;
                    break;
                }
            }
        }
    }

    m_layout->removeItem(m_lastSpace);

    if (insertIndex == -1 || insertIndex >= m_layout->count()) {
        m_layout->addItem(widget);
    } else {
        m_layout->insertItem(insertIndex, widget);
    }

    if (m_lastSpace) {
        m_layout->addItem(m_lastSpace);
    }

    //FIXME: there must be some beter way to do this rather than this rather error prone arbitrary wait
    m_lastSpaceTimer->start(2000);

    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);
    if (applet) {
        connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
    }
}

void GroupingPanel::appletRemoved(Plasma::Applet *applet)
{
    widgetRemoved(applet);
}

void GroupingPanel::groupRemoved(AbstractGroup *group)
{
    widgetRemoved(group);
}

void GroupingPanel::widgetRemoved(QGraphicsWidget *widget)
{
    if (!m_layout) {
        return;
    }

    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);
    if (applet) {
        connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
    }

    m_layout->removeItem(widget);

    //shrink the panel if possible
    if (formFactor() == Plasma::Horizontal) {
        resize(size().width() - widget->size().width(), size().height());
    } else {
        resize(size().width(), size().height() - widget->size().height());
    }
    m_layout->setMaximumSize(size());

    m_lastSpaceTimer->start(200);
}

void GroupingPanel::updateSize()
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(sender());

    if (m_canResize && applet) {
        if (formFactor() == Plasma::Horizontal) {
            const int delta = applet->preferredWidth() - applet->size().width();
            //setting the preferred width when delta = 0 and preferredWidth() < minimumWidth()
            // leads to the same thing as setPreferredWidth(minimumWidth())
            if (delta != 0) {
                setPreferredWidth(preferredWidth() + delta);
            }
        } else if (formFactor() == Plasma::Vertical) {
            const int delta = applet->preferredHeight() - applet->size().height();
            if (delta != 0) {
                setPreferredHeight(preferredHeight() + delta);
            }
        }

        resize(preferredSize());
        //for a while we won't execute updateSize() again
        m_canResize = false;
        QTimer::singleShot(400, this, SLOT(enableUpdateSize()));
    }
}

void GroupingPanel::updateBorders(const QRect &geom, bool themeChange)
{
    Plasma::Location loc = location();
    FrameSvg::EnabledBorders enabledBorders = FrameSvg::AllBorders;

    int s = screen();
    //kDebug() << loc << s << formFactor() << geometry();

    qreal topHeight = m_background->marginSize(Plasma::TopMargin);
    qreal bottomHeight = m_background->marginSize(Plasma::BottomMargin);
    qreal leftWidth = m_background->marginSize(Plasma::LeftMargin);
    qreal rightWidth = m_background->marginSize(Plasma::RightMargin);

    //remove unwanted borders
    if (s < 0) {
        // do nothing in this case, we want all the borders
    } else if (loc == BottomEdge || loc == TopEdge) {
        QRect r = Kephal::ScreenUtils::screenGeometry(s);

        if (loc == BottomEdge) {
            enabledBorders ^= FrameSvg::BottomBorder;
            bottomHeight = 0;
        } else {
            enabledBorders ^= FrameSvg::TopBorder;
            topHeight = 0;
        }

        if (geom.x() <= r.x()) {
            enabledBorders ^= FrameSvg::LeftBorder;
            leftWidth = 0;
        }
        if (geom.right() >= r.right()) {
            enabledBorders ^= FrameSvg::RightBorder;
            rightWidth = 0;
        }

        //kDebug() << "top/bottom: Width:" << width << ", height:" << height;
    } else if (loc == LeftEdge || loc == RightEdge) {
        QRect r = Kephal::ScreenUtils::screenGeometry(s);

        if (loc == RightEdge) {
            enabledBorders ^= FrameSvg::RightBorder;
            rightWidth = 0;
        } else {
            enabledBorders ^= FrameSvg::LeftBorder;
            leftWidth = 0;
        }

        if (geom.y() <= r.y()) {
            enabledBorders ^= FrameSvg::TopBorder;
            topHeight = 0;
        }

        if (geom.bottom() >= r.bottom()) {
            enabledBorders ^= FrameSvg::BottomBorder;
            bottomHeight = 0;
        }

        //kDebug() << "left/right: Width:" << width << ", height:" << height;
    } else {
        kDebug() << "no location!?";
    }

    //activate borders and fetch sizes again
    m_background->setEnabledBorders(enabledBorders);
    m_background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    //calculation of extra margins has to be done after getMargins
    const QGraphicsItem *box = toolBox();
    if (box && immutability() == Mutable) {
        QSizeF s = box->boundingRect().size();
        if (formFactor() == Vertical) {
            //hardcoded extra margin for the toolbox right now
            bottomHeight += s.height();
            //Default to horizontal for now
        } else {
            rightWidth += s.width();
        }
    }

    //invalidate the layout and set again
    if (m_layout) {
        switch (location()) {
        case LeftEdge:
            rightWidth = qMin(rightWidth, qMax(qreal(1), size().width() - KIconLoader::SizeMedium));
            break;
        case RightEdge:
            leftWidth = qMin(leftWidth, qMax(qreal(1), size().width() - KIconLoader::SizeMedium));
            break;
        case TopEdge:
            bottomHeight = qMin(bottomHeight, qMax(qreal(1), size().height() - KIconLoader::SizeMedium));
            break;
        case BottomEdge:
            topHeight = qMin(topHeight, qMax(qreal(1), size().height() - KIconLoader::SizeMedium));
            break;
        default:
            break;
        }
        qreal oldLeft = leftWidth;
        qreal oldTop = topHeight;
        qreal oldRight = rightWidth;
        qreal oldBottom = bottomHeight;

        if (themeChange) {
            m_layout->getContentsMargins(&oldLeft, &oldTop, &oldRight, &oldBottom);
        }

        m_layout->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

        m_layout->invalidate();
        resize(preferredSize());
    }

    update();
}

void GroupingPanel::constraintsEvent(Plasma::Constraints constraints)
{
    m_maskDirty = true;

    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor form = formFactor();
        Qt::Orientation layoutDirection = form == Plasma::Vertical ? Qt::Vertical : Qt::Horizontal;
        // create or set up our layout!
        if (m_layout) {
            m_layout->setMaximumSize(size());
            m_layout->setOrientation(layoutDirection);
        }
    }

    //we need to know if the width or height is 100%
    if (constraints & Plasma::LocationConstraint || constraints & Plasma::SizeConstraint) {
        m_currentSize = geometry().size().toSize();
        QRectF screenRect = screen() >= 0 ? Kephal::ScreenUtils::screenGeometry(screen()) :
                                            geometry();

        if ((formFactor() == Horizontal && m_currentSize.width() >= screenRect.width()) ||
            (formFactor() == Vertical && m_currentSize.height() >= screenRect.height())) {
            m_background->setElementPrefix(location());
        } else {
            switch (location()) {
            case LeftEdge:
                //this call will automatically fallback at no prefix if the element isn't available
                m_background->setElementPrefix("west-mini");
                break;
            case RightEdge:
                m_background->setElementPrefix("east-mini");
                break;
            case TopEdge:
                m_background->setElementPrefix("north-mini");
                break;
            case BottomEdge:
            default:
                m_background->setElementPrefix("south-mini");
                break;
            }
        }

        m_background->resizeFrame(m_currentSize);

        //FIXME: this seems the only way to correctly resize the layout the first time when the
        // saved panel size is less than the default is to setting a maximum size.
        // this shouldn't happen. maybe even a qgraphicslayout bug?
        if (m_layout && (constraints & Plasma::SizeConstraint)) {
            m_layout->setMaximumSize(size());
        }

        if (constraints & Plasma::LocationConstraint) {
            setFormFactorFromLocation(location());
        }
    }

    if (constraints & Plasma::StartupCompletedConstraint) {
        connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
                this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
        connect(this, SIGNAL(groupAdded(AbstractGroup*,QPointF)),
                this, SLOT(layoutGroup(AbstractGroup*,QPointF)));
    }

    if (constraints & Plasma::ImmutableConstraint) {
        bool unlocked = immutability() == Plasma::Mutable;

        if (m_configureAction) {
            m_configureAction->setEnabled(unlocked);
            m_configureAction->setVisible(unlocked);
        }

        updateBorders(geometry().toRect());
    }
}

void GroupingPanel::saveState(KConfigGroup &config) const
{
    config.writeEntry("minimumSize", minimumSize());
    config.writeEntry("maximumSize", maximumSize());
}

void GroupingPanel::themeUpdated()
{
    updateBorders(geometry().toRect(), true);
}

void GroupingPanel::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           const QRect& contentsRect)
{
    Q_UNUSED(contentsRect)
    //FIXME: this background drawing is bad and ugly =)
    // draw the background untransformed (saves lots of per-pixel-math)
    painter->resetTransform();

    const Containment::StyleOption *containmentOpt = qstyleoption_cast<const Containment::StyleOption *>(option);

    QRect viewGeom;
    if (containmentOpt && containmentOpt->view) {
        viewGeom = containmentOpt->view->geometry();
    }

    if (m_maskDirty || m_lastViewGeom != viewGeom) {
        m_maskDirty = false;
        m_lastViewGeom = viewGeom;

        updateBorders(viewGeom);
        if (containmentOpt && containmentOpt->view && !m_background->mask().isEmpty()) {
            containmentOpt->view->setMask(m_background->mask());
        }
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->setRenderHint(QPainter::Antialiasing);

    m_background->paintFrame(painter, option->exposedRect);
}

void GroupingPanel::setFormFactorFromLocation(Plasma::Location loc) {
    switch (loc) {
        case BottomEdge:
        case TopEdge:
            //kDebug() << "setting horizontal form factor";
            setFormFactor(Plasma::Horizontal);
            break;
        case RightEdge:
        case LeftEdge:
            //kDebug() << "setting vertical form factor";
            setFormFactor(Plasma::Vertical);
            break;
        case Floating:
            //TODO: implement a form factor for floating panels
            kDebug() << "Floating is unimplemented.";
            break;
        default:
            kDebug() << "invalid location!!";
    }
}

void GroupingPanel::showDropZone(const QPoint pos)
{
    if (!scene() || !m_layout) {
        return;
    }

    if (pos == QPoint()) {
        if (m_spacer) {
            m_layout->removeItem(m_spacer);
            m_spacer->hide();
        }
        return;
    }

    //lucky case: the spacer is already in the right position
    if (m_spacer && m_spacer->geometry().contains(pos)) {
        return;
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = m_layout->count();

    //FIXME: needed in two places, make it a function?
    for (int i = 0; i < m_layout->count(); ++i) {
        QRectF siblingGeometry = m_layout->itemAt(i)->geometry();

        if (f == Plasma::Horizontal) {
            qreal middle = siblingGeometry.left() + (siblingGeometry.width() / 2.0);
            if (pos.x() < middle) {
                insertIndex = i;
                break;
            } else if (pos.x() <= siblingGeometry.right()) {
                insertIndex = i + 1;
                break;
            }
        } else { // Plasma::Vertical
            qreal middle = siblingGeometry.top() + (siblingGeometry.height() / 2.0);
            if (pos.y() < middle) {
                insertIndex = i;
                break;
            } else if (pos.y() <= siblingGeometry.bottom()) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        if (!m_spacer) {
            m_spacer = new Spacer(this);
            m_spacer->panel = this;
        } else {
            m_layout->removeItem(m_spacer);
        }

        m_spacer->show();
        m_layout->insertItem(insertIndex, m_spacer);
    }
}

void GroupingPanel::restore(KConfigGroup &group)
{
    GroupingContainment::restore(group);

    KConfigGroup appletsConfig(&group, "Applets");

    QMap<int, Applet *> oderedApplets;
    QList<Applet *> unoderedApplets;

    foreach (Applet *applet, applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup layoutConfig(&appletConfig, "LayoutInformation");

        int order = layoutConfig.readEntry("Order", -1);

        if (order > -1) {
            oderedApplets[order] = applet;
        //if LayoutInformation is not available use the usual way, as a bonus makes it retrocompatible with oler configs
        } else {
            unoderedApplets.append(applet);
        }

        connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
    }

    foreach (Applet *applet, oderedApplets) {
        if (m_lastSpace) {
            m_layout->insertItem(m_layout->count()-1, applet);
        } else {
            m_layout->addItem(applet);
        }
    }

    foreach (Applet *applet, unoderedApplets) {
        layoutApplet(applet, applet->pos());
    }

    updateSize();
}

void GroupingPanel::saveContents(KConfigGroup &group) const
{
    GroupingContainment::saveContents(group);

    KConfigGroup appletsConfig(&group, "Applets");
    for (int order = 0; order < m_layout->count(); ++order) {
        const Applet *applet = dynamic_cast<Applet *>(m_layout->itemAt(order));
        if (applet) {
            KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
            KConfigGroup layoutConfig(&appletConfig, "LayoutInformation");

            layoutConfig.writeEntry("Order", order);
        }
    }
}

K_EXPORT_PLASMA_APPLET(groupingpanel, GroupingPanel)

#include "groupingpanel.moc"

