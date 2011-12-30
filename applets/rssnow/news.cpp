/*
 * Copyright 2007, 2008  Petri Damsten <damu@iki.fi>
 * Copyright 2008  Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// This applet is based on the news applet from petri damsten.

//Own
#include "news.h"
#include "scroller.h"
#include "header.h"

//QT
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsLinearLayout>
#include <QTimer>
#include <QDomDocument>
#include <QFile>
#include <QLineEdit>

//KDE
#include <kdebug.h>
#include <kglobal.h>
#include <kicon.h>
#include <kconfigdialog.h>
#include <KStandardDirs>

//Plasma
#include <plasma/containment.h>

News::News(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_collapsed(false)
{
    setHasConfigurationInterface(true);
    setBackgroundHints(DefaultBackground);
    setAcceptDrops(true);
    setAspectRatioMode( Plasma::IgnoreAspectRatio );

    resize(300,300);
}

News::~News()
{
}

void News::init()
{
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_layout->setSpacing(2);
    setLayout(m_layout);

    m_header = new Header(this);
    m_timer = new QTimer(this);

    setAssociatedApplication("akregator");

    configChanged();

    connect(m_timer, SIGNAL(timeout()), this, SLOT(switchItems()));
}

void News::configChanged()
{
    KConfigGroup cg = config();

    m_interval       = cg.readEntry("interval", 30);
    m_switchInterval = cg.readEntry("switchInterval", 10);
    m_logo           = cg.readEntry("logo", true);
    m_maxAge         = cg.readEntry("maxAge", 0);
    m_animations     = cg.readEntry("animations", true);
    m_showdroptarget = cg.readEntry("droptarget", true);
    m_feedlist       = cg.readEntry("feeds", QStringList("http://dot.kde.org/rss.xml"));

    updateScrollers();
}

void News::switchItems()
{
    int delay = 1;
    int start = 0;
    if (m_logo) {
        start = 1;
    }
    for (int i = start; i < m_layout->count(); i++ ){
        Scroller * scroller = (Scroller *) m_layout->itemAt(i);
        if (!scroller->hovering()) {
            scroller->delayedMoveNext(delay++ * 100);
        }
    }
}

void News::connectToEngine()
{
    int i = 0;

    if (!m_feedlist.isEmpty()) {
        Plasma::DataEngine* engine = dataEngine("rss");

        int maxScroller = m_scrollerList.size();
        if (m_showdroptarget) {
            maxScroller--;
        }
        foreach (Scroller * scroller, m_scrollerList) {
            if (i < maxScroller) {
                kDebug() << "Connect to source: " << m_feedlist.at(i);
                engine->connectSource(m_feedlist.at(i++), scroller,
                                      m_interval * 60 * 1000);
            }
        }
    }
}

void News::createConfigurationInterface(KConfigDialog *parent)
{
    //TODO: we probably want to disable the add button until there's actually
    //an url in the combobox.
    QWidget *widget = new QWidget(0);
    ui.setupUi(widget);
    QWidget *fWidget = new QWidget(0);
    feedsUi.setupUi(fWidget);
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(feedsUi.addFeed, SIGNAL(clicked()), this, SLOT(addFeed()));
    connect(feedsUi.removeFeed, SIGNAL(clicked()), this, SLOT(removeFeed()));
    connect(feedsUi.feedList, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemChanged()));
    connect(feedsUi.feedComboBox->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(slotChangeText(QString)) );
    feedsUi.removeFeed->setEnabled( false );
    KConfig feedsFile(QString("news/feeds"), KConfig::FullConfig, "data");
    m_defaultFeeds = feedsFile.group("feeds").entryMap();
    feedsUi.addFeed->setEnabled( false );
    feedsUi.feedComboBox->clear();
    feedsUi.feedList->clear();
    foreach (const QString& name, m_defaultFeeds.keys()) {
        feedsUi.feedComboBox->addItem(name);
    }

    foreach (const QString& name, akregatorFeeds()) {
        feedsUi.feedComboBox->addItem(name);
    }

    ui.intervalSpinBox->setValue(m_interval);
    ui.intervalSpinBox->setSuffix(ki18np(" minute", " minutes"));
    ui.switchInterval->setValue(m_switchInterval);
    ui.switchInterval->setSuffix(ki18np(" second", " seconds"));
    ui.maxAge->setValue(m_maxAge);
    ui.maxAge->setSuffix(ki18np(" hour", " hours"));
    if (m_logo) {
        ui.logo->setCheckState(Qt::Checked);
    } else {
        ui.logo->setCheckState(Qt::Unchecked);
    }
    if (m_animations) {
        ui.animations->setCheckState(Qt::Checked);
    } else {
        ui.animations->setCheckState(Qt::Unchecked);
    }
    if (m_showdroptarget) {
        ui.showDropTarget->setCheckState(Qt::Checked);
    } else {
        ui.showDropTarget->setCheckState(Qt::Unchecked);
    }

    feedsUi.feedList->addItems(m_feedlist);
    parent->addPage(widget, i18n("General"), icon());
    parent->addPage(fWidget, i18n("Feeds"), icon());
    
    connect(ui.showDropTarget, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.logo, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.animations, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.intervalSpinBox, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.switchInterval, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.maxAge, SIGNAL(valueChanged(QString)), parent, SLOT(settingsModified()));
    connect(feedsUi.feedComboBox, SIGNAL(editTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(feedsUi.addFeed, SIGNAL(released()), parent, SLOT(settingsModified()));
    connect(feedsUi.removeFeed, SIGNAL(released()), parent, SLOT(settingsModified()));
    connect(feedsUi.feedList, SIGNAL(itemSelectionChanged()), parent, SLOT(settingsModified())); 
}

void News::slotChangeText( const QString& text )
{
    feedsUi.addFeed->setEnabled( !text.isEmpty() );
}

void News::slotItemChanged()
{
    feedsUi.removeFeed->setEnabled( ( !feedsUi.feedList->selectedItems ().isEmpty() ) );
}

void News::addFeed()
{
    if (!feedsUi.feedComboBox->currentText().isEmpty()) {
        QString url = feedsUi.feedComboBox->currentText();
        if (m_defaultFeeds.keys().contains(url)) {
            url = m_defaultFeeds[url];
        }
        bool found = false;
        for (int i = 0; i < feedsUi.feedList->count(); i++) {
            QString feed = feedsUi.feedList->item(i)->text();
            if (feed == url) {
                found = true;
            }
        }
        if (!found) {
            feedsUi.feedList->addItem(url);
        }
    }
}

void News::removeFeed()
{
    int row = feedsUi.feedList->currentRow();
    if (row != -1) {
        feedsUi.feedList->takeItem(row);
        if (feedsUi.feedList->count() == 0) {
            feedsUi.removeFeed->setEnabled(false);
        }
    }
}

void News::configAccepted()
{
    m_interval = ui.intervalSpinBox->value();
    m_switchInterval = ui.switchInterval->value();
    m_maxAge = ui.maxAge->value();
    m_logo = ui.logo->isChecked();
    m_animations = ui.animations->isChecked();
    m_showdroptarget = ui.showDropTarget->isChecked();

    m_feedlist.clear();
    QString feed;
    for (int i = 0; i < feedsUi.feedList->count(); i++) {
        feed = feedsUi.feedList->item(i)->text();
        if (m_defaultFeeds.keys().contains(feed)) {
            feed = m_defaultFeeds[feed];
        }
        m_feedlist.append(feed);
    }

    KConfigGroup cg = config();
    cg.writeEntry("feeds", m_feedlist);
    cg.writeEntry("interval", m_interval);
    cg.writeEntry("switchInterval", m_switchInterval);
    cg.writeEntry("animations", m_animations);
    cg.writeEntry("logo", m_logo);
    cg.writeEntry("droptarget", m_showdroptarget);
    cg.writeEntry("maxAge", m_maxAge);

    emit configNeedsSaving();

    // updateScrollers() is already called from configChanged(), which will
    // always be called after this method when the config dialog is applied
}

void News::updateScrollers()
{
    m_timer->stop();
    m_timer->setInterval(m_switchInterval * 1000);

    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_layout->setSpacing(2);
    setLayout(m_layout);

    if (m_logo) {
        m_layout->addItem(m_header);
        m_header->show();
    } else {
        m_header->hide();
    }

    qDeleteAll(m_scrollerList);
    m_scrollerList.clear();
    for (int i = 0; i < m_feedlist.size(); i++ ) {
        Scroller * scroller = new Scroller(this);
        m_layout->addItem(scroller);
        m_scrollerList.append(scroller);
        scroller->setAnimations(m_animations);
        scroller->setMaxAge(m_maxAge);
        scroller->listUpdated();
    }

    if (m_showdroptarget) {
        Scroller * scroller = new Scroller(this);
        m_scrollerList.append(scroller);
        m_layout->addItem(scroller);
        scroller->setAnimations(m_animations);
        scroller->setDropTarget(true);
        scroller->setMaxAge(m_maxAge);
        scroller->listUpdated();
    }


    foreach (Scroller *scroller, m_scrollerList) {
        m_layout->addItem(scroller);
    }

    m_timer->start();
    connectToEngine();
}

//TODO: move to scroller to make adding to existing groups by dropping
//possible.
void News::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    kDebug() << "dropEvent()";
    if (KUrl::List::canDecode(event->mimeData())) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());

        if (urls.count() > 0) {
            event->accept();

            //FIXME: yeah, using strings her is ugly, but nested lists give
            //issues with KConfigGroup. This could be done better though.
            // it causes bugs, too! see the spiegel.de rss which url contains ","
            // i have fixed the rss dataengine as well so now " " is standard
            QString feeds;
            foreach (const KUrl& feed, urls){
                feeds.append(QString(feed.prettyUrl()));
                feeds.append(" ");
            }

            int start = 0;
            if (m_logo) {
                start = 1;
            }

            //ok, let's just temporarily disable this, since this has to be
            //ported to WoC, and is less trivial at a glance.
            for (int i = start; i < (m_layout->count()); i++ ) {
                QGraphicsLayoutItem * item = m_layout->itemAt(i);
                QRectF rect = item->geometry();
                if (rect.contains(event->pos())) {
                    if (m_showdroptarget && (i == (m_layout->count() - 1))) {
                        m_feedlist.append(feeds);
                    } else {
                        if (!m_feedlist[i-start].endsWith(' ')) {
                            m_feedlist[i-start].append(" ");
                        }
                        m_feedlist[i-start].append(feeds);
                    }
                }
            }

            KConfigGroup cg = config();
            cg.writeEntry("feeds", m_feedlist);
            emit configNeedsSaving();

            updateScrollers();
        }
    }
}

void News::constraintsEvent(Plasma::Constraints constraints)
{
    //kDebug() << "constrainsUpdated : " << constraints;
    if (constraints & Plasma::FormFactorConstraint || constraints & Plasma::SizeConstraint ) {
        if (formFactor() == Plasma::Horizontal) {
            int minSize = (m_feedlist.size() * m_scrollerList.first()->minimumSize().height()) + m_header->size().height();
            //kDebug() << "minimum size to contain all scrollers: " << minSize << m_feedlist;
            if (contentsRect().height() < minSize) {
                //group all feeds together so it can fit (only a single time):
                QString allfeeds;
                foreach (const QString &feed, m_feedlist) {
                    allfeeds.append(feed);
                    if (!feed.endsWith(' ')) {
                        allfeeds.append(" ");
                    }
                }
                kDebug() << "allfeeds = " << allfeeds;
                m_feedlist.clear();
                m_feedlist.append(allfeeds);
                m_showdroptarget = false;
                m_logo = false;
                m_collapsed = true;
                updateScrollers();
            } else {
                m_collapsed = false;
                updateScrollers();
            }
        }
    }
}

QMap<QString, QString> News::akregatorFeeds()
{
    QMap<QString, QString> result;
    QString file = KStandardDirs::locate("data", "akregator/data/feeds.opml");
    kDebug() << file;
    if (!file.isEmpty()) {
        QFile f(file);
        if (f.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            if (doc.setContent(&f)) {
                QDomNodeList nodes = doc.elementsByTagName("outline");
                for (int i = 0; i < nodes.count(); ++i) {
                    QDomElement element = nodes.item(i).toElement();
                    QString title = element.attribute("title");
                    if (!title.isEmpty()) {
                        result[title] = element.attribute("xmlUrl");
                    }
                }
            }
            f.close();
        }
    }
    return result;
}

#include "news.moc"
