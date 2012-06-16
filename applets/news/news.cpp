/*
 * Copyright (C) 2007, 2008 Petri Damsten <damu@iki.fi>
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

#include "news.h"

#include <QGraphicsSceneDragDropEvent>
#include <QFile>
#include <QColor>
#include <QDomDocument>
#include <QGraphicsLinearLayout>
#include <QWebFrame>
#include <QWebPage>
#include <QFileInfo>

#include <KConfigDialog>
#include <KColorScheme>
#include <KDebug>
#include <KStandardDirs>
#include <KRun>

#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/WebView>

static const char *BEGIN =
"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
"<head>\n"
" <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
" <link rel=\"stylesheet\" type=\"text/css\" href=\"news.css\" />\n"
" <title>Plasma News</title>\n"
"</head>\n"
"<body>\n";

static const char *BEGIN_TABLE = "<table>\n";
static const char *HEADER_LINE = "<tr class=\"header_line\">"
                                 "<td colspan=\"2\" class=\"header\"><a href=\"%1\">%2</a></td></tr>\n";
static const char *DATE_LINE   = "<tr class=\"date_line\"><td colspan=\"2\" class=\"date\">%1</td></tr>\n";
static const char *LINE        = "<tr class=\"line-%1\">"
                                 "<td colspan=\"2\" class=\"text\"><a href=\"%2\">%3</a></td></tr>\n";
static const char *TIME_LINE   = "<tr class=\"line-%1\"><td class=\"time\">%2</td>"
                                 "<td class=\"text\"><a href=\"%3\">%4</a></td></tr>\n";
static const char *TITLE       = "<span class=\"title\">%1</span>";
static const char *TITLE_DESC  = "<span class=\"title_with_description\">%1</span><br/>";
static const char *DESCRIPTION = "<span class=\"description\">%1</span>";
static const char *END_TABLE   = "</table>\n";
static const char *END         = "</body>\n</html>\n";

static const char *CSS =
            ".header { font-size:11pt; font-weight:bold; color:%1; background-color:%2; }\n"
            ".date { font-size:10pt; font-weight:bold; color:%1; background-color:%2; }\n"
            ".line-0 { font-size:10pt; color:%1; background-color:%3; }\n"
            ".line-1 { font-size:10pt; color:%1; background-color:%4; }\n"
            ".time, .title, .description { }\n"
            ".text { width:100%; }\n"
            ".title_with_description { font-weight:bold; }\n"
            "a { text-decoration:none; color:%1 }\n"
            "table { width:100%; border-spacing:0px; }\n"
            "td { vertical-align: top; }\n"
            "body { margin:0px; background:none }\n";

News::News(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_graphicsWidget(0)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    setPopupIcon("application-rss+xml");

    setAssociatedApplication("akregator");
    
    resize(300,300);
}

News::~News()
{
}


QGraphicsWidget *News::graphicsWidget()
{
    if (m_graphicsWidget) {
        return m_graphicsWidget;
    }

    m_layout = new QGraphicsLinearLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    m_news = new Plasma::WebView(this);
    m_news->setDragToScroll(true);
    m_news->installEventFilter(this);
    m_news->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    connect(m_news->page(), SIGNAL(linkClicked(QUrl)),
            this, SLOT(linkActivated(QUrl)));
    m_news->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    m_layout->addItem(m_news);


    setAcceptDrops(true);
    makeStylesheet();

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(makeStylesheet()));

    m_graphicsWidget = new QGraphicsWidget(this);
    m_graphicsWidget->setLayout(m_layout);
    m_graphicsWidget->setPreferredSize(370,440);
    //a tiny minimum size, a panel 48px high is enough to display the whole applet
    m_graphicsWidget->setMinimumSize(150, 150);
    QPalette palette = m_graphicsWidget->palette();
    palette.setBrush(QPalette::Base, Qt::transparent);
    m_graphicsWidget->setPalette(palette);

    configChanged();

    return m_graphicsWidget;
}

void News::configChanged()
{
    kDebug();
    KConfigGroup cg = config();
    m_interval = cg.readEntry("interval", 30);
    m_showTimestamps = cg.readEntry("showTimestamps", true);
    m_showTitles = cg.readEntry("showTitles", true);
    m_showDescriptions = cg.readEntry("showDescriptions", false);

    m_feeds = cg.readEntry("feeds", QStringList("http://www.kde.org/dotkdeorg.rdf"));

    connectToEngine();
}

void News::connectToEngine()
{
    if (!m_feeds.isEmpty()) {
        Plasma::DataEngine* engine = dataEngine("rss");
        if( engine) {

            QString feedstring;

            foreach (const QString& feed, m_feeds) {
                QByteArray ba = QUrl::toPercentEncoding(feed, ":/&?=");
                feedstring.append(ba);
                feedstring.append(" ");
            }
            m_feedstring = feedstring;
            kDebug() << feedstring;
            engine->connectSource(feedstring, this, m_interval * 60 * 1000);
        }
        else
            kDebug()<<"Can not connect to dataengine rss";
    }
}

void News::makeStylesheet()
{
    const QString search = "desktoptheme/" + Plasma::Theme::defaultTheme()->themeName() +
                     "/stylesheets/news.css";
    const QString path =  KStandardDirs::locate("data", search);
    if (path.isEmpty()) {
        QColor backgroundColor =
                Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
        QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        QColor altBackgroundColor = backgroundColor;
        altBackgroundColor.setHsv(altBackgroundColor.hue(), altBackgroundColor.saturation(),
                (altBackgroundColor.value() < 128)
                ? backgroundColor.value() + 40 : backgroundColor.value() - 40,
                altBackgroundColor.alpha());
        QString css = QString(CSS).arg(textColor.name())
                                .arg(QString("rgba(%1, %2, %3, 0.45)").arg(backgroundColor.red()).arg( backgroundColor.green()).arg( backgroundColor.blue()))
                                .arg(QString("rgba(%1, %2, %3, 0.25)").arg(backgroundColor.red()).arg( backgroundColor.green()).arg( backgroundColor.blue()))
                                .arg(QString("rgba(%1, %2, %3, 0.25)").arg(altBackgroundColor.red()).arg( altBackgroundColor.green()).arg( altBackgroundColor.blue()));
        KStandardDirs dirs;
        m_cssDir = dirs.saveLocation("data", "plasma_applet_news");

        QFile file(m_cssDir + "/news.css");
        if (file.open(QIODevice::WriteOnly)) {
            file.write(css.toUtf8());
        }
    } else {
        m_cssDir = QFileInfo(path).absolutePath() + '/';
    }

    QWebFrame* frame = m_news->mainFrame();
    if (frame) {
        frame->setHtml(frame->toHtml(), m_cssDir);
    }
}

QMap<QString, QString> News::akregatorFeeds()
{
    QMap<QString, QString> result;
    const QString file = KStandardDirs::locate("data", "akregator/data/feeds.opml");
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

void News::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    QWidget *fWidget = new QWidget(0);
    feedsUi.setupUi(fWidget);
    parent->addPage(widget, i18n("General"), icon());
    parent->addPage(fWidget, i18n("Feeds"), "application-rss+xml");

    connect(feedsUi.feedComboBox, SIGNAL(editTextChanged(QString)),
            this, SLOT(feedTextChanged(QString)));
    connect(parent, SIGNAL(accepted()), this, SLOT(configAccepted()));
    connect(feedsUi.addFeed, SIGNAL(clicked()), this, SLOT(addFeed()));
    connect(feedsUi.removeFeed, SIGNAL(clicked()), this, SLOT(removeFeed()));

    m_defaultFeeds = akregatorFeeds();
    feedsUi.feedComboBox->clear();
    feedTextChanged(QString());
    feedsUi.feedList->clear();
    foreach (const QString& name, m_defaultFeeds.keys()) {
        feedsUi.feedComboBox->addItem(name);
    }

    ui.intervalSpinBox->setValue(m_interval);
    ui.intervalSpinBox->setSuffix(ki18np(" minute", " minutes"));

    ui.timestampCheckBox->setChecked(m_showTimestamps);
    ui.titlesCheckBox->setChecked(m_showTitles);
    ui.descriptionsCheckBox->setChecked(m_showDescriptions);

    feedsUi.feedList->addItems(m_feeds);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(ui.intervalSpinBox, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.timestampCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.titlesCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.descriptionsCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(feedsUi.feedComboBox, SIGNAL(editTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(feedsUi.addFeed, SIGNAL(released()), parent, SLOT(settingsModified()));
    connect(feedsUi.removeFeed, SIGNAL(released()), parent, SLOT(settingsModified()));
    connect(feedsUi.feedList, SIGNAL(itemSelectionChanged()), parent, SLOT(settingsModified()));
}

void News::feedTextChanged(const QString& text)
{
    feedsUi.addFeed->setEnabled(!text.isEmpty());
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
        feedsUi.removeFeed->setEnabled(true);
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
    m_showTimestamps = ui.timestampCheckBox->isChecked();
    m_showTitles = ui.titlesCheckBox->isChecked();
    m_showDescriptions = ui.descriptionsCheckBox->isChecked();

    m_feeds.clear();
    QString feed;
    for (int i = 0; i < feedsUi.feedList->count(); i++) {
        feed = feedsUi.feedList->item(i)->text();
        if (m_defaultFeeds.keys().contains(feed)) {
            feed = m_defaultFeeds[feed];
        }
        m_feeds.append(feed);
    }

    KConfigGroup cg = config();
    cg.writeEntry("feeds", m_feeds);
    cg.writeEntry("interval", m_interval);
    cg.writeEntry("showTimestamps", m_showTimestamps);
    cg.writeEntry("showTitles", m_showTitles);
    cg.writeEntry("showDescriptions", m_showDescriptions);

    dataUpdated(m_feedstring, m_dataCache);
}

void News::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (KUrl::List::canDecode(event->mimeData())) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());

        if (!urls.isEmpty()) {
            event->accept();
            m_feeds.clear();
            foreach (const KUrl &feed, urls) {
                m_feeds << feed.prettyUrl();
            }
            m_interval = 30;
            connectToEngine();
        }
    }
}

bool News::eventFilter(QObject *receiver, QEvent *event)
{
    if (receiver == m_news && event->type() == QEvent::GraphicsSceneContextMenu) {
        return true;
    }

    return false;
}

void News::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    if (source == m_feedstring && !data.isEmpty()) {
        m_dataCache = data;
        QDateTime previous = QDateTime::currentDateTime();
        QString html;
        QVariantList items = data["items"].toList();
        uint columns = 1;
        uint i = 0;

        html += BEGIN;
        html += BEGIN_TABLE;
        html += QString(HEADER_LINE).arg(data["link"].toString())
                                    .arg(data["title"].toString());

        foreach (const QVariant &tmp, items) {
            QMap<QString, QVariant> item = tmp.toMap();
            if (item["time"].toUInt() != 0) {
                columns = 2;
            } else {
                columns = 1;
            }
            uint timeStamp = item["time"].toUInt();
            QString title = item["title"].toString();
            QString description = item["description"].toString().remove(QRegExp("<[^>]*>"));
            QString line;

            if (title != 0 && m_showTitles) {
                if (description != 0 && m_showDescriptions) {
                    line += QString(TITLE_DESC).arg(title);
                } else {
                    line += QString(TITLE).arg(title);
                }
            }
            if (description != 0 && m_showDescriptions) {
                line += QString(DESCRIPTION).arg(description);
            }
            if (timeStamp != 0 && m_showTimestamps) {
                QDateTime time;
                time.setTime_t(timeStamp);
                if (previous.date() != time.date()) {
                    html += QString(DATE_LINE).arg(time.date().toString());
                    i = 0;
                }
                html += QString(TIME_LINE).arg(i % 2)
                                            .arg(time.time().toString("HH:mm"))
                                            .arg(item["link"].toString())
                                            .arg(line);
                previous = time;
            } else {
                html += QString(LINE).arg(i % 2)
                                     .arg(item["link"].toString())
                                     .arg(line);
            }

            ++i;
        }
        html += END_TABLE;
        html += END;
        m_graphicsWidget->setPreferredSize(-1, -1);

        m_news->setHtml(html, m_cssDir);
        emit sizeHintChanged(Qt::PreferredSize);
    }
}

void News::linkActivated(const QUrl& link)
{
   KRun::runUrl(KUrl(link), "text/html", 0);
}

#include "news.moc"
