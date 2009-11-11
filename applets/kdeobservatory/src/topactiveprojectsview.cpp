#include "topactiveprojectsview.h"

#include <QPen>
#include <QFontMetrics>

#include <KIcon>
#include <KGlobalSettings>

#include "kdeobservatorydatabase.h"

TopActiveProjectsView::TopActiveProjectsView(const QHash<QString, bool> &topActiveProjectsViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QRectF rect, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(rect, parent, wFlags),
  m_topActiveProjectsViewProjects(topActiveProjectsViewProjects),
  m_projects(projects)
{
}

TopActiveProjectsView::~TopActiveProjectsView()
{
}

void TopActiveProjectsView::updateViews()
{
    deleteViews();

    QMultiMap<int, QString> topActiveProjects;
    QHashIterator<QString, bool> i1(m_topActiveProjectsViewProjects);
    while (i1.hasNext())
    {
        i1.next();
        if (i1.value())
        {
            int rank = KdeObservatoryDatabase::self()->commitsByProject(m_projects[i1.key()].commitSubject);
            topActiveProjects.insert(rank, i1.key());
        }
    }

    QGraphicsWidget *container = createView(i18n("Top Active Projects"));

    int maxRank = 0;
    qreal width = container->geometry().width();
    qreal step = qMax(container->geometry().height() / topActiveProjects.size(), (qreal) 22);

    QMapIterator<int, QString> i2(topActiveProjects);
    i2.toBack();
    int j = 0;
    while (i2.hasPrevious())
    {
        i2.previous();
        QString project = i2.value();
        int rank = i2.key();
        if (j == 0)
            maxRank = rank;

        qreal widthFactor = (width-24)/maxRank;
        qreal yItem = (j*step)+2;

        QGraphicsRectItem *projectRect = new QGraphicsRectItem(0, 0, (qreal) widthFactor*rank, (qreal) step-4, container);
        projectRect->setPos(0, yItem);
        projectRect->setPen(QPen(QColor(0, 0, 0)));
        projectRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));

        QGraphicsPixmapItem *icon = new QGraphicsPixmapItem(KIcon(m_projects[project].icon).pixmap(22, 22), container);
        icon->setPos((qreal) widthFactor*rank+2, (qreal) yItem+((step-4)/2)-11);

        QGraphicsTextItem *commitsNumber = new QGraphicsTextItem(QString::number(rank), projectRect);
        commitsNumber->setDefaultTextColor(QColor(255, 255, 255));
        commitsNumber->setFont(KGlobalSettings::smallestReadableFont());
        QFontMetrics fontMetricsNumber(commitsNumber->font());
        commitsNumber->setPos((qreal) ((projectRect->rect().width())/2)-(fontMetricsNumber.width(commitsNumber->toPlainText())/2),
                              (qreal) ((projectRect->rect().height())/2)-(fontMetricsNumber.height()/2));
        j++;
    }
}
