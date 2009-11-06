#include "topactiveprojectsview.h"

#include <QPen>
#include <QFontMetrics>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

#include <KIcon>

#include <Plasma/Frame>

#include "icollector.h"
#include "commitcollector.h"

TopActiveProjectsView::TopActiveProjectsView(const QMap<QString, KdeObservatory::Project> &projects, ICollector *collector, const QString &title, const QRectF &rect, QGraphicsItem *parent, Qt::WindowFlags wFlags)
: IView(i18n("Top Active Projects"), rect, parent, wFlags), m_projects(projects), m_collector(collector)
{
    const QMap<QString, int> &resultMap = (qobject_cast<CommitCollector *>(m_collector))->resultMap();
    QList<int> list = resultMap.values();

    qSort(list);

    QListIterator<int> i(list);
    i.toBack();

    int maxRank = list.last();
    qreal width = m_container->geometry().width();
    //qreal step = qMin(m_container->geometry().height() / list.count(), m_header->geometry().height());
    qreal step = qMin(m_container->geometry().height() / list.count(), (qreal) 22);

    int j = 0;
    QGraphicsRectItem *commits;
    QGraphicsPixmapItem *icon;
    while (i.hasPrevious())
    {
        int rank = i.previous();
        qreal widthFactor = (width-34)/maxRank;
        qreal yItem = (j*step)+2;

        commits = new QGraphicsRectItem((qreal) 0,
                                     (qreal) yItem,
                                     (qreal) widthFactor*rank,
                                     (qreal) step-4, m_container);
        commits->setPen(QPen(QColor(0, 0, 0)));
        commits->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));

        icon = new QGraphicsPixmapItem(KIcon(m_projects[resultMap.key(rank)].icon).pixmap(22, 22), m_container);
        icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+((step-4)/2)-11);

        QGraphicsTextItem *textNumber = new QGraphicsTextItem(QString::number(rank), m_container);
        //textNumber->setFont(QFont("Times", 10, QFont::Bold));
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
/*
    QTimeLine *timer = new QTimeLine(1000);
    timer->setFrameRange(0, 1);
    timer->setCurveShape(QTimeLine::EaseOutCurve);

    QGraphicsItemAnimation *animation = new QGraphicsItemAnimation;
    animation->setItem(this);
    animation->setTimeLine(timer);

    animation->setPosAt(0, QPointF(rect.x() + rect.width(), rect.y()));
    animation->setPosAt(1, QPointF(rect.x(), rect.y()));

    timer->start();
*/
}

TopActiveProjectsView::~TopActiveProjectsView()
{
}
