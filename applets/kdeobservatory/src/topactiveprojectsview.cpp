#include "topactiveprojectsview.h"

#include <QPen>
#include <QFontMetrics>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

#include <KIcon>

#include "icollector.h"
#include "commitcollector.h"

TopActiveProjectsView::TopActiveProjectsView(const QMap<QString, KdeObservatory::Project> &projects, ICollector *collector, const QRectF &rect, QGraphicsItem *parent)
: QGraphicsRectItem(rect, parent), m_projects(projects), m_collector(collector)
{
    setPos(rect.width(), 0);
    setPen(QPen(Qt::NoPen));

    QGraphicsSimpleTextItem *simpleTextItem = new QGraphicsSimpleTextItem("Top Active Projects", this);
    simpleTextItem->setFont(QFont("Times", 12, QFont::Bold));
    QFontMetrics fontMetrics(simpleTextItem->font());
    simpleTextItem->setPos((rect.width()/2)-(fontMetrics.width("Top Active Projects")/2), 0);

    const QMap<QString, int> &resultMap = (qobject_cast<CommitCollector *>(m_collector))->resultMap();
    QList<int> list = resultMap.values();

    qSort(list);

    QListIterator<int> i(list);
    i.toBack();

    int maxRank = list.last();
    int y = fontMetrics.height()+10;
    qreal width = rect.width();
    qreal step = (rect.height()-y) / list.count();

    int j = 0;
    QGraphicsRectItem *commits;
    QGraphicsPixmapItem *icon;
    while (i.hasPrevious())
    {
        int rank = i.previous();
        qreal widthFactor = (width-34)/maxRank;
        qreal yItem = y+(j*step)+2;

        commits = new QGraphicsRectItem((qreal) 0,
                                     (qreal) yItem,
                                     (qreal) widthFactor*rank,
                                     (qreal) step-4, this);
        commits->setPen(QPen(QColor(0, 0, 0)));
        commits->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));

        icon = new QGraphicsPixmapItem(KIcon(m_projects[resultMap.key(rank)].icon).pixmap(22, 22), this);
        icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+(((step-4)/2)-11));

        QGraphicsTextItem *textNumber = new QGraphicsTextItem(QString::number(rank), this);
        textNumber->setFont(QFont("Times", 10, QFont::Bold));
        textNumber->setDefaultTextColor(QColor(255, 255, 255));
        textNumber->setZValue(1);
        QFontMetrics fontMetricsNumber(textNumber->font());
        textNumber->setPos((qreal) ((widthFactor*rank)/2)-(fontMetricsNumber.width(textNumber->toPlainText())/2),
                           (qreal) yItem+((step-4)/2)-(fontMetricsNumber.height()/2));

//        QGraphicsTextItem *textItem = m_scene->addText(resultMap.key(rank));
//        QFontMetrics fontMetrics(textItem->font());
//        textItem->setPos((qreal) (68*j)+30-(fontMetrics.width(textItem->toPlainText())/2), (qreal) -(rank*2)-74-(fontMetrics.height()));
        j++;
    }

    QTimeLine *timer = new QTimeLine(1000);
    timer->setFrameRange(0, 1);
    timer->setCurveShape(QTimeLine::EaseOutCurve);

    QGraphicsItemAnimation *animation = new QGraphicsItemAnimation;
    animation->setItem(parent);
    animation->setTimeLine(timer);

    animation->setPosAt(0, QPointF(rect.width(), 0));
    animation->setPosAt(1, QPointF(0, 0));

    timer->start();

}

TopActiveProjectsView::~TopActiveProjectsView()
{
}
