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
#include <QPropertyAnimation>

// KDE
#include <KDebug>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KIO/PreviewJob>
#include <KUrl>
#include <KIcon>

// Plasma
#include <Plasma/Animator>
#include <Plasma/FrameSvg>
#include <Plasma/ScrollBar>
#include <Plasma/Svg>
#include <Plasma/Theme>

static const int EXPANDING_DURATION = 200;
static const int REMOVE_EMBLEM_SIZE = 22;

PreviewWidget::PreviewWidget(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_selectedIndex(-1),
      m_hoveredIndex(-1),
      m_hoverSvg(new Plasma::FrameSvg(this)),
      m_closeStatus(true)
{
    m_scrollBar = new Plasma::ScrollBar(this);
    connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrolled(int)));
    m_scrollBar->setSingleStep(10);
    m_scrollBar->hide();

    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    //kDebug() << "setting dialog up";
    setupOptionViewItem();

    // let's draw the top border of the applet
    m_panel = new Plasma::FrameSvg(this);
    m_panel->setImagePath("widgets/background");

    m_logo = new Plasma::Svg(this);
    m_logo->setImagePath("widgets/previewer-16");

    m_hoverSvg->setImagePath("widgets/viewitem");
    m_hoverSvg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    m_hoverSvg->setCacheAllRenderedFrames(true);

    updateMargins();

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(setupOptionViewItem()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(updateMargins()));
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

    if (m_animation.isNull() || m_animation.data()->state() != QAbstractAnimation::Running) {
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
    if (m_previewHistory.isEmpty()) {
        return;
    }

    m_previews.clear();
    QList<KFileItem> f_items;
    for (int i = 0; i < m_previewHistory.count(); i++) {
        f_items << KFileItem(KFileItem::Unknown, KFileItem::Unknown, KUrl(m_previewHistory[i]));
    }

    KIO::PreviewJob *previewJob = new KIO::PreviewJob(f_items, QSize(256, 512));
    previewJob->setOverlayIconAlpha(0);
    previewJob->setScaleType(KIO::PreviewJob::Unscaled);
    connect(previewJob, SIGNAL(gotPreview(KFileItem,QPixmap)),
            this, SLOT(setPreview(KFileItem,QPixmap)));
}

void PreviewWidget::setPreview(const KFileItem &item, const QPixmap &pixmap)
{
    m_previews.insert(item.url(), pixmap);
    update();
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
            break;
        }
    }

    if (m_selectedIndex == -1) {
        return;
    }

    // here we check if the remove emblem was clicked
    QRect r = m_items[m_selectedIndex];
    r.setX(r.right() - REMOVE_EMBLEM_SIZE);
    r.setSize(QSize(REMOVE_EMBLEM_SIZE, REMOVE_EMBLEM_SIZE));
    if (r.contains(point)) {
        removeItem(m_selectedIndex);
        return;
    }

    emit fileOpenRequested(KUrl(m_previewHistory[m_selectedIndex]));

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

    // updating is unsafe when layout is broken
    if (!m_layoutIsValid) {
        return;
    }

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
                    update();
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
        } else if (m_arrowRect.contains(event->pos().toPoint())) {
            animateHeight(m_closeStatus);
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

    m_layoutIsValid = false;
    update();

    lookForPreview();
}

void PreviewWidget::addItem(const QUrl &url)
{
    m_previewHistory << url;
    m_layoutIsValid = false;
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
    QRect rect = this->rect().toRect();
    const int scrollBarWidth = m_scrollBar->isVisible() ? int(m_scrollBar->preferredSize().width()) : 0;
    const int itemWidth = iconSize().width() * 2;
    const int itemRectWidth = itemWidth + scrollBarWidth + (s_spacing * 2) +
                              (scrollBarWidth > 0 ?  2 : 0);
    const int bottomBorder = bottomBorderHeight();

    // items rect shouldn't be visible even if the status is close
    const bool isAnimating = m_animation.isNull() || m_animation.data()->state() != QAbstractAnimation::Running;
    if (m_previewHistory.isEmpty() || (m_closeStatus && isAnimating)) {
        m_animationHeight = s_topBorder + bottomBorder;
    } else if (isAnimating) {
        m_animationHeight = rect.height();
    }

    const int itemRectHeight = m_animationHeight - s_topBorder - bottomBorder;
    m_itemsRect = QRect(contentsRect().x(), s_topBorder, itemRectWidth, itemRectHeight);

    if (itemRectHeight) {
        QRect r = QRect(m_itemsRect.right() - scrollBarWidth + 2, s_topBorder + 1, scrollBarWidth,
                        m_animationHeight - s_topBorder - bottomBorder - 2);
        m_scrollBar->setGeometry(r);
    } else {
        m_scrollBar->setGeometry(QRect());
    }

    //kDebug() << m_animationHeight;
    m_arrowRect = QRect(contentsRect().right() - 10, (s_topBorder / 2), 10,10);

    m_previewRect = QRect(m_itemsRect.right(), m_itemsRect.y(),
                         rect.width() - m_itemsRect.right() - rect.x(),
                          m_itemsRect.height());

    m_layoutIsValid = false;
}

