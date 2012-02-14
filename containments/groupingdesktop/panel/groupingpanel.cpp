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
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneContextMenuEvent>

#include <KLocale>
#include <KDebug>
#include <KIcon>

#include <Plasma/AbstractToolBox>
#include <Plasma/View>
#include <Plasma/PaintUtils>
#include <Plasma/Corona>
#include <Plasma/WindowEffects>

#include "../lib/abstractgroup.h"

using namespace Plasma;

GroupingPanel::GroupingPanel(QObject *parent, const QVariantList &args)
    : GroupingContainment(parent, args),
      m_configureAction(0),
      m_newRowAction(0),
      m_delRowAction(0),
      m_layout(0),
      m_separator(new Plasma::Svg()),
      m_maskDirty(true),
      m_rightBorder(0),
      m_bottomBorder(0)
{
    KGlobal::locale()->insertCatalog("libplasma_groupingcontainment");

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/panel-background");
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    m_separator->setImagePath("widgets/line");
    m_separator->setContainsMultipleImages(true);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    resize(m_currentSize);
    setMinimumSize(m_currentSize);
    setMaximumSize(m_currentSize);
    useMainGroup("flow");

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
}

GroupingPanel::~GroupingPanel()
{
    delete m_separator;
}

void GroupingPanel::init()
{
    setContainmentType(Containment::CustomPanelContainment);

    m_currentSize = QSize(corona()->screenGeometry(screen()).width(), 35);

    GroupingContainment::init();
    //FIXME: This should be enabled, but in that case proxywidgets won't get rendered
    //setFlag(ItemClipsChildrenToShape, true);

    updateBorders(geometry().toRect());

    KConfigGroup cg = config("Configuration");

    m_currentSize = cg.readEntry("minimumSize", m_currentSize);
    if (formFactor() == Plasma::Vertical) {
        m_currentSize.expandedTo(QSize(0, 35));
        m_layout = new QGraphicsLinearLayout(Qt::Horizontal);
    } else {
        m_currentSize.expandedTo(QSize(35, 0));
        m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    }
    m_layout->setContentsMargins(0, 0, m_rightBorder, m_bottomBorder);
    setLayout(m_layout);

    setMinimumSize(cg.readEntry("minimumSize", m_currentSize));
    setMaximumSize(cg.readEntry("maximumSize", m_currentSize));
    setDrawWallpaper(false);
}

QList<QAction *> GroupingPanel::contextualActions()
{
    if (!m_configureAction) {
        m_configureAction = new QAction(i18n("Panel Settings"), this);
        m_configureAction->setIcon(KIcon("configure"));
        connect(m_configureAction, SIGNAL(triggered()), this, SIGNAL(toolBoxToggled()));

        constraintsEvent(Plasma::ImmutableConstraint);
    }
    if (!m_newRowAction) {
        m_newRowAction = new QAction(this);
        m_newRowAction->setIcon(KIcon("list-add"));
        if (formFactor() == Plasma::Vertical) {
            m_newRowAction->setText(i18n("Add A New Column"));
        } else {
            m_newRowAction->setText(i18n("Add A New Row"));
        }
        connect(m_newRowAction, SIGNAL(triggered()), this, SLOT(addNewRow()));
    }

    m_delRowAction->setVisible(m_delRowAction->data().toInt() != -1);

    QList<QAction *> actions = GroupingContainment::contextualActions();

    actions << m_configureAction << m_newRowAction << m_delRowAction;
    return actions;
}

void GroupingPanel::addNewRow()
{
    AbstractGroup *g = addGroup("flow");
    g->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    m_layout->addItem(g);
    g->setIsMainGroup();

    KConfigGroup groupsConfig = config("Groups");
    KConfigGroup groupConfig(&groupsConfig, QString::number(g->id()));
    KConfigGroup layoutConfig(&groupConfig, "LayoutInformation");

    layoutConfig.writeEntry("Index", m_layout->count() - 1);

    emit configNeedsSaving();
}

