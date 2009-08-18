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

//Qt
#include <QGraphicsGridLayout>
#include <QWebPage>
#include <QLabel>

//KDE
#include <KDebug>
#include <KDirWatch>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KLocale>
#include <KStandardDirs>

// Plasma
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/WebView>

// own
#include "userwidget.h"
#include "contactimage.h"
#include "stylesheet.h"

using namespace Plasma;

UserWidget::UserWidget(DataEngine* engine, QGraphicsWidget* parent)
    : Frame(parent),
      m_css(0),
      m_image(0),
      m_nameLabel(0),
      m_infoView(0),
      m_engine(engine)
{
    m_info = i18n("No information available.");

    // listen for changes to the stylesheet file

    m_css = new StyleSheet(this);
    m_cssFile = KStandardDirs::locate("data", "plasma-applet-opendesktop/user.css");
    connect(m_css, SIGNAL(styleSheetChanged(const QString&)), this, SLOT(setStyleSheet(const QString&)));


    //loadStyleSheet(m_cssFile);

    /*
    KDirWatch *cssWatch = new KDirWatch(this);
    cssWatch->addFile(m_cssFile);
    connect(cssWatch,SIGNAL(dirty(QString)),this,SLOT(loadStyleSheet(QString)));
    connect(cssWatch,SIGNAL(created(QString)),this,SLOT(loadStyleSheet(QString)));
    connect(cssWatch,SIGNAL(deleted(QString)),this,SLOT(loadStyleSheet(QString)));
    */
    setMinimumHeight(200);
    setMinimumWidth(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    buildDialog();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateColors()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(updateColors()));
}

UserWidget::~UserWidget()
{
}


void UserWidget::buildDialog()
{
    updateColors();

    int m = 64; // size of the image
    m_layout = new QGraphicsGridLayout(this);
    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->setColumnFixedWidth(0, m); // This could probably be a bit more dynamic
    m_layout->setColumnMinimumWidth(1, 60);
    m_layout->setHorizontalSpacing(4);

    m_image = new ContactImage(this);

    //m_image->setIcon("system-users");
    //m_image->setPreferredWidth(64);
    //m_image->setPreferredHeight(64);
    m_image->setMinimumHeight(m);
    m_image->setMinimumWidth(m);
    m_layout->addItem(m_image, 0, 0, 1, 1, Qt::AlignTop);

    m_nameLabel = new Plasma::Label(this);
    m_nameLabel->nativeWidget()->setWordWrap(true);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nameLabel->setMinimumWidth(60);
    m_nameLabel->setMaximumHeight(40);
    m_layout->addItem(m_nameLabel, 0, 1, 1, 1, Qt::AlignTop);


    m_infoView = new Plasma::WebView(this);
    m_infoView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //m_infoView->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    //m_infoView->nativeWidget()->setWordWrap(true);

    m_layout->addItem(m_infoView, 1, 0, 1, 2, Qt::AlignTop);

    setLayout(m_layout);

    updateColors();
}

void UserWidget::setId(const QString& id) {
    // Disconnect from the last person displayed
    if (!m_id.isEmpty())
        m_engine->disconnectSource(QString("Person-%1").arg(m_id), this);
    
    m_id = id;
    
    if (!m_id.isEmpty())
        m_engine->connectSource(QString("Person-%1").arg(m_id), this);

    m_data = m_engine->query(QString("Person-%1").arg(m_id));
    setName();
    setInfo();
}

void UserWidget::dataUpdated(const QString& source, const Plasma::DataEngine::Data& atticaData)
{
    //kDebug() << data;
    kDebug() << "Updating user widget";
    m_data = atticaData[source].value<Plasma::DataEngine::Data>();

    QPixmap pm = m_data["Avatar"].value<QPixmap>();
    QString pmUrl = m_data["AvatarUrl"].toUrl().toString();
    if (!pm.isNull()) {
        m_image->setPixmap(pm);
    } else {
        kDebug() << "avatarUrl" << pmUrl;
    }
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
    kDebug() << "Setting new stylesheet" << stylesheet;
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


    QColor text = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QColor link = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    link.setAlphaF(qreal(.8));
    QColor linkvisited = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    linkvisited.setAlphaF(qreal(.6));

    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Link, link);
    p.setColor(QPalette::LinkVisited, linkvisited);

    setPalette(p);

    if (m_image) {
        m_image->fg = text;
        m_image->bg = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    }
    if (m_nameLabel) {
        m_nameLabel->setPalette(p);
        if (m_css) {
            m_nameLabel->setStyleSheet(m_css->styleSheet());
        }
        m_infoView->page()->setPalette(p);
    }
    update();
    kDebug() << "CSS:" << m_css->styleSheet();
}

void UserWidget::setName()
{
    QString html;

    QString _name = m_data["Name"].toString();

    if (_name.isEmpty()) {
        html = QString("<font size=\"+2\"><b>%1</b></font>").arg(m_id);
    } else {
        html = QString("<font size=\"+2\"><b>%1 (%2)</b></font>").arg(_name, m_id);
    }

    QString crole = m_data["description"].toString();
    
    if (!crole.isEmpty()) {
        html.append(QString("\n<br />%1").arg(crole));
    }

    if (m_nameLabel) {
        m_nameLabel->setText(QString("<style>%1</style>%2").arg(m_css->styleSheet(), html));
    }
}

void UserWidget::setInfo()
{
    QString city = m_data["City"].toString();
    QString country = m_data["Country"].toString();
    qreal lat = (qreal)(m_data["Latitude"].toDouble());
    qreal lon = (qreal)(m_data["Longitude"].toDouble());

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

    QString birthday = KGlobal::locale()->formatDate(m_data["Birthday"].toDate(), KLocale::FancyLongDate);

    QStringList userInfo;

    userInfo << "<table>";
    userInfo << addRow(i18n("Birthday:"), birthday);
    userInfo << addRow(i18n("Location:"), location);
    userInfo << addRow(i18n("IRC Nickname:"), m_data["ircnick"].toString());
    userInfo << addRow(i18n("Company:"), m_data["company"].toString());
    userInfo << addRow(i18n("Languages:"), m_data["languages"].toString());
    userInfo << addRow(i18n("Interests:"), m_data["interests"].toString());
    userInfo << addRow(i18n("Music:"), m_data["favouritemusic"].toString());
    userInfo << addRow(i18n("TV Shows:"), m_data["favouritetvshows"].toString());
    userInfo << addRow(i18n("Games:"), m_data["favouritegames"].toString());
    userInfo << addRow(i18n("Programming:"), m_data["programminglanguages"].toString());
    userInfo << addRow(i18n("%1 likes:", m_id), m_data["likes"].toString());
    userInfo << addRow(i18n("%1 does not like:", m_id), m_data["dontlikes"].toString());
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
