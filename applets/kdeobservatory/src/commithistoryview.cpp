#include "commithistoryview.h"

#include <QGraphicsProxyWidget>

#include <KGlobalSettings>

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_scale_widget.h>

#include "kdeobservatorydatabase.h"

class TimeScaleDraw : public QwtScaleDraw
{
public:
    virtual QwtText label(double v) const
    {
        return QString::number(v).insert(2, "/");
    }
};

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
        long long maxDate = QDate::fromString(projectCommits.at(projectCommits.count()-1).first, "yyyy-MM-dd").toString("MMdd").toInt();
        int minDate = QDate::fromString(projectCommits.at(0).first, "yyyy-MM-dd").toString("MMdd").toInt();
        double x[30];
        double y[30];

        int count = projectCommits.count();
        int j;
        for (j=0; j < count; ++j)
        {
            const QPair<QString, int> &pair = projectCommits.at(j);
            x[j] = QDate::fromString(pair.first, "yyyy-MM-dd").toString("MMdd").toInt();
            y[j] = pair.second;
            if (y[j] > maxCommit)
                maxCommit = y[j];
        }

        QGraphicsWidget *container = createView(i18n("Commit History") + " - " + project);
        QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(container);
        proxy->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

        QwtPlot *plot = new QwtPlot;
        plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        plot->setAxisScale(QwtPlot::yLeft, 0, qRound((maxCommit/5.)+0.5)*5, qRound((maxCommit/5.)+0.5));
        plot->setAxisScale(QwtPlot::xBottom, minDate, maxDate, 1);

        plot->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw);

        plot->setAxisFont(QwtPlot::yLeft, KGlobalSettings::smallestReadableFont());
        plot->setAxisFont(QwtPlot::xBottom, KGlobalSettings::smallestReadableFont());

        plot->setAxisLabelRotation(QwtPlot::xBottom, -15);

        plot->setCanvasBackground(QColor(0, 0, 140));

        QwtPlotCurve *curve = new QwtPlotCurve;
        curve->setData(x, y, j);

        curve->attach(plot);
        QPen pen = curve->pen();
        pen.setColor(QColor(255, 255, 0));
        curve->setPen(pen);
        plot->replot();

        proxy->setWidget(plot);
        plot->setGeometry(0, 0, container->geometry().width(), container->geometry().height());
    }
}
