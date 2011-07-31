/*
    Copyright 2008-2009 by Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "userwidget.h"

//Qt
#include <QGraphicsGridLayout>
#include <QWebPage>
#include <QLabel>
#include <QSignalMapper>

//KDE
#include <KDebug>
#include <KDirWatch>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KLocale>

// Plasma
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/WebView>

// own
#include "contactimage.h"
#include "stylesheet.h"
#include "utils.h"

using namespace Plasma;

UserWidget::UserWidget(DataEngine* engine, QGraphicsWidget* parent)
    : Frame(parent),
      m_css(0),
      m_image(0),
      m_nameLabel(0),
      m_infoView(0),
      m_friendWatcher(engine),
      m_engine(engine),
      m_personWatch(engine)
{
    m_info = i18n("No information available.");

    // listen for changes to the stylesheet file

    m_css = new StyleSheet(this);
    connect(m_css, SIGNAL(styleSheetChanged(QString)), this, SLOT(setStyleSheet(QString)));

    setMinimumHeight(200);
    setMinimumWidth(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    buildDialog();
    connect(Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateColors()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(updateColors()));
}

UserWidget::~UserWidget()
{
}


void UserWidget::buildDialog()
{
    updateColors();

    int m = 64; // size of the image
    int actionSize = KIconLoader::SizeSmallMedium;

    m_layout = new QGraphicsGridLayout(this);
    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->setColumnFixedWidth(0, m); // This could probably be a bit more dynamic
    m_layout->setColumnMinimumWidth(1, 60);
    m_layout->setHorizontalSpacing(4);

    m_image = new ContactImage(m_engine, this);

    m_image->setPreferredWidth(m);
    m_image->setPreferredHeight(m);
    m_image->setMinimumHeight(m);
    m_image->setMinimumWidth(m);
    m_layout->addItem(m_image, 0, 0, 1, 1, Qt::AlignTop);

    m_nameLabel = new Label(this);
    m_nameLabel->nativeWidget()->setWordWrap(true);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nameLabel->setMinimumWidth(60);
    m_nameLabel->setMaximumHeight(40);
    m_layout->addItem(m_nameLabel, 0, 1, 1, 1, Qt::AlignTop);


    m_infoView = new WebView(this);
    m_infoView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //m_infoView->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    //m_infoView->nativeWidget()->setWordWrap(true);

    m_layout->addItem(m_infoView, 1, 0, 1, 2, Qt::AlignTop);

    Plasma::IconWidget* back = new Plasma::IconWidget;
    back->setIcon("go-previous-view");
    back->setToolTip(i18n("Back"));
    back->setMinimumHeight(actionSize);
    back->setMaximumHeight(actionSize);
    back->setMinimumWidth(actionSize);
    back->setMaximumWidth(actionSize);

    m_addFriend = new Plasma::IconWidget;
    m_addFriend->setIcon("list-add-user");
    m_addFriend->setToolTip(i18n("Add friend"));
    m_addFriend->setMinimumHeight(actionSize);
    m_addFriend->setMaximumHeight(actionSize);
    m_addFriend->setMinimumWidth(actionSize);
    m_addFriend->setMaximumWidth(actionSize);

    m_sendMessage = new Plasma::IconWidget;
    m_sendMessage->setIcon("mail-send");
    m_sendMessage->setToolTip(i18n("Send message"));
    m_sendMessage->setMinimumHeight(actionSize);
    m_sendMessage->setMaximumHeight(actionSize);
    m_sendMessage->setMinimumWidth(actionSize);
    m_sendMessage->setMaximumWidth(actionSize);

    QGraphicsLinearLayout* actionLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    actionLayout->addItem(back);
    actionLayout->addStretch();
    actionLayout->addItem(m_addFriend);
    actionLayout->addItem(m_sendMessage);

    m_layout->addItem(actionLayout, 2, 0, 1, 2);
    setLayout(m_layout);

    m_mapper = new QSignalMapper(this);

    connect(back, SIGNAL(clicked()), SIGNAL(done()));
    connect(m_sendMessage, SIGNAL(clicked()), m_mapper, SLOT(map()));
    connect(m_addFriend, SIGNAL(clicked()), m_mapper, SLOT(map()));

    connect(m_mapper, SIGNAL(mapped(QString)),
            this, SIGNAL(sendMessage(QString)));

    updateColors();

    connect(&m_personWatch, SIGNAL(updated()), SLOT(dataUpdated()));
}

void UserWidget::setId(const QString& id)
{
    m_id = id;
    m_mapper->setMapping(m_sendMessage, m_id);
    m_mapper->setMapping(m_addFriend, m_id);
    m_personWatch.setId(id);
    m_addFriend->setVisible(!m_friendWatcher.contains(m_id));
}

void UserWidget::setOwnId(const QString& ownId)
{
    m_ownId = ownId;
    m_friendWatcher.setSource(friendsQuery(m_provider, m_ownId));
    m_addFriend->setVisible(!m_friendWatcher.contains(m_id));
}

void UserWidget::setProvider(const QString& provider)
{
    m_personWatch.setProvider(provider);
    m_provider = provider;
    m_friendWatcher.setSource(friendsQuery(m_provider, m_ownId));
    m_addFriend->setVisible(!m_friendWatcher.contains(m_id));
}


void UserWidget::dataUpdated()
{
    m_image->setUrl(m_personWatch.data().value("AvatarUrl").toUrl());

    setName();
    setInfo();

}

QString UserWidget::addRow(const QString& title, const QString& text)
{
    if (!text.isEmpty()) {
        return QString("<tr><td class=\"rowheader\">%1</td><td>%2</td></tr>\n").arg(title, text);
    } else {
        return QString();
    }
}

void UserWidget::setStyleSheet(const QString &stylesheet)
{
    Q_UNUSED(stylesheet)
    // kDebug() << "Setting new stylesheet" << stylesheet;
    updateColors();
    setName();
    setInfo();
}

void UserWidget::updateColors()
{
    QPalette p = palette();

    // Set background to transparent and use the theme to provide contrast with the text
    p.setColor(QPalette::Base, Qt::transparent); // new in Qt 4.5
    p.setColor(QPalette::Window, Qt::transparent); // For Qt 4.4, remove when we depend on 4.5


    QColor text = Theme::defaultTheme()->color(Theme::TextColor);
    QColor link = Theme::defaultTheme()->color(Theme::TextColor);
    link.setAlphaF(qreal(.8));
    QColor linkvisited = Theme::defaultTheme()->color(Theme::TextColor);
    linkvisited.setAlphaF(qreal(.6));

    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Link, link);
    p.setColor(QPalette::LinkVisited, linkvisited);

    setPalette(p);

    if (m_nameLabel) {
        m_nameLabel->setPalette(p);
        if (m_css) {
            m_nameLabel->setStyleSheet(m_css->styleSheet());
        }
        m_infoView->page()->setPalette(p);
    }
    update();
    // kDebug() << "CSS:" << m_css->styleSheet();
}

void UserWidget::setName()
{
    DataEngine::Data data = m_personWatch.data();
    QString html;

    QString _name = data["Name"].toString();

    if (_name.isEmpty()) {
        html = QString("<font size=\"+2\"><b>%1</b></font>").arg(m_id);
    } else {
        html = QString("<font size=\"+2\"><b>%1 (%2)</b></font>").arg(_name, m_id);
    }

    QString crole = data["description"].toString();
    
    if (!crole.isEmpty()) {
        html.append(QString("\n<br />%1").arg(crole));
    }

    if (m_nameLabel) {
        m_nameLabel->setText(QString("<style>%1</style>%2").arg(m_css->styleSheet(), html));
    }
}

void UserWidget::setInfo()
{
    DataEngine::Data data = m_personWatch.data();
    QString city = data["City"].toString();
    QString country = data["Country"].toString();
    qreal lat = (qreal)(data["Latitude"].toDouble());
    qreal lon = (qreal)(data["Longitude"].toDouble());

    QString location;
    if (!city.isEmpty() && !country.isEmpty()) {
        location = QString("%1, %2").arg(city, country);
    } else if (country.isEmpty() && !city.isEmpty()) {
        location = city;
    } else if (city.isEmpty() && !country.isEmpty()) {
        location = country;
    }

    if (lat > 0 && lon) {
        location = i18nc("city, country, latitude and longitude", "%1 (Lat: %2, Long: %3)", location, KGlobal::locale()->formatNumber(lat, 2), KGlobal::locale()->formatNumber(lon, 2));
    }

    QString birthday = KGlobal::locale()->formatDate(data["Birthday"].toDate(), KLocale::FancyLongDate);

    QStringList userInfo;

    userInfo << "<table>";
    userInfo << addRow(i18n("Birthday:"), birthday);
    userInfo << addRow(i18n("Location:"), location);
    userInfo << addRow(i18n("IRC Nickname:"), data["ircnick"].toString());
    userInfo << addRow(i18n("Company:"), data["company"].toString());
    userInfo << addRow(i18n("Languages:"), data["languages"].toString());
    userInfo << addRow(i18n("Interests:"), data["interests"].toString());
    userInfo << addRow(i18n("Music:"), data["favouritemusic"].toString());
    userInfo << addRow(i18n("TV Shows:"), data["favouritetvshows"].toString());
    userInfo << addRow(i18n("Games:"), data["favouritegames"].toString());
    userInfo << addRow(i18n("Programming:"), data["programminglanguages"].toString());
    userInfo << addRow(i18n("%1 likes:", m_id), data["likes"].toString());
    userInfo << addRow(i18n("%1 does not like:", m_id), data["dontlikes"].toString());
    userInfo << "</table>";

    QString html = userInfo.join("");

    if (m_infoView) {
        if (m_css) {
            html = (QString("<style>%1</style>%2").arg(m_css->styleSheet(), html));
        }
        m_infoView->setHtml(html);
    }
}


#include "userwidget.moc"
