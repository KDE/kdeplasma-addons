#include "topcommitersview.h"

#include <QPen>
#include <QFontMetrics>

#include <KIcon>

#include <Plasma/Frame>

#include "icollector.h"
#include "commitcollector.h"

TopCommitersView::TopCommitersView(ICollector *collector, const QRectF &rect, QGraphicsItem *parent, Qt::WindowFlags wFlags)
: IViewProvider(rect, parent, wFlags),
  m_collector(collector)
{
    const QHash<QString, QList< QPair<QString, int> > > &resultingCommiters = (qobject_cast<CommitCollector *>(m_collector))->resultingCommiters();

    QHashIterator<QString, QList< QPair<QString, int> > > i1(resultingCommiters);
    while (i1.hasNext())
    {
        i1.next();
        QString project = i1.key();
        const QList< QPair<QString, int> > &projectCommiters = i1.value();

        QListIterator< QPair<QString, int> > i2(projectCommiters);
        i2.toBack();

        QGraphicsWidget *container = createView(i18n("Top Commiters") + " - " + project);

        int maxRank = projectCommiters.last().second;
        qreal width = container->geometry().width();
        qreal step = qMax(container->geometry().height() / projectCommiters.count(), (qreal) 22);

        int j = 0;
        while (i2.hasPrevious() && j < 6)
        {
            const QPair<QString, int> &pair = i2.previous();
            QString commiter = pair.first;
            int rank = pair.second;

            qreal widthFactor = (width-24)/maxRank;
            qreal yItem = (j*step)+2;

            QGraphicsRectItem *commiterRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
            commiterRect->setPos(0, yItem);
            commiterRect->setPen(QPen(QColor(0, 0, 0)));
            commiterRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));

//            QGraphicsPixmapItem *icon = new QGraphicsPixmapItem(KIcon(m_projects[resultMap.key(rank)].icon).pixmap(22, 22), container);
//            icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+((step-4)/2)-11);

            QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(QString::number(rank) + " - " + commiter, commiterRect);
            commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
            QFontMetrics fontMetricsNumber(commitsNumber->font());
            commitsNumber->setPos((qreal) 0, (qreal) ((commiterRect->rect().height())/2)-(fontMetricsNumber.height()/2));
            j++;
        }
    }
}

TopCommitersView::~TopCommitersView()
{
}
