/***************************************************************************
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KNOWLEDGEBASE_HEADER
#define KNOWLEDGEBASE_HEADER


#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

#include "ui_knowledgeBaseConfig.h"

class QGraphicsLinearLayout;
class QTimer;

namespace Plasma
{
    class LineEdit;
    class ScrollWidget;
    class Frame;
    class ToolButton;
    class Label;
}

class KBItemWidget;

class KnowledgeBase : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    // Basic Create/Destroy
    KnowledgeBase(QObject *parent, const QVariantList &args);
    ~KnowledgeBase();

    void init();
    QGraphicsWidget *graphicsWidget();

public slots:
    void configChanged();

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void configAccepted();

protected:
    void clearResults();
    void createConfigurationInterface(KConfigDialog *parent);

private Q_SLOTS:
    void doQuery();
    void delayedQuery();
    void detailsClicked(KBItemWidget *item, bool shown);
    void nextPage();
    void prevPage();
    void registerAccount();

private:
    QString m_currentQuery;
    QStringList m_sources;
    QGraphicsWidget *m_graphicsWidget;
    Plasma::LineEdit *m_questionInput;
    Plasma::ScrollWidget *m_KBItemsScroll;
    QGraphicsWidget *m_KBItemsPage;
    QGraphicsLinearLayout *m_KBItemsLayout;
    Plasma::Frame *m_bottomToolbar;
    Plasma::ToolButton *m_prevButton;
    Plasma::ToolButton *m_nextButton;
    Plasma::Label *m_statusLabel;
    QString m_provider;

    int m_currentPage;
    int m_totalPages;
    int m_totalItems;
    static const int m_itemsPerPage = 10;

    int m_refreshTime;

    QTimer *m_searchTimeout;
    QHash<QString, QList<KBItemWidget *> > m_kbItemsByUser;

    // Configuration dialog
    Ui::knowledgeBaseConfig ui;
};

K_EXPORT_PLASMA_APPLET(knowledgebase, KnowledgeBase)
#endif
