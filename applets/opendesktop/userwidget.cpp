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

UserWidget::UserWidget(QGraphicsWidget *parent)
    : Frame(parent),
      m_css(0),
      m_image(0),
      m_nameLabel(0),
      m_infoView(0)
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

void UserWidget::setAtticaData(Plasma::DataEngine::Data data)
{
    //kDebug() << data;
    kDebug() << "Updating user widget";
    m_atticaData = data;
    setName();

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
    QPixmap pm = data["Avatar"].value<QPixmap>();
    QString pmUrl = data["AvatarUrl"].toUrl().toString();
    if (!pm.isNull()) {
        m_image->setPixmap(pm);
    } else {
        kDebug() << "avatarUrl" << pmUrl;
    }
    QStringList userInfo;

    userInfo << "<table>";
    QString d = KGlobal::locale()->formatDate(data["Birthday"].toDate(), KLocale::FancyLongDate);
    userInfo << addRow(i18n("Birthday:"), d);
    if (!location.isEmpty()) {
        userInfo << addRow(i18n("Location:"), location);
    }
    userInfo << addRow(i18n("IRC Nickname:"), data["ircnick"].toString());
    userInfo << addRow(i18n("Company:"), data["company"].toString());
    userInfo << addRow(i18n("Languages:"), data["languages"].toString());
    userInfo << addRow(i18n("Interests:"), data["interests"].toString());
    userInfo << addRow(i18n("Music:"), data["favouritemusic"].toString());
    userInfo << addRow(i18n("TV Shows:"), data["favouritetvshows"].toString());
    userInfo << addRow(i18n("Games:"), data["favouritegames"].toString());
    userInfo << addRow(i18n("Programming:"), data["programminglanguages"].toString());
    userInfo << addRow(i18n("%1 likes:", data["Id"].toString()), data["likes"].toString());
    userInfo << addRow(i18n("%1 does not like:", data["Id"].toString()), data["dontlikes"].toString());


    userInfo << "</table>";
    QString html = userInfo.join("");
    kDebug() << html;
    setInfo(html);

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

void UserWidget::setName(const QString &name)
{
    QString html;
    if (name.isEmpty()) {
        QString _id = m_atticaData["Id"].toString();

        QString _name = m_atticaData["Name"].toString();

        if (_name.isEmpty()) {
            html.append(QString("<font size=\"+2\"><b>%1</b></font>").arg(_id));
        } else {
            html.append(QString("<font size=\"+2\"><b>%1 (%2)</b></font>").arg(_name, _id));
        }

        QString crole = m_atticaData["description"].toString();
        kDebug() << "=========== DESC" << m_atticaData["description"].toString() << m_atticaData["description"].toString();
        if (!crole.isEmpty()) {
            html.append(QString("\n<br />%1").arg(crole));
            kDebug() << "CROLE" << crole;
        }
    } else {
        html = QString("<h2>%1</h2>").arg(name);
    }

    if (m_nameLabel) {
        m_nameLabel->setText(QString("<style>%1</style>%2").arg(m_css->styleSheet(), html));
        //kDebug() << "NAME TEXT" << m_nameLabel->text();
    }
}

void UserWidget::setInfo(const QString &text)
{
    QString t = text;
    if (!text.isEmpty()) {
        m_info = text;
    }
    //kDebug();
    if (m_infoView) {
      QString html;
      if (m_css) {
          html = (QString("<style>%1</style>%2").arg(m_css->styleSheet(), m_info));
      } else {
          html = m_info;
      }
      m_infoView->setHtml(html);
    }
}


#include "userwidget.moc"
