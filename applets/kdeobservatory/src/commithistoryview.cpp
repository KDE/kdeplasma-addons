#include "commithistoryview.h"

#include <QGraphicsProxyWidget>

#include <KGlobalSettings>

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_scale_widget.h>

#include "kdeobservatorydatabase.h"

CommitHistoryView::CommitHistoryView(const QHash<QString, bool> &commitHistoryViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QRectF rect, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(rect, parent, wFlags),
  m_commitHistoryViewProjects(commitHistoryViewProjects),
  m_projects(projects)
{
}

CommitHistoryView::~CommitHistoryView()
{
}

void CommitHistoryView::updateViews()
{
    deleteViews();

    QMap< QString, QList< QPair<QString, int> > > commitHistory;
    QHashIterator<QString, bool> i1(m_commitHistoryViewProjects);
    while (i1.hasNext())
    {
        i1.next();
        if (i1.value())
            commitHistory.insert(i1.key(), KdeObservatoryDatabase::self()->commitHistory(m_projects[i1.key()].commitSubject));
    }

    QMapIterator< QString, QList< QPair<QString, int> > > i2(commitHistory);
    while (i2.hasNext())
    {
        i2.next();

        QString project = i2.key();
        const QList< QPair<QString, int> > &projectCommits = i2.value();

        int maxCommit = 0;
        long long maxDate = 0;
        int minDate = INT_MAX;
        double x[30];
        double y[30];

        int count = projectCommits.count();
        int j;
        qDebug() << "Commits for" << project;
        for (j=0; j < count; ++j)
        {
            const QPair<QString, int> &pair = projectCommits.at(j);
            qDebug() << pair.first << QDate::fromString(pair.first, "yyyy-MM-dd").toString("MMdd");
            x[j] = QDate::fromString(pair.first, "yyyy-MM-dd").toString("MMdd").toInt();
            y[j] = pair.second;
            if (x[j] > maxDate)
                maxDate = x[j];
            if (x[j] < minDate)
                minDate = x[j];
            if (y[j] > maxCommit)
                maxCommit = y[j];
        }

        QGraphicsWidget *container = createView(i18n("Commit History") + " - " + project);
        QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(container);
        proxy->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

        QwtPlot *plot = new QwtPlot;
        plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        plot->setAxisScale(0, 0, maxCommit, maxCommit/5.);
        plot->setAxisScale(2, minDate, maxDate, 1);
        plot->setAxisFont(0, KGlobalSettings::smallestReadableFont());
        plot->setAxisFont(2, KGlobalSettings::smallestReadableFont());
        plot->setAxisLabelRotation(2, -15);
        plot->axisWidget(0)->hide();
        plot->axisWidget(2)->hide();
        plot->setCanvasBackground(QColor(0, 0, 140));

        QwtPlotCurve *curve = new QwtPlotCurve;
        curve->setData(x, y, j);
        curve->attach(plot);
        QPen pen = curve->pen();
        pen.setColor(QColor(255, 255, 0));
        curve->setPen(pen);
        plot->replot();

        proxy->setWidget(plot);
//        proxy->setGeometry(container->geometry());
        plot->setGeometry(0, 0, container->geometry().width(), container->geometry().height());
//        proxy->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
//        container->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    }
}
