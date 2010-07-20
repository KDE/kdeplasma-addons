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

#include <QAction>

#include <KDebug>
#include <KIcon>

#include <Plasma/AbstractToolBox>
#include <Plasma/View>
#include <Plasma/PaintUtils>

#include <kephal/screens.h>

#include "../lib/abstractgroup.h"

using namespace Plasma;

GroupingPanel::GroupingPanel(QObject *parent, const QVariantList &args)
    : GroupingContainment(parent, args),
      m_configureAction(0),
      m_currentSize(QSize(Kephal::ScreenUtils::screenSize(screen()).width(), 35)),
      m_maskDirty(true)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/panel-background");
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    resize(m_currentSize);
    setMinimumSize(m_currentSize);
    setMaximumSize(m_currentSize);
    useMainGroup("grid");

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
}

GroupingPanel::~GroupingPanel()
{
}

void GroupingPanel::init()
{
    setContainmentType(Containment::CustomPanelContainment);

    GroupingContainment::init();
    //FIXME: This should be enabled, but in that case proxywidgets won't get rendered
    //setFlag(ItemClipsChildrenToShape, true);

    updateBorders(geometry().toRect());

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

void GroupingPanel::updateBorders(const QRect &geom)
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

    update();
}

void GroupingPanel::constraintsEvent(Plasma::Constraints constraints)
{
    GroupingContainment::constraintsEvent(constraints);

    m_maskDirty = true;

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

        if (constraints & Plasma::LocationConstraint) {
            setFormFactorFromLocation(location());
        }
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
    updateBorders(geometry().toRect());
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

K_EXPORT_PLASMA_APPLET(groupingpanel, GroupingPanel)

#include "groupingpanel.moc"

