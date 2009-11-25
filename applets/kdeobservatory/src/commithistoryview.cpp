/*************************************************************************
 * Copyright 2009 Sandro Andrade sandroandrade@kde.org                   *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#include "commithistoryview.h"

#include <QGraphicsProxyWidget>

#include <KGlobalSettings>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>

#include "kdeobservatorydatabase.h"

class TimeScaleDraw : public QwtScaleDraw
{
public:
    virtual QwtText label(double v) const
    {
        return QwtText(QString::number(v).insert(2, "/"));
    }
};

CommitHistoryView::CommitHistoryView(const QHash<QString, bool> &commitHistoryViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(parent, wFlags),
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

        if (projectCommits.count() > 0)
        {
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
            plot->setAttribute(Qt::WA_TranslucentBackground, true);

            plot->setAxisScale(QwtPlot::yLeft, 0, qRound((maxCommit/5.)+0.5)*5, qRound((maxCommit/5.)+0.5));
            plot->setAxisScale(QwtPlot::xBottom, minDate, maxDate, qRound(((maxDate-minDate)/7)+0.5));

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

            QwtPlotGrid *grid = new QwtPlotGrid;
            grid->enableXMin(true);
            grid->setMajPen(QPen(Qt::white, 0, Qt::DotLine));
            grid->setMinPen(QPen(Qt::NoPen));
            grid->attach(plot);

            proxy->setWidget(plot);
            plot->setGeometry(0, 0, container->geometry().width(), container->geometry().height());
        }
    }
}
