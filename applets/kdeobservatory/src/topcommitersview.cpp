#include "topcommitersview.h"

#include <QPen>
#include <QFontMetrics>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

#include <KIcon>

#include <Plasma/Frame>

#include "icollector.h"
#include "commitcollector.h"

TopCommitersView::TopCommitersView(ICollector *collector, const QRectF &rect, QGraphicsItem *parent, Qt::WindowFlags wFlags)
: IViewProvider(rect, parent, wFlags),
  m_collector(collector)
{
    const QMap<QString, QMap<QString, int> > &resultMapCommiters = (qobject_cast<CommitCollector *>(m_collector))->resultMapCommiters();
    const QList<QString> &projects = resultMapCommiters.keys();

    foreach (const QString &project, projects)
    {
        const QMap<QString, int> &projectCommiters = resultMapCommiters[project];
        QList<int> commits = projectCommiters.values();
        qSort(commits);

        QListIterator<int> i(commits);
        i.toBack();

        QGraphicsWidget *container = createView(i18n("Top Commiters") + " - " + project);

        int maxRank = commits.last();
        qreal width = container->geometry().width();
        qreal step = qMax(container->geometry().height() / commits.count(), (qreal) 22);

        int j = 0;
        while (i.hasPrevious())
        {
            int rank = i.previous();
            qreal widthFactor = (width-24)/maxRank;
            qreal yItem = (j*step)+2;

            QGraphicsRectItem *commiterRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
            commiterRect->setPos(0, yItem);
            commiterRect->setPen(QPen(QColor(0, 0, 0)));
            commiterRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));

//            QGraphicsPixmapItem *icon = new QGraphicsPixmapItem(KIcon(m_projects[resultMap.key(rank)].icon).pixmap(22, 22), container);
//            icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+((step-4)/2)-11);

            QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(QString::number(rank), commiterRect);
            commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
            QFontMetrics fontMetricsNumber(commitsNumber->font());
            commitsNumber->setPos((qreal) ((commiterRect->rect().width())/2)-(fontMetricsNumber.width(commitsNumber->toPlainText())/2),
                               (qreal) ((commiterRect->rect().height())/2)-(fontMetricsNumber.height()/2));
            j++;
        }
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
