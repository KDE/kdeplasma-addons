/*************************************************************************
 * Copyright 2009-2010 Sandro Andrade sandroandrade@kde.org              *
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

class TimeScaleDraw : public QwtScaleDraw
{
public:
    TimeScaleDraw(const QDate &minDate)
    {
        m_minDate = minDate;
    }

    virtual QwtText label(double v) const
    {
        QString dateFormat = KGlobal::locale()->dateFormatShort();
        if (dateFormat == "%Y-%m-%d")
            return QwtText(m_minDate.addDays((int)v).toString("MM/dd"));
        else
            return QwtText(m_minDate.addDays((int)v).toString("dd/MM"));
    }

private:
    QDate m_minDate;
};

CommitHistoryView::CommitHistoryView(KdeObservatory *kdeObservatory, const QHash<QString, bool> &commitHistoryViewProjects, const QMap<QString, KdeObservatory::Project> &projects, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(kdeObservatory, parent, wFlags),
  m_commitHistoryViewProjects(commitHistoryViewProjects),
  m_projects(projects)
{
}

CommitHistoryView::~CommitHistoryView()
{
}

void CommitHistoryView::createViews()
{
    deleteViews();
    QHashIterator<QString, bool> i(m_commitHistoryViewProjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value())
        createView(i18nc("Commit history for a given project %1", "Commit History - %1", i.key()), QString("Commit History - ") + i.key());
    }
}

void CommitHistoryView::updateViews(const Plasma::DataEngine::Data &data)
{
    QString project = data["project"].toString();

    const DateCommitList &projectCommits = data[project].value<DateCommitList>();

    int count = projectCommits.count();
    if (count > 0)
    {
        int maxCommit = 0;

        QString tmpStr = projectCommits.at(0).first;
        qlonglong minDate = tmpStr.remove('-').toLongLong();
        double *x = new double[count];
        double *y = new double[count];

        int count = projectCommits.count();
        int j;
        for (j = 0; j < count; ++j)
        {
            const QPair<QString, int> &pair = projectCommits.at(j);
            tmpStr = pair.first;
            x[j] = j;
            y[j] = pair.second;
            if (y[j] > maxCommit)
                maxCommit = y[j];
        }

        QGraphicsWidget *container = containerForView(QString("Commit History - ") + project);
        if (!container) {
            delete []x;
            delete []y;
            return;
        }

        QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(container);
        proxy->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

        QwtPlot *plot = new QwtPlot;
        plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        plot->setAttribute(Qt::WA_TranslucentBackground, true);

        plot->setAxisScale(QwtPlot::yLeft, 0, qRound((maxCommit/5.)+0.5)*5, qRound((maxCommit/5.)+0.5));

        plot->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(QDate::fromString(QString::number(minDate), "yyyyMMdd")));

        plot->setAxisFont(QwtPlot::yLeft, KGlobalSettings::smallestReadableFont());
        plot->setAxisFont(QwtPlot::xBottom, KGlobalSettings::smallestReadableFont());

        plot->setAxisLabelRotation(QwtPlot::xBottom, -15);

        plot->setCanvasBackground(QColor(0, 0, 140));

        QwtPlotCurve *curve = new QwtPlotCurve;
#if QWT_VERSION >= (((6) << 16) | ((0) << 8) | (0))
        curve->setSamples(x, y, j);
#else
        curve->setData(x, y, j);
#endif
        delete []x;
        delete []y;

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
