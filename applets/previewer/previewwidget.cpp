/*
 *  Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>

 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of 
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "previewwidget.h"

#include "previewitemmodel.h"

// Qt
#include <QGraphicsSceneWheelEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimeLine>

// KDE
#include <KDebug>
#include <KConfigGroup>
#include <KGlobalSettings>
#include <KService>
#include <KIconLoader>
#include <KIO/PreviewJob>
#include <KUrl>

// Plasma
#include <Plasma/Animator>
#include <Plasma/FrameSvg>
#include <Plasma/ScrollBar>
#include <Plasma/Svg>
#include <Plasma/Theme>

const int EXPANDING_DURATION = 200;

PreviewWidget::PreviewWidget(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_selectedIndex(-1),
      m_hoveredIndex(-1),
      m_closeStatus(true),
      m_animId(-1)
{
    m_scrollBar = new Plasma::ScrollBar(this);
    connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrolled( int )));
    m_scrollBar->setSingleStep(10);
    m_scrollBar->hide();

    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    //kDebug() << "setting dialog up";
    setupOptionViewItem();

    // let's draw the top border of the applet
    m_panel = new Plasma::FrameSvg(this);
    m_panel->setImagePath("dialogs/background");

    m_logo = new Plasma::Svg(this);
    m_logo->setImagePath("widgets/previewer-16");

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(setupOptionViewItem()));
}

PreviewWidget::~PreviewWidget()
{}

void PreviewWidget::setupOptionViewItem()
{
    m_option.palette.setColor(QPalette::All, QPalette::Text,
                              Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    m_option.font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DesktopFont);
    m_option.fontMetrics = QFontMetrics(m_option.font);
    m_option.decorationPosition = QStyleOptionViewItem::Top;
    m_option.decorationAlignment = Qt::AlignTop | Qt::AlignHCenter;
    m_option.decorationSize = iconSize();
    m_option.displayAlignment = Qt::AlignHCenter;
    m_option.textElideMode = Qt::ElideRight;
    m_option.features = QStyleOptionViewItemV2::WrapText;
    m_option.locale = QLocale::system();
    m_option.widget = 0;
    m_option.viewItemPosition = QStyleOptionViewItemV4::OnlyOne;
    m_option.state |= QStyle::State_Enabled;
}

void PreviewWidget::layoutItems()
{
    //kDebug() << "layouting items";
    m_items.clear();

    if (m_previewHistory.isEmpty()) {
        return;
    }

    const int itemWidth = iconSize().width() * 2;
    const int itemHeight = iconSize().height() * 2;
    int y = s_spacing + m_itemsRect.y() - (m_scrollBar->value() / itemHeight) * (itemHeight + s_spacing);
    int lineCount = m_previewHistory.count(); // number of lines of item..

    for (int i = 0; i < lineCount; i++) {
        m_items << QRect(m_itemsRect.x() + s_spacing, y, itemWidth, itemHeight);
        y += (itemHeight + s_spacing);
    }

    m_scrollBar->setRange(0, (lineCount - 1) * itemHeight);
    m_scrollBar->setSingleStep(itemHeight + s_spacing);
    m_scrollBar->setPageStep(m_itemsRect.height());

    if (m_animId < 1) {
        if (m_items.last().bottom() > m_itemsRect.bottom() || m_items.first().top() < m_itemsRect.top()) {
            if (!m_scrollBar->isVisible()) {
                m_scrollBar->show();
                calculateRects();
            }
        } else if (m_scrollBar->isVisible()) {
            m_scrollBar->hide();
            calculateRects();
        }
    }

    m_layoutIsValid = true;
}

QSize PreviewWidget::iconSize() const
{
    const int size = KIconLoader::global()->currentSize(KIconLoader::Desktop);
    return QSize(size, size);
}

void PreviewWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    const int numDegrees = event->delta() / 8;
    const int numSteps = numDegrees / 15;
    const int itemHeight = iconSize().height() * 2;

    m_scrollBar->setValue(m_scrollBar->value() - (numSteps * itemHeight));
    QGraphicsItem::wheelEvent(event);
}

void PreviewWidget::lookForPreview()
{
    m_previewReady = false;
    if (m_previewHistory.isEmpty()) {
        return;
    }

    m_previews.clear();
    QList<KFileItem> f_items;
    for (int i = 0; i < m_previewHistory.count(); i++) {
        f_items << KFileItem(KFileItem::Unknown, KFileItem::Unknown, KUrl(m_previewHistory[i]));
    }

    KIO::PreviewJob *previewJob = new KIO::PreviewJob(f_items, 256, 512, 0, 0, false, true, 0);
    connect(previewJob, SIGNAL(gotPreview(KFileItem,QPixmap)),
            this, SLOT(setPreview(KFileItem,QPixmap)));
}

void PreviewWidget::setPreview(const KFileItem &item, const QPixmap &pixmap)
{
    m_previews.insert(item.url(), pixmap);
    m_previewReady = true;
    updatePreview();
}

void PreviewWidget::updatePreview()
{
    //NOTE: use this to update the exact rect in order to repaint
    //      correctly the preview

    QRect rect = m_previewRect;
    rect.setHeight(rect.height() + bottomBorderHeight());
    update(rect);
}

void PreviewWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    updateHoveredItems(event->pos().toPoint());
}

void PreviewWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if (m_hoveredIndex != -1) {
        update(m_items[m_hoveredIndex]);
    }

    m_hoveredIndex = -1;
}

void PreviewWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    updateHoveredItems(event->pos().toPoint());
}

void PreviewWidget::updateSelectedItems(const QPoint &point)
{
    const int previous = m_selectedIndex;
    m_selectedIndex = -1;

    for (int i = 0; i < m_items.count(); i++) {
        if (m_items[i].contains(point)) {
            m_selectedIndex = i;
            emit fileOpenRequested(KUrl(m_previewHistory[i]));
            break;
        }
    }

    if (m_selectedIndex != previous) {
        if (m_selectedIndex != -1) {
            update(m_items[m_selectedIndex]);
        }

        if (previous != -1) {
            update(m_items[previous]);
        }
    }
}

void PreviewWidget::updateHoveredItems(const QPoint &point)
{
    m_hoveredUrl = KUrl();
    const int previous = m_hoveredIndex;
    m_hoveredIndex = -1;
    int i = 0;

    if (m_itemsRect.contains(point)) {
        for (; i < m_items.count(); i++) {
            if (m_items[i].contains(point)) {
                m_hoveredIndex = i;

                //here we update the preview section
                KUrl item = KUrl(m_previewHistory[i]);
                if (m_previews.contains(item)) {
                    m_hoveredUrl = item;
                    updatePreview();
                }

                break;
            }
        }
    }

    if (m_hoveredIndex != previous) {
        if (m_hoveredIndex != -1) {
            update(m_items[m_hoveredIndex]);
        }

        if (previous != -1) {
            update(m_items[previous]);
        }
    }
}

void PreviewWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        m_clicked = scenePos().toPoint();
        event->setAccepted(true);
        return;
    }

    QGraphicsWidget::mousePressEvent(event);
}

void PreviewWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if ((m_clicked - scenePos().toPoint()).manhattanLength() < KGlobalSettings::dndEventDelay() ) {

        if (m_itemsRect.contains(event->pos().toPoint())) {
            updateSelectedItems(event->pos().toPoint());
        }

        if (m_arrowRect.contains(event->pos().toPoint())) {
            if (m_closeStatus) {
                expand();
            } else {
                contract();
            }
        }
    }

    QGraphicsWidget::mouseReleaseEvent(event);
}

void PreviewWidget::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!KUrl::List::canDecode(event->mimeData())) {
        return;
    }

    const KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
    emit urlsDropped(urls);
}

void PreviewWidget::setItemsList(const QList<QUrl> &list)
{
    m_previewHistory.clear();
    m_previewHistory = list;

    calculateRects();
    update();

    lookForPreview();
}

void PreviewWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event);
    calculateRects();
}

void PreviewWidget::calculateRects()
{
    QRect rect = contentsRect().toRect();
    const int scrollBarWidth = m_scrollBar->isVisible() ? int(m_scrollBar->preferredSize().width()) : 0;
    const int itemWidth = iconSize().width() * 2;
    const int itemRectWidth = itemWidth + scrollBarWidth + (s_spacing * 2) +
                              (scrollBarWidth > 0 ?  2 : 0);
    const int bottomBorder = bottomBorderHeight();

    // items rect shouldn't be visible even if the status is close
    if (m_previewHistory.isEmpty() || (m_closeStatus && m_animId < 1)) {
        m_animationHeight = s_topBorder + bottomBorder;
        m_scrollBar->setGeometry(QRect());
    } else {
        if (m_animId < 1) {
            m_animationHeight = rect.height();
        }
        const int itemRectHeight = m_animationHeight - s_topBorder - bottomBorder;
        m_itemsRect = QRect(5, s_topBorder, itemRectWidth, itemRectHeight);

        QRect r = QRect(m_itemsRect.right() - scrollBarWidth + 2, s_topBorder + 1, scrollBarWidth,
                        m_animationHeight - s_topBorder - bottomBorder - 2);
        m_scrollBar->setGeometry(r);
    }

    //kDebug() << m_animationHeight;

    m_arrowRect = QRect(rect.width() - 30 - 5, (s_topBorder / 2), 10,10);

    m_previewRect = QRect(m_itemsRect.right(), m_itemsRect.y(),
                         rect.width() - m_itemsRect.right() - rect.x(),
                          m_itemsRect.height());

    m_layoutIsValid = false;
}

void PreviewWidget::expandingSlot(qreal progress)
{
    const int min = s_topBorder + bottomBorderHeight();
    if (qFuzzyCompare(qreal(1.0), progress)) {
        m_animId = -1;

        if (m_closeStatus) {
            m_animationHeight = min;
        } else {
            m_animationHeight = int(size().height()) - min;

            // show the scroll bar again if necessary
            if (!m_items.isEmpty() && (m_items.last().bottom() > m_itemsRect.bottom() ||
                m_items.first().top() < m_itemsRect.top())) {
                m_scrollBar->show();
            }
        }
    } else {
        m_animationHeight = qMax(qreal(min),
                                 min - 1 + ((size().height() - min)  * 
                                            (m_closeStatus ? qreal(1.0) - progress : progress)));
    }

    calculateRects();
    update();
}

void PreviewWidget::contract()
{
    if (m_closeStatus) {
        return;
    }

    m_closeStatus = true;

    if (m_animId > 0) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    // don't draw the scrollbar while animating
    m_scrollBar->hide();

    // nasty hack alert: expandingSlot will get called before customAnimation returns
    // and we rely on m_animId != -1 .. so .. set it to a dummy value here
    m_animId = 1;
    m_animId = Plasma::Animator::self()->customAnimation(20, EXPANDING_DURATION,
                                                         Plasma::Animator::EaseInOutCurve,
                                                         this, "expandingSlot");
}

void PreviewWidget::expand()
{
    if (!m_closeStatus) {
        return;
    }

    m_closeStatus = false;

    if (m_animId > 0) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    // don't draw the scrollbar while animating
    m_scrollBar->hide();

    // nasty hack alert: expandingSlot will get called before customAnimation returns
    // and we rely on m_animId != -1 .. so .. set it to a dummy value here
    m_animId = 1;
    m_animId = Plasma::Animator::self()->customAnimation(20, EXPANDING_DURATION,
                                                         Plasma::Animator::EaseInOutCurve,
                                                         this, "expandingSlot");
}

void PreviewWidget::scrolled( int value )
{
    Q_UNUSED(value);

    m_layoutIsValid = false;
    m_hoveredIndex = -1;
    update(m_itemsRect);
}

int PreviewWidget::bottomBorderHeight() const
{
    const int svgHeight = m_panel->marginSize(Plasma::BottomMargin);

    if (m_items.isEmpty()) {
        return svgHeight;
    }

    QFontMetrics fm(KGlobalSettings::smallestReadableFont());
    return qMax(fm.height() + s_spacing * 2, svgHeight);
}

void PreviewWidget::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *opt,
                          QWidget *widget)
{
    Q_UNUSED(widget);

    if (!opt->rect.isValid()) {
        return;
    }

    // let's draw items..
    if (!m_layoutIsValid) {
        layoutItems();
    }

    const int bottomBorder = bottomBorderHeight();
    QRect contentsRect = this->contentsRect().toRect();
    painter->translate(contentsRect.topLeft());
    painter->setRenderHint(QPainter::Antialiasing);

    painter->save();

    if (!m_items.isEmpty()) {
        QRegion clipRegion;
        clipRegion += contentsRect;
        clipRegion -= m_itemsRect;
        painter->setClipping(true);
        painter->setClipRegion(clipRegion);
    }

    m_panel->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    m_panel->resizeFrame(QSize(contentsRect.width(), s_topBorder + m_itemsRect.height() + bottomBorderHeight()));

    m_panel->paintFrame(painter);
    painter->restore();

    m_logo->paint(painter, QRect(10, 5, s_logoSize, s_logoSize));

    painter->save();
    QFont font = KGlobalSettings::smallestReadableFont();
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    painter->drawText(QRect(39, 3, contentsRect.width() - 39, 30), Qt::AlignBottom | Qt::AlignLeft,
                      ' ' + i18n("Previewer"));
    QFontMetrics fm(font);
    int length = fm.width(i18n("Previewer") + "  ");
    font.setBold(false);
    painter->setFont(KGlobalSettings::smallestReadableFont());
    painter->drawText(QRect(39 + length, 3, contentsRect.width() - 39, 30),
                      Qt::AlignBottom | Qt::AlignLeft,
                      i18n("Drop files on me to preview them."));
    painter->restore();

    drawOpenCloseArrow(painter);

    if (!m_items.isEmpty() && m_itemsRect.isValid()) {
        painter->save();
        painter->setClipping(true);
        painter->setClipRect(m_itemsRect);

        PreviewItemModel model(m_previewHistory);

        //TODO: this could look a lot nicer with a gradient
        painter->fillRect(m_itemsRect.intersected(opt->rect), QColor(0, 0, 0, 80));

        m_option.decorationSize = iconSize();
        for (int i = 0; i < m_items.count(); i++) {
            QRect r = m_items[i];

            if (!r.intersects(opt->rect)) {
                continue;
            }

            m_option.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver) ;
            if (m_selectedIndex == i) {
                m_option.state |= QStyle::State_Selected;
            }

            if (m_hoveredIndex == i) {
                m_option.state |= QStyle::State_MouseOver;
            }

            m_option.rect = r;
            m_delegate.setShadowColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
            m_delegate.paint(painter, m_option, model.index(i, 0));
        }

        painter->restore();

    }

    // hovering rect
    if (m_previewReady && !m_hoveredUrl.isEmpty()) {
        QPixmap pmap;
        if (m_previews.contains(m_hoveredUrl)) {
            pmap = m_previews[m_hoveredUrl];
            QRect itemNameRect = QRect(m_itemsRect.right(), m_itemsRect.bottom(),
                                       contentsRect.width() - m_itemsRect.right(), bottomBorder);
            // here we paint the file name
            painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
            painter->drawText(itemNameRect, Qt::AlignVCenter | Qt::AlignLeft, m_hoveredUrl.fileName());
        }

        if (m_previewRect.width() >= pmap.width() && m_previewRect.height() >= pmap.height()) {
            QRect p_rect = QRect(m_previewRect.x() + abs(m_previewRect.width() - pmap.width())/2,
                    m_previewRect.y() + abs(m_previewRect.height() - pmap.height())/2,
                    pmap.width(), pmap.height());
            painter->drawPixmap(p_rect, pmap);
        } else {
            QPixmap scaled = pmap.scaled(m_previewRect.size(), Qt::KeepAspectRatio);
            QRect p_rect = QRect(m_previewRect.x() + abs(m_previewRect.width() - scaled.width())/2,
                    m_previewRect.y() + abs(m_previewRect.height() - scaled.height())/2,
                    scaled.width(), scaled.height());

            painter->drawPixmap(p_rect, scaled);
        }
    }
}

void PreviewWidget::drawOpenCloseArrow(QPainter *painter)
{
    if (m_items.isEmpty()) {
        return;
    }

    bool up = !m_closeStatus;

    painter->save();
    QPen pen = painter->pen();
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setWidth(2);
    pen.setColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    painter->setPen(pen);

    QPainterPath path;

    if (up) {
        QPoint top = m_arrowRect.center();
        top.setY(m_arrowRect.top());
        path.moveTo(QPoint(m_arrowRect.x(), m_arrowRect.bottom() - 2));
        path.lineTo(top);
        path.lineTo(QPoint(m_arrowRect.right(), m_arrowRect.bottom() - 2));
        painter->drawPath(path);
    } else {
        QPoint bottom = m_arrowRect.center();
        bottom.setY(m_arrowRect.bottom());
        path.moveTo(QPoint(m_arrowRect.left(), m_arrowRect.top() + 2));
        path.lineTo(bottom);
        path.lineTo(QPoint(m_arrowRect.right(), m_arrowRect.top() + 2));
        painter->drawPath(path);
    }

    painter->restore();
}

int PreviewWidget::suggestedWidth()
{
    QFont font = KGlobalSettings::smallestReadableFont();
    font.setBold(true);

    QFontMetrics fm(font);

    const int length = fm.width(i18n("Previewer") + "  ");

    font.setBold(false);
    fm = QFontMetrics(font);

    return (39 + length + fm.width(i18n("Drop files on me to preview them.")) + 70);
}
