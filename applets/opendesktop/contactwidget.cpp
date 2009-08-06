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
#include <QGraphicsSceneMouseEvent>
#include <QLabel>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KIconLoader>
#include <KGlobalSettings>
#include <KRun>

// Plasma
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Frame>

// own
#include "contactwidget.h"
#include "contactimage.h"

using namespace Plasma;

ContactWidget::ContactWidget(QGraphicsWidget *parent)
    : Frame(parent),
      id(0),
      m_isHovered(false),
      m_isFriend(false),
      m_image(0),
      m_nameLabel(0),
      m_sendMessage(0),
      m_addFriend(0),
      m_showDetails(0)
{
    setAcceptHoverEvents(true);
    buildDialog();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateColors()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(updateColors()));
    setMinimumHeight(40);
    setMinimumWidth(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

ContactWidget::~ContactWidget()
{
}

void ContactWidget::setAtticaData(const Plasma::DataEngine::Data &data)
{
    //kDebug() << data;
    m_ocsData = data;
    QString _id = data["Id"].toString();

    QString name = data["Name"].toString();
    if (name.isEmpty()) {
        setName(_id);
    } else {
        setName(QString("%1 (%2)").arg(name, _id));
    }

    QString city = data["City"].toString();
    QString country = data["Country"].toString();
    QString location;
    if (!city.isEmpty() && !country.isEmpty()) {
        location = QString("%1, %2").arg(city, country);
    } else if (country.isEmpty() && !city.isEmpty()) {
        location = city;
    } else if (city.isEmpty() && !country.isEmpty()) {
        location = country;
    }

    if (!location.isEmpty()) {
        setInfo(location);
    }
    QPixmap pm = data["Avatar"].value<QPixmap>();
    QString pmUrl = data["AvatarUrl"].toUrl().toString();
    if (!pm.isNull()) {
        m_image->setPixmap(pm);
    } else {
        kDebug() << "avatarUrl" << pmUrl;
    }
}

void ContactWidget::buildDialog()
{
    updateColors();

    int m = KIconLoader::SizeMedium;

    m_layout = new QGraphicsGridLayout(this);
    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->setColumnFixedWidth(0, int(m*1.2));
    m_layout->setHorizontalSpacing(4);

    m_image = new ContactImage(this);
    m_image->setMinimumHeight(m);
    m_image->setMinimumWidth(m);
    m_layout->addItem(m_image, 0, 0, 2, 1, Qt::AlignTop);

    m_nameLabel = new Plasma::Label(this);
    m_nameLabel->nativeWidget()->setWordWrap(false);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nameLabel->setMinimumWidth(m*2);
    m_layout->addItem(m_nameLabel, 0, 1, 1, 1, Qt::AlignTop);

    int s = KIconLoader::SizeSmallMedium; // The size for the action icons

    m_actions = new QGraphicsLinearLayout(m_layout);
    m_actions->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_infoLabel = new Plasma::Label(this);
    m_infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_infoLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_infoLabel->nativeWidget()->setWordWrap(false);
    m_actions->addItem(m_infoLabel);

    m_sendMessage = new Plasma::IconWidget(this);
    m_sendMessage->setIcon("mail-send");
    m_sendMessage->setToolTip(i18n("Send Message"));
    m_sendMessage->setMinimumHeight(s);
    m_sendMessage->setMaximumHeight(s);
    m_sendMessage->setMinimumWidth(s);
    m_sendMessage->setMaximumWidth(s);

    m_addFriend = new Plasma::IconWidget(this);
    m_addFriend->setIcon("list-add-user");
    m_addFriend->setToolTip(i18n("Add as Friend"));
    m_addFriend->setMinimumHeight(s);
    m_addFriend->setMaximumHeight(s);
    m_addFriend->setMinimumWidth(s);
    m_addFriend->setMaximumWidth(s);

    m_showDetails = new Plasma::IconWidget(this);
    m_showDetails->setIcon("user-properties");
    m_showDetails->setToolTip(i18n("User Details"));
    m_showDetails->setMinimumHeight(s);
    m_showDetails->setMaximumHeight(s);
    m_showDetails->setMinimumWidth(s);
    m_showDetails->setMaximumWidth(s);

    connect(m_sendMessage, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(m_addFriend, SIGNAL(clicked()), this, SLOT(addFriend()));
    connect(m_showDetails, SIGNAL(clicked()), this, SLOT(userProperties()));

    m_actions->addItem(m_addFriend);
    m_actions->addItem(m_sendMessage);
    m_actions->addItem(m_showDetails);

    m_layout->addItem(m_actions, 1, 1, 1, 1, Qt::AlignBottom | Qt::AlignRight);

    setLayout(m_layout);

    updateActions();
    updateColors();
}

void ContactWidget::setIsFriend(bool isFriend)
{
    m_isFriend = isFriend;
    updateActions();
}

void ContactWidget::updateActions()
{
    m_addFriend->setVisible(m_isHovered && !m_isFriend);
    m_sendMessage->setVisible(m_isHovered);
    m_showDetails->setVisible(m_isHovered);
}

void ContactWidget::setImage(const QImage &image)
{
    m_image->setImage(image);
}

void ContactWidget::sendMessage()
{
    QUrl url = QString("https://www.opendesktop.org/messages/?action=newmessage&username=%1").arg(user());
    new KRun(url, m_nameLabel->nativeWidget());
}

void ContactWidget::addFriend()
{
    KUrl url = QString("https://www.opendesktop.org/usermanager/relationadd.php?username=%1").arg(user());
    kDebug() << "add friend" << user() << url.url();
    new KRun(url, m_nameLabel->nativeWidget());
}

void ContactWidget::userProperties()
{
    kDebug() << "user details" << user();
    // When sliding out, we'll usually miss the hoverLeaveEvent,
    // so do it manually
    m_isHovered = false;
    updateActions();
    emit showDetails(m_ocsData);
}

void ContactWidget::updateColors()
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

    qreal fontsize = KGlobalSettings::smallestReadableFont().pointSize();
    m_stylesheet = QString("\
                body { \
                    color: %1; \
                    font-size: %4pt; \
                    width: 100%, \
                    margin-left: 0px; \
                    margin-top: 0px; \
                    margin-right: 0px; \
                    margin-bottom: 0px; \
                    padding: 0px; \
                } \
\
                a:visited   { color: %1; }\
                a:link   { color: %2; opacity: .8; }\
                a:visited   { color: %3; opacity: .6; }\
                a:hover { text-decoration: none; opacity: .4; } \
\
    ").arg(text.name()).arg(link.name()).arg(linkvisited.name()).arg(fontsize);

    if (m_nameLabel) {
        m_nameLabel->setPalette(p);
        m_nameLabel->setStyleSheet(m_stylesheet);
    }
}

Plasma::DataEngine::Data ContactWidget::data()
{
    return m_ocsData;
}

void ContactWidget::setName(const QString &name)
{
    m_nameLabel->setText(name);
}

QString ContactWidget::name()
{
    return m_ocsData["Name"].toString();
}

QString ContactWidget::user()
{
    return m_ocsData["Id"].toString();
}

void ContactWidget::setInfo(const QString &text)
{
    if (text.isEmpty()) {
        m_infoLabel->setEnabled(false);
        m_infoLabel->setText(i18n("Unknown location"));
    } else {
        m_infoLabel->setEnabled(true);
        m_infoLabel->setText(text);
    }
}

void ContactWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_isHovered = true;
    updateActions();
}

void ContactWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_isHovered = false;
    updateActions();
}

#include "contactwidget.moc"
