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

#include "krazyreportview.h"

#include <QPen>

#include <KIcon>
#include <KGlobalSettings>

#include "kdeobservatorydatabase.h"

KrazyReportView::KrazyReportView(const QHash<QString, bool> &krazyReportViewProjects, const QMap<QString, KdeObservatory::Project> &projects, const Plasma::DataEngine::Data &data, QGraphicsWidget *parent, Qt::WindowFlags wFlags)
: IViewProvider(data, parent, wFlags),
  m_krazyReportViewProjects(krazyReportViewProjects),
  m_projects(projects)
{
}

KrazyReportView::~KrazyReportView()
{
}

void KrazyReportView::updateViews()
{
    deleteViews();

    QMap< QString, QMap<QString, QMultiMap<int, QString> > > krazyReports;
    QHashIterator<QString, bool> i1(m_krazyReportViewProjects);
    while (i1.hasNext())
    {
        i1.next();
        if (i1.value())
            krazyReports[i1.key()] = KdeObservatoryDatabase::self()->krazyErrorsByProject(i1.key());
    }

    KdeObservatory *kdeObservatory = dynamic_cast<KdeObservatory *>(m_parent->parentItem()->parentItem());

    QMapIterator< QString, QMap<QString, QMultiMap<int, QString> > > i2(krazyReports);
    while (i2.hasNext())
    {
        i2.next();

        QString project = i2.key();
        const QMap<QString, QMultiMap<int, QString> > &projectKrazyReport = i2.value();

        QMapIterator< QString, QMultiMap<int, QString> > i3(projectKrazyReport);

        while (i3.hasNext())
        {
            i3.next();

            QString fileType = i3.key();
            const QMultiMap<int, QString> &projectFileTypeKrazyReport = i3.value();

            QGraphicsWidget *container = createView(i18n("Krazy Report") + " - " + project);

            QGraphicsTextItem *fileTypeText = new QGraphicsTextItem(fileType, container);
            fileTypeText->setDefaultTextColor(QColor(0, 0, 0));
            fileTypeText->setFont(KGlobalSettings::smallestReadableFont());
            fileTypeText->setPos((qreal) ((container->rect().width())/2)-(fileTypeText->boundingRect().width()/2), (qreal) 0);

            int maxRank = 0;
            qreal height = container->geometry().height() - fileTypeText->boundingRect().height();
            qreal step = qMin(container->geometry().width() / projectFileTypeKrazyReport.size(), (qreal) 22);

            int j = 0;
            QMapIterator<int, QString> i4(projectFileTypeKrazyReport);
            i4.toBack();
            while (i4.hasPrevious())
            {
                i4.previous();
                QString testName = i4.value();
                int rank = i4.key();
                if (j == 0)
                    maxRank = rank;

                qreal heightFactor = height/maxRank;
                qreal xItem = (j*step)+2;

                QGraphicsRectItem *testNameRect = new QGraphicsRectItem(0, 0, (qreal) step-4, (qreal) heightFactor*rank, container);
                testNameRect->setPos(xItem, container->geometry().height() - testNameRect->rect().height());
                testNameRect->setPen(QPen(QColor(0, 0, 0)));
                testNameRect->setBrush(QBrush(QColor::fromHsv(qrand() % 256, 255, 190), Qt::SolidPattern));
                QString toolTip = "<html><body><h5>" + testName + ' ' + QString::number(rank) + ' ' + i18np("error", "errors", rank) + "<ul>";

                QStringList files = KdeObservatoryDatabase::self()->krazyFilesByProjectTypeAndTest(project, fileType, testName);
                foreach (const QString &fileName, files)
                    toolTip += "<li>" + fileName + "</li>";
                toolTip += "</ul></h5></body></html>";
                testNameRect->setToolTip(toolTip);
                testNameRect->setAcceptHoverEvents(true);
                testNameRect->installSceneEventFilter(kdeObservatory);

                j++;
            }
        }
    }
}
