#include "topactiveprojectsview.h"

#include <QPen>
#include <QFontMetrics>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

#include <KIcon>

#include <Plasma/Frame>

#include "icollector.h"
#include "commitcollector.h"

TopActiveProjectsView::TopActiveProjectsView(const QMap<QString, KdeObservatory::Project> &projectsInView, ICollector *collector, const QRectF &rect, QGraphicsItem *parent, Qt::WindowFlags wFlags)
: IViewProvider(rect, parent, wFlags),
  m_projectsInView(projectsInView),
  m_collector(collector)
{
    const QMap<QString, int> &resultMapCommits = (qobject_cast<CommitCollector *>(m_collector))->resultMapCommits();
    QList<int> projects = resultMapCommits.values();

    qSort(projects);

    QListIterator<int> i(projects);
    i.toBack();

    QGraphicsWidget *container = createView(i18n("Top Active Projects"));

    int maxRank = projects.last();
    qreal width = container->geometry().width();
    qreal step = qMax(container->geometry().height() / projects.count(), (qreal) 22);

    int j = 0;
    while (i.hasPrevious())
    {
        int rank = i.previous();
        qreal widthFactor = (width-24)/maxRank;
        qreal yItem = (j*step)+2;

        QGraphicsRectItem *projectRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
        projectRect->setPos(0, yItem);
        projectRect->setPen(QPen(QColor(0, 0, 0)));
        projectRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));

        QGraphicsPixmapItem *icon = new QGraphicsPixmapItem(KIcon(m_projectsInView[resultMapCommits.key(rank)].icon).pixmap(22, 22), container);
        icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+((step-4)/2)-11);

        QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(QString::number(rank), projectRect);
        commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
        QFontMetrics fontMetricsNumber(commitsNumber->font());
        commitsNumber->setPos((qreal) ((projectRect->rect().width())/2)-(fontMetricsNumber.width(commitsNumber->toPlainText())/2),
                           (qreal) ((projectRect->rect().height())/2)-(fontMetricsNumber.height()/2));
        j++;
    }
}

TopActiveProjectsView::~TopActiveProjectsView()
{
}