qreal PreviewWidget::animationValue() const
{
    return m_animationHeight;
}

void PreviewWidget::expandingSlot(qreal progress)
{
    const int min = s_topBorder + bottomBorderHeight();

    if (!m_closeStatus && !m_items.isEmpty() && (m_items.last().bottom() > m_itemsRect.bottom() ||
        m_items.first().top() < m_itemsRect.top())) {
        m_scrollBar->show();
    }

    m_animationHeight = qMax(qreal(min), min - 1 + ((size().height() - min)  * progress));

    calculateRects();
    update();
}

void PreviewWidget::animateHeight(bool isExpanding)
{
    if (m_closeStatus != isExpanding) {
        return;
    }

    m_closeStatus = !isExpanding;

    QPropertyAnimation *animation = m_animation.data();
    if (!animation) {
        animation = new QPropertyAnimation(this, "animationValue");
        animation->setDuration(EXPANDING_DURATION);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        m_animation = animation;
    } else if (animation->state() == QAbstractAnimation::Running) {
        animation->pause();
    }

    // don't draw the scrollbar while animating
    m_scrollBar->hide();

    if (isExpanding) {
        animation->setDirection(QAbstractAnimation::Forward);
        animation->start(QAbstractAnimation::KeepWhenStopped);
    } else {
        animation->setDirection(QAbstractAnimation::Backward);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
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
    QRect contentsRect = opt->rect;
//    painter->translate(contentsRect.topLeft());
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

    // FIXME: probably too much magic numbers
    painter->save();
    QFont font = KGlobalSettings::smallestReadableFont();
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    painter->drawText(QRect(39, 3, contentsRect.width() - 39, 30), Qt::AlignBottom | Qt::AlignLeft,
                      ' ' + i18n("Previewer"));
    int length = opt->fontMetrics.width(i18n("Previewer") + "  ");
    font.setBold(false);
    painter->setFont(KGlobalSettings::smallestReadableFont());
    QString elidedHint = opt->fontMetrics.elidedText(i18n("Drop files on me to preview them."), Qt::ElideRight, contentsRect.width() - 39 - length - 3);
    painter->drawText(QRect(39 + length + 3, 3, contentsRect.width() - 39 - length - 3, 30),
                      Qt::AlignBottom | Qt::AlignLeft,
                      elidedHint);
    painter->restore();

    drawOpenCloseArrow(painter);

    if (!m_items.isEmpty() && m_itemsRect.isValid()) {
        painter->save();
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
	    m_hoverSvg->setElementPrefix("");
            if (m_selectedIndex == i) {
                m_hoverSvg->prefix().isEmpty() ? m_hoverSvg->setElementPrefix("selected") : m_hoverSvg->setElementPrefix(m_hoverSvg->prefix() + "+selected");
            }

            if (m_hoveredIndex == i) {
                m_hoverSvg->prefix().isEmpty() ? m_hoverSvg->setElementPrefix("hover") : m_hoverSvg->setElementPrefix(m_hoverSvg->prefix() + "+hover");
            }
	    
	    if (!m_hoverSvg->prefix().isEmpty()) {
	        m_hoverSvg->resizeFrame(r.size());
                m_hoverSvg->paintFrame(painter, r.topLeft());
                KIcon("list-remove").paint(painter, r.right() - REMOVE_EMBLEM_SIZE, r.y(), REMOVE_EMBLEM_SIZE, REMOVE_EMBLEM_SIZE);
	    }

            m_option.rect = r;
            m_delegate.setShadowColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
            m_delegate.paint(painter, m_option, model.index(i, 0));
        }

        painter->restore();

    }

    // hovering rect
    if (!m_hoveredUrl.isEmpty() && m_previews.contains(m_hoveredUrl)) {
        QPixmap pmap;
        pmap = m_previews[m_hoveredUrl];

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

QList<QUrl> PreviewWidget::previews()
{
    return m_previewHistory;
}

void PreviewWidget::removeItem(int index)
{
    m_previewHistory.removeAt(index);
    m_selectedIndex = -1;
    m_hoveredIndex = -1;
    m_layoutIsValid = false;
    if (m_previewHistory.isEmpty()) {
        animateHeight(false);
    }
    lookForPreview();
    update();
}

void PreviewWidget::updateMargins()
{
    if (!m_panel) {
        return;
    }

    qreal left;
    qreal top;
    qreal right;
    qreal bottom;

    m_panel->getMargins(left, top, right, bottom);
    setContentsMargins(left, top, right, bottom);
}