void GroupingPanel::delRow()
{
    AbstractGroup *g = static_cast<AbstractGroup *>(m_layout->itemAt(m_delRowAction->data().toInt()));
    m_layout->removeItem(g);
    if (mainGroup() == g) {
        setMainGroup(static_cast<AbstractGroup *>(m_layout->itemAt(0)));
    }
    g->destroy();

    m_delRowAction->setData(-1);
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
        QRect r = corona()->screenGeometry(s);

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
        QRect r = corona()->screenGeometry(s);

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

    if (formFactor() == Vertical) {
        m_bottomBorder = bottomHeight;
        m_rightBorder = 0;
    } else {
        m_rightBorder = rightWidth;
        m_bottomBorder = 0;
    }

    if (m_layout) {
        m_layout->setContentsMargins(0, 0, m_rightBorder, m_bottomBorder);
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
        QRectF screenRect = screen() >= 0 ? corona()->screenGeometry(screen()) :
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
                           const QRect &)
{
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
            const QRegion mask = m_background->mask();
            containmentOpt->view->setMask(mask);
            Plasma::WindowEffects::enableBlurBehind(containmentOpt->view->winId(), true, mask);
        }
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->setRenderHint(QPainter::Antialiasing);

    m_background->paintFrame(painter, option->exposedRect);

    QRectF rect = contentsRect();
    if (formFactor() == Plasma::Vertical) {
        rect.setHeight(rect.height() - m_bottomBorder);
        QSizeF lineSize(m_separator->elementSize("vertical-line").width(), rect.height());
        for (int i = 1; i < m_layout->count(); ++i) {
            const qreal x = rect.width() * i / m_layout->count();
            QRectF r(rect.topLeft() + QPointF(x - lineSize.width() / 2., 0), lineSize);
            m_separator->paint(painter, r, "vertical-line");
        }
    } else {
        rect.setWidth(rect.width() - m_rightBorder);
        QSizeF lineSize = QSizeF(rect.width(), m_separator->elementSize("horizontal-line").height());
        for (int i = 1; i < m_layout->count(); ++i) {
            const qreal y = rect.height() * i / m_layout->count();
            QRectF r(rect.topLeft() + QPointF(0, y - lineSize.height() / 2.), lineSize);
            m_separator->paint(painter, r, "horizontal-line");
        }
    }
}

void GroupingPanel::setFormFactorFromLocation(Plasma::Location loc) {
    switch (loc) {
        case BottomEdge:
        case TopEdge:
            //kDebug() << "setting horizontal form factor";
            setFormFactor(Plasma::Horizontal);
            m_layout->setOrientation(Qt::Vertical);
            if (m_newRowAction) {
                m_newRowAction->setText(i18n("Add A New Row"));
                m_delRowAction->setText(i18n("Remove This Row"));
            }
            break;
        case RightEdge:
        case LeftEdge:
            //kDebug() << "setting vertical form factor";
            setFormFactor(Plasma::Vertical);
            m_layout->setOrientation(Qt::Horizontal);
            if (m_newRowAction) {
                m_newRowAction->setText(i18n("Add A New Column"));
                m_delRowAction->setText(i18n("Remove This Column"));
            }
            break;
        case Floating:
            //TODO: implement a form factor for floating panels
            kDebug() << "Floating is unimplemented.";
            break;
        default:
            kDebug() << "invalid location!!";
    }
}

void GroupingPanel::layoutMainGroup()
{
    mainGroup()->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    m_layout->addItem(mainGroup());

    KConfigGroup groupsConfig = config("Groups");
    KConfigGroup groupConfig(&groupsConfig, QString::number(mainGroup()->id()));
    KConfigGroup layoutConfig(&groupConfig, "LayoutInformation");

    layoutConfig.writeEntry("Index", m_layout->count() - 1);
}

void GroupingPanel::restore(KConfigGroup &group)
{
    GroupingContainment::restore(group);

    KConfigGroup groupsConfig(&group, "Groups");

    foreach (AbstractGroup *group, groups()) {
        KConfigGroup groupConfig(&groupsConfig, QString::number(group->id()));
        KConfigGroup layoutConfig(&groupConfig, "LayoutInformation");

        if (layoutConfig.isValid()) {
            int order = layoutConfig.readEntry("Index", -1);

            m_layout->insertItem(order, group);
            group->setIsMainGroup();
        }
    }
}

void GroupingPanel::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!m_delRowAction) {
        m_delRowAction = new QAction(this);
        m_delRowAction->setIcon(KIcon("list-remove"));
        m_delRowAction->setData(-1);
        if (formFactor() == Plasma::Vertical) {
            m_delRowAction->setText(i18n("Remove This Column"));
        } else {
            m_delRowAction->setText(i18n("Remove This Row"));
        }
        connect(m_delRowAction, SIGNAL(triggered()), this, SLOT(delRow()));
    }

    if (m_layout->count() > 1) {
        QPointF pos = event->pos();
        for (int i = 0; i < m_layout->count(); ++i) {
            if (m_layout->itemAt(i)->geometry().contains(pos)) {
                m_delRowAction->setData(i);
                break;
            }
        }
    } else {
        m_delRowAction->setData(-1);
    }

    GroupingContainment::contextMenuEvent(event);
}

K_EXPORT_PLASMA_APPLET(groupingpanel, GroupingPanel)

#include "groupingpanel.moc"

