#include "topcommitersview.h"

#include <QPen>
#include <QFontMetrics>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

#include <KIcon>

#include <Plasma/Frame>

#include "icollector.h"
#include "commitcollector.h"

TopCommitersView::TopCommitersView(const QMap<QString, KdeObservatory::Project> &projects, ICollector *collector, const QRectF &rect, QGraphicsItem *parent, Qt::WindowFlags wFlags)
: IView(rect, parent, wFlags), m_projects(projects), m_collector(collector)
{
    const QMap<QString, int> &resultMap = (qobject_cast<CommitCollector *>(m_collector))->resultMapCommits();
    QList<int> list = resultMap.values();

    qSort(list);

    QListIterator<int> i(list);
    i.toBack();

    QGraphicsWidget *container = createView(i18n("Top Commiters"));

    int maxRank = list.last();
    qreal width = container->geometry().width();
    qreal step = qMax(container->geometry().height() / list.count(), (qreal) 22);

    int j = 0;
    while (i.hasPrevious())
    {
        int rank = i.previous();
        qreal widthFactor = (width-24)/maxRank;
        qreal yItem = (j*step)+2;

        QGraphicsRectItem *commits = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
        commits->setPos(0, yItem);
        commits->setPen(QPen(QColor(0, 0, 0)));
        commits->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));

        QGraphicsPixmapItem *icon = new QGraphicsPixmapItem(KIcon(m_projects[resultMap.key(rank)].icon).pixmap(22, 22), container);
        icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+((step-4)/2)-11);

        QGraphicsTextItem *textNumber = new QGraphicsTextItem(QString::number(rank), commits);
        textNumber->setDefaultTextColor(QColor(255, 255, 255));
        QFontMetrics fontMetricsNumber(textNumber->font());
        textNumber->setPos((qreal) ((commits->rect().width())/2)-(fontMetricsNumber.width(textNumber->toPlainText())/2),
                           (qreal) ((commits->rect().height())/2)-(fontMetricsNumber.height()/2));
        j++;
    }

    QTimeLine *timer = new QTimeLine(500);
    timer->setFrameRange(0, 1);
    timer->setCurveShape(QTimeLine::EaseOutCurve);

    QGraphicsItemAnimation *animation = new QGraphicsItemAnimation;
    animation->setItem(m_views[0]);
    animation->setTimeLine(timer);

    animation->setPosAt(0, QPointF(rect.x() + rect.width(), rect.y()));
    animation->setPosAt(1, QPointF(rect.x(), rect.y()));

    timer->start();
}

TopCommitersView::~TopCommitersView()
{
}
