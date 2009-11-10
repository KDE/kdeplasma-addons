#include "topdevelopersview.h"

#include <QPen>
#include <QFontMetrics>

#include <KIcon>

#include "kdeobservatorydatabase.h"

TopDevelopersView::TopDevelopersView(const QMap<QString, KdeObservatory::Project> &projectsInView, const QRectF &rect, QGraphicsItem *parent, Qt::WindowFlags wFlags)
: IViewProvider(rect, parent, wFlags),
  m_projectsInView(projectsInView)
{
    QMap< QString, QMultiMap<int, QString> > topDevelopers;
    QMapIterator<QString, KdeObservatory::Project> i1(projectsInView);
    while (i1.hasNext())
    {
        i1.next();
        topDevelopers.insert(i1.key(), KdeObservatoryDatabase::self()->developersByProject(i1.value().commitSubject));
    }

    QMapIterator< QString, QMultiMap<int, QString> > i2(topDevelopers);
    while (i2.hasNext())
    {
        i2.next();

        QString project = i2.key();
        const QMultiMap<int, QString> &projectDevelopers = i2.value();

        QMapIterator<int, QString> i3(projectDevelopers);
        i3.toBack();

        QGraphicsWidget *container = createView(i18n("Top Commiters") + " - " + project);

        int maxRank = 0;
        qreal width = container->geometry().width();
        qreal step = qMax(container->geometry().height() / projectDevelopers.size(), (qreal) 22);

        int j = 0;
        while (i3.hasPrevious())
        {
            i3.previous();
            QString developer = i3.value();
            int rank = i3.key();
            if (j == 0)
                maxRank = rank;

            qreal widthFactor = (width-24)/maxRank;
            qreal yItem = (j*step)+2;

            QGraphicsRectItem *commiterRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
            commiterRect->setPos(0, yItem);
            commiterRect->setPen(QPen(QColor(0, 0, 0)));
            commiterRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));

//            QGraphicsPixmapItem *icon = new QGraphicsPixmapItem(KIcon(m_projects[resultMap.key(rank)].icon).pixmap(22, 22), container);
//            icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+((step-4)/2)-11);

            QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(QString::number(rank) + " - " + developer, commiterRect);
            commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
            QFontMetrics fontMetricsNumber(commitsNumber->font());
            commitsNumber->setPos((qreal) 0, (qreal) ((commiterRect->rect().height())/2)-(fontMetricsNumber.height()/2));
            j++;
        }
    }
}

TopDevelopersView::~TopDevelopersView()
{
}
