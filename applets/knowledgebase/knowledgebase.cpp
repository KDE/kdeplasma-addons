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

#include "knowledgebase.h"

#include <cmath>

//Qt
#include <QGraphicsLinearLayout>
#include <QTimer>

//KDE
#include <KConfigDialog>
#include <KToolInvocation>
#include <KLineEdit>

//Plasma
#include <Plasma/LineEdit>
#include <Plasma/ScrollWidget>
#include <Plasma/Frame>
#include <Plasma/ToolButton>
#include <Plasma/Label>

//Own
#include "kbitemwidget.h"

KnowledgeBase::KnowledgeBase(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_graphicsWidget(0),
      m_provider("https://api.opendesktop.org/v1/"),
      m_currentPage(1),
      m_totalPages(1)
{
    setHasConfigurationInterface(true);
    setPopupIcon("help-contents");
}


KnowledgeBase::~KnowledgeBase()
{
}

void KnowledgeBase::init()
{
    graphicsWidget();
    m_searchTimeout = new QTimer(this);
    m_searchTimeout->setSingleShot(true);
    connect(m_searchTimeout, SIGNAL(timeout()), this, SLOT(doQuery()));
    delayedQuery();
    configChanged();
}

void KnowledgeBase::configChanged()    
{
    m_refreshTime = config().readEntry("refreshTime", 5);
    //setAssociatedApplicationUrls(KUrl("http://opendesktop.org/knowledgebase"));
}

QGraphicsWidget *KnowledgeBase::graphicsWidget()
{
    if (!m_graphicsWidget) {
        m_graphicsWidget = new QGraphicsWidget(this);
        m_graphicsWidget->setPreferredSize(300,400);

        QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(Qt::Vertical, m_graphicsWidget);
        m_questionInput = new Plasma::LineEdit(this);
        m_questionInput->setClearButtonShown(true);
        m_questionInput->nativeWidget()->setClickMessage(i18n("Search Knowledge Base"));
        lay->addItem(m_questionInput);
        connect(m_questionInput, SIGNAL(returnPressed()), this, SLOT(doQuery()));
        connect(m_questionInput, SIGNAL(textEdited(QString)), this, SLOT(delayedQuery()));

        m_KBItemsScroll= new Plasma::ScrollWidget(this);
        m_KBItemsScroll->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        m_KBItemsPage = new QGraphicsWidget(this);
        m_KBItemsLayout = new QGraphicsLinearLayout(Qt::Vertical, m_KBItemsPage);
        m_KBItemsScroll->setWidget(m_KBItemsPage);
        lay->addItem(m_KBItemsScroll);

        //Bottom "toolbar"
        m_bottomToolbar = new Plasma::Frame(this);
        m_bottomToolbar->setFrameShadow(Plasma::Frame::Raised);
        QGraphicsLinearLayout *bottomLayout = new QGraphicsLinearLayout(m_bottomToolbar);
        m_nextButton = new Plasma::ToolButton(this);
        m_nextButton->setText(i18nc("label for a button used to navigate to the next page", ">"));
        connect(m_nextButton, SIGNAL(clicked()), this, SLOT(nextPage()));
        m_prevButton = new Plasma::ToolButton(this);
        m_prevButton->setText(i18nc("label for a button used to navigate to the previous page", "<"));
        connect(m_prevButton, SIGNAL(clicked()), this, SLOT(prevPage()));
        m_statusLabel = new Plasma::Label(this);
        m_statusLabel->nativeWidget()->setWordWrap(false);
        m_statusLabel->nativeWidget()->setAlignment(Qt::AlignCenter);

        bottomLayout->addItem(m_prevButton);
        bottomLayout->addItem(m_statusLabel);
        bottomLayout->addItem(m_nextButton);

        m_bottomToolbar->hide();
    }

    return m_graphicsWidget;
}

void KnowledgeBase::clearResults()
{
    dataEngine("ocs")->disconnectSource(m_currentQuery, this);

    foreach (const QString& source, m_sources) {
        dataEngine("ocs")->disconnectSource(source, this);
    }
    m_sources.clear();

    m_kbItemsByUser.clear();
    int n = m_KBItemsLayout->count();

    for (int i = 0; i < n; i++) {
        KBItemWidget *kw = static_cast<KBItemWidget*>(m_KBItemsLayout->itemAt(i));
        kw->deleteLater();
    }

    QGraphicsLinearLayout *lay = static_cast<QGraphicsLinearLayout *>(m_graphicsWidget->layout());
    lay->removeItem(m_bottomToolbar);
    m_bottomToolbar->hide();
}

void KnowledgeBase::doQuery()
{
    m_searchTimeout->stop();
    setBusy(true);
    clearResults();

    dataEngine("ocs")->disconnectSource( m_currentQuery, this );

    m_currentQuery = QString("KnowledgeBaseList\\provider:%1\\query:%2\\sortMode:new\\page:%3\\pageSize:10")
        .arg(m_provider)
        .arg(m_questionInput->text())
        .arg(m_currentPage-1);

    //if is null refresh periodically
    if (m_questionInput->text().isNull()) {
        //TODO: config UI
        //FIXME: order by date
        dataEngine("ocs")->connectSource(m_currentQuery, this, m_refreshTime * 60 * 1000);
    } else {
        dataEngine("ocs")->connectSource(m_currentQuery, this);
    }
}

void KnowledgeBase::nextPage()
{
    if (m_currentPage < m_totalPages) {
        ++m_currentPage;
        doQuery();
        if (m_currentPage >= m_totalPages) {
            m_nextButton->setEnabled(false);
        }

        if (m_currentPage > 1) {
            m_prevButton->setEnabled(true);
        }
    }
}

void KnowledgeBase::prevPage()
{
    if (m_currentPage > 0) {
        --m_currentPage;
        doQuery();
        if (m_currentPage <= 0) {
            m_prevButton->setEnabled(false);
        }

        if (m_currentPage < m_totalPages) {
            m_nextButton->setEnabled(true);
        }
    }
}

void KnowledgeBase::delayedQuery()
{
    m_currentPage = 1;
    m_totalPages = 1;

    m_searchTimeout->start(1000);
}



void KnowledgeBase::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    setBusy(false);

    if (source.startsWith(QLatin1String("KnowledgeBaseList\\"))) {
        m_totalItems = data["TotalItems"].toInt();
        m_totalPages = ceil((qreal)m_totalItems/m_itemsPerPage);

        QGraphicsLinearLayout *lay = static_cast<QGraphicsLinearLayout *>(m_graphicsWidget->layout());

        if (m_totalItems > 0) {
            lay->addItem(m_bottomToolbar);
            m_bottomToolbar->show();
        } else {
            lay->removeItem(m_bottomToolbar);
            m_bottomToolbar->hide();
        }

        m_statusLabel->setText(i18np("one item, page %2/%3", "%1 items, page %2/%3", m_totalItems, m_currentPage, m_totalPages));

        m_prevButton->setEnabled(m_currentPage > 1);
        m_nextButton->setEnabled(m_currentPage < m_totalPages);

        //unfortunately keys QHash aren't ordered
        QStringList keys = data.keys();
        keys.sort();
        foreach (const QString &kb, keys) {
            if (kb.startsWith(QLatin1String("KnowledgeBase-"))) {
                Plasma::DataEngine::Data kbData = data[kb].value<Plasma::DataEngine::Data>();

                KBItemWidget *kbItem = new KBItemWidget(m_KBItemsPage);
                connect(kbItem, SIGNAL(detailsVisibilityChanged(KBItemWidget*,bool)), this, SLOT(detailsClicked(KBItemWidget*,bool)));

                kbItem->setAtticaData(kbData);
                //we want inverted order
                m_KBItemsLayout->insertItem(0, kbItem);
                QString user = kbData["User"].toString();

                if (!m_kbItemsByUser.contains(user)) {
                    m_kbItemsByUser[user] = QList<KBItemWidget *>();
                }

                m_kbItemsByUser[user].append(kbItem);
                dataEngine("ocs")->connectSource("Person\\provider:" + m_provider + "\\id:" + user, this);
                m_sources.append("Person-"+user);
            }
        }

    } else if (source.startsWith(QLatin1String("Person\\"))) {
        Plasma::DataEngine::Data personData = data[source].value<Plasma::DataEngine::Data>();
        QList<KBItemWidget *> items = m_kbItemsByUser[personData["Id"].toString()];

        foreach (KBItemWidget *kbItem, items) {
            kbItem->setPixmap(personData["Avatar"].value<QPixmap>());
        }
    }

    m_graphicsWidget->setPreferredSize(-1,-1);
    m_KBItemsLayout->invalidate();

    emit sizeHintChanged(Qt::PreferredSize);
}

void KnowledgeBase::detailsClicked(KBItemWidget *item, bool shown)
{
    Q_UNUSED(shown)

    m_KBItemsLayout->invalidate();
    m_KBItemsPage->resize(QSizeF(m_KBItemsPage->size().width(), m_KBItemsPage->effectiveSizeHint(Qt::PreferredSize).height()));
    //m_KBItemsScroll->ensureItemVisible(item);

    emit sizeHintChanged(Qt::PreferredSize);
}


void KnowledgeBase::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18nc("General settings for the applet", "General"), Applet::icon());

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(ui.registerButton, SIGNAL(clicked()), this, SLOT(registerAccount()));

    ui.refreshTime->setValue(m_refreshTime);
    ui.refreshTime->setSuffix(ki18np(" minute", " minutes"));

    connect(ui.refreshTime, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
}

void KnowledgeBase::configAccepted()
{
    KConfigGroup cg = config();

    m_refreshTime = ui.refreshTime->value();
    cg.writeEntry("refreshTime", m_refreshTime);

    emit configNeedsSaving();
}

void KnowledgeBase::registerAccount()
{
    KToolInvocation::invokeBrowser("http://www.opendesktop.org/usermanager/new.php");
}

#include "knowledgebase.moc"
