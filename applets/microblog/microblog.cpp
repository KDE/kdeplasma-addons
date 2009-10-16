/***************************************************************************
 *   Copyright (C) 2007 by André Duffeck <duffeck@kde.org>                 *
 *   Copyright (C) 2007 Chani Armitage <chanika@gmail.com>                 *
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

#include "microblog.h"

#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QFontMetrics>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsProxyWidget>
#include <QAction>
#include <QLabel>

#include <KColorScheme>
#include <KConfigDialog>
#include <KDateTime>
#include <KDebug>
#include <KIcon>
#include <KLineEdit>
#include <KMessageBox>
#include <KRun>
#include <KSharedConfig>
#include <KStringHandler>
#include <KTextEdit>
#include <KTextBrowser>
#include <KWallet/Wallet>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>
#include <Plasma/Service>
#include <Plasma/FlashingLabel>
#include <Plasma/IconWidget>
#include <Plasma/SvgWidget>
#include <Plasma/TextBrowser>
#include <Plasma/ScrollWidget>
#include <Plasma/TextEdit>
#include <Plasma/Frame>
#include <Plasma/ServiceJob>


Q_DECLARE_METATYPE(Plasma::DataEngine::Data)

MicroBlog::MicroBlog(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_graphicsWidget(0),
      m_newTweets(0),
      m_service(0),
      m_profileService(0),
      m_statusService(0),
      m_lastTweet(0),
      m_wallet(0),
      m_walletWait(None),
      m_colorScheme(0),
      m_tz(KTimeZone::utc()),
      m_showTweetsTimer(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setPopupIcon("view-pim-journal");
}

void MicroBlog::init()
{
    m_flash = new Plasma::FlashingLabel(this);
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/microblog");
    m_theme->setContainsMultipleImages(true);
}

void MicroBlog::constraintsEvent(Plasma::Constraints constraints)
{
    //i am an icon?
    if ((constraints|Plasma::SizeConstraint || constraints|Plasma::FormFactorConstraint) &&
        layout()->itemAt(0) != m_graphicsWidget) {
        paintIcon();
    }
}

void MicroBlog::paintIcon()
{
    int size = qMin(contentsRect().width(), contentsRect().height());
    if (size < 1) {
        size = KIconLoader::SizeSmall;
    }

    QPixmap icon(size, size);
    if (m_popupIcon.isNull()) {
        icon = KIconLoader::global()->loadIcon("view-pim-journal", KIconLoader::NoGroup, size);
    } else {
        icon.fill(Qt::transparent);
    }

    QPainter p(&icon);
    p.setRenderHints(QPainter::SmoothPixmapTransform);
    p.drawPixmap(icon.rect(), m_popupIcon, m_popupIcon.rect());
    //4.3: a notification system for popupapplets would be cool
    if (m_newTweets > 0) {
        QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
        QFontMetrics fm(font);
        QRect textRect(fm.boundingRect(QString::number(m_newTweets)));
        int textSize = qMax(textRect.width(), textRect.height());
        textRect.setSize(QSize(textSize, textSize));
        textRect.moveBottomRight(icon.rect().bottomRight());

        QColor c(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
        c.setAlphaF(0.6);

        p.setBrush(c);
        p.setPen(Qt::NoPen);
        p.setRenderHints(QPainter::Antialiasing);
        p.drawEllipse(textRect);

        p.setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        p.drawText(textRect, Qt::AlignCenter, QString::number(m_newTweets));
    }
    p.end();

    setPopupIcon(icon);
}

void MicroBlog::popupEvent(bool show)
{
    if (show) {
        m_newTweets = 0;
        paintIcon();
    }
}

void MicroBlog::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    m_statusEdit->setFocus();
}

QGraphicsWidget *MicroBlog::graphicsWidget()
{
    if (m_graphicsWidget) {
        return m_graphicsWidget;
    }

    m_graphicsWidget = new QGraphicsWidget(this);
    m_graphicsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_colorScheme = new KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));

    //config stuff
    KConfigGroup cg = config();
    m_serviceUrl = cg.readEntry("serviceUrl", "https://identi.ca/api/");
    m_username = cg.readEntry("username");
    m_password = KStringHandler::obscure(cg.readEntry("password"));
    m_historySize = cg.readEntry("historySize", 2);
    m_historyRefresh = cg.readEntry("historyRefresh", 5);
    m_includeFriends = cg.readEntry("includeFriends", true);

    m_engine = dataEngine("microblog");
    if (! m_engine->isValid()) {
        setFailedToLaunch(true, i18n("Failed to load twitter DataEngine"));
        return m_graphicsWidget;
    }

    //ui setup
    m_layout = new QGraphicsLinearLayout( Qt::Vertical, m_graphicsWidget );
    m_layout->setSpacing( 3 );

    QGraphicsLinearLayout *flashLayout = new QGraphicsLinearLayout( Qt::Horizontal );
    flashLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_flash->setAutohide( true );
    m_flash->setMinimumSize( 0, 20 );
    m_flash->setColor( Plasma::Theme::TextColor );
    QFont fnt = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    fnt.setBold( true );
    QFontMetrics fm( fnt );
    m_flash->setFont( fnt );
    m_flash->flash( "", 20000 );
    m_flash->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QGraphicsLinearLayout *titleLayout = new QGraphicsLinearLayout(Qt::Vertical);
    Plasma::SvgWidget *svgTitle = new Plasma::SvgWidget(m_theme, "microblog", this);
    svgTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    svgTitle->setPreferredSize(m_theme->elementSize("microblog"));
    titleLayout->addItem(svgTitle);

    flashLayout->addItem(m_flash);
    flashLayout->addItem(titleLayout);

    m_layout->addItem(flashLayout);

    Plasma::Frame *headerFrame = new Plasma::Frame(this);
    m_headerLayout = new QGraphicsLinearLayout( Qt::Horizontal, headerFrame );
    m_headerLayout->setContentsMargins( 5, 5, 5, 10 );
    m_headerLayout->setSpacing( 5 );
    m_layout->addItem( headerFrame );


    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon(KIcon("user-identity"));
    m_icon->setText(m_username);
    m_icon->setTextBackgroundColor(QColor());
    QSizeF iconSize = m_icon->sizeFromIconSize(48);
    m_icon->setMinimumSize( iconSize );
    m_icon->setMaximumSize( iconSize );
    m_headerLayout->addItem( m_icon );

    Plasma::Frame *statusEditFrame = new Plasma::Frame(this);

    statusEditFrame->setMaximumHeight(fm.height() * 4);
    statusEditFrame->setFrameShadow(Plasma::Frame::Sunken);
    QGraphicsLinearLayout *statusEditLayout = new QGraphicsLinearLayout(statusEditFrame);
    m_statusEdit = new Plasma::TextEdit(this);

    connect(m_statusEdit, SIGNAL(textChanged()), this, SLOT(editTextChanged()));
    statusEditLayout->addItem(m_statusEdit);

    //FIXME: m_statusEdit->setTextColor( m_colorScheme->foreground().color() );
    // seems to have no effect
    QPalette editPal = m_statusEdit->palette();
    editPal.setColor(QPalette::Text, m_colorScheme->foreground().color());
    m_statusEdit->nativeWidget()->setPalette(editPal);
    m_statusEdit->nativeWidget()->installEventFilter(this);
    m_headerLayout->addItem( statusEditFrame );

    m_scrollWidget = new Plasma::ScrollWidget(this);
    m_tweetsWidget = new QGraphicsWidget;
    m_scrollWidget->setWidget(m_tweetsWidget);
    m_tweetsLayout = new QGraphicsLinearLayout(Qt::Vertical, m_tweetsWidget);

    m_layout->addItem(m_scrollWidget);

    //hook up some sources
    m_imageQuery = "UserImages:"+m_serviceUrl;
    m_engine->connectSource(m_imageQuery, this);


    //set things in motion
    if (m_username.isEmpty()) {
        setAuthRequired(true);
    } else if (m_password.isEmpty()) {
        kDebug() << "started, password is not in config file, trying wallet";
        m_walletWait = Read;
        getWallet();
    } else { //use config value
        kDebug() << "password was in config file, using that to get twitter history";
        downloadHistory();
    }

    m_graphicsWidget->setPreferredSize(300, 400);
    return m_graphicsWidget;
}

void MicroBlog::getWallet()
{
    //TODO: maybe Plasma in general should handle the wallet
    delete m_wallet;

    QGraphicsView *v = view();
    WId w = 0;
    if (v) {
        w = v->winId();
    }

    kDebug() << "opening wallet";
    m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                           w, KWallet::Wallet::Asynchronous);

    if (m_walletWait == Write) {
        connect(m_wallet, SIGNAL(walletOpened(bool)), SLOT(writeWallet(bool)));
    } else {
        connect(m_wallet, SIGNAL(walletOpened(bool)), SLOT(readWallet(bool)));
    }
}

void MicroBlog::writeWallet(bool success)
{
    kDebug() << success;
    if (success &&
        enterWalletFolder(QString::fromLatin1("Plasma-MicroBlog")) &&
        (m_wallet->writePassword(m_username, m_password) == 0)) {
        kDebug() << "successfully put password in wallet, removing from config file";
        config().deleteEntry("password");
        emit configNeedsSaving();
    } else {
        kDebug() << "failed to store password in wallet, putting into config file instead";
        writeConfigPassword();
    }
    m_walletWait = None;
    delete m_wallet;
    m_wallet = 0;
}

void MicroBlog::readWallet(bool success)
{
    kDebug() << success;
    QString pwd;
    if (success &&
        enterWalletFolder(QString::fromLatin1("Plasma-MicroBlog")) &&
        (m_wallet->readPassword(m_username, pwd) == 0)) {
        kDebug() << "successfully retrieved password from wallet";
        m_password = pwd;
        downloadHistory();
    } else if (m_password.isEmpty()) {
        //FIXME: when out of string freeze, tell the user WHY they need
        //       to configure the widget;
        setConfigurationRequired(true);
        kDebug() << "failed to read password";
    }

    m_walletWait = None;
    delete m_wallet;
    m_wallet = 0;
}

bool MicroBlog::enterWalletFolder(const QString &folder)
{
    //TODO: seems a bit silly to have a function just for this here
    //why doesn't kwallet have this itself?
    m_wallet->createFolder(folder);
    if (! m_wallet->setFolder(folder)) {
        kDebug() << "failed to open folder" << folder;
        return false;
    }

    kDebug() << "wallet now on folder" << folder;
    return true;
}

void MicroBlog::setAuthRequired(bool required)
{
    setConfigurationRequired(required);
    m_statusEdit->setEnabled(!required);
}

void MicroBlog::writeConfigPassword()
{
    //kDebug();
    //TODO: don't use "Yes" and "No", but replace with meaningful labels!
    if (KMessageBox::warningYesNo(0, i18n("Failed to access kwallet. Store password in config file instead?"))
            == KMessageBox::Yes) {
        config().writeEntry("password", KStringHandler::obscure(m_password));
    }
}

void MicroBlog::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    //kDebug() << source << data.count() << m_curTimeline;
    if (data.isEmpty()) {
        if (source.startsWith("Error")) {
            m_flash->kill(); //FIXME only clear it if it was showing an error msg
        } else {
            //this is a fake update from a new source
            return;
        }
    }

    if (source == m_curTimeline) {
        m_flash->flash( i18n("Refreshing timeline...") );

        //add the newbies
        int newCount = 0;
        qulonglong maxId = m_lastTweet;
        foreach (const QString &id, data.keys()) {
            qulonglong i = id.toULongLong();
            //kDebug() << i << m_lastTweet;
            if (i > m_lastTweet) {
                newCount++;
                QVariant v = data.value(id);
                //Warning: This function is not available with MSVC 6
                Plasma::DataEngine::Data t = v.value<Plasma::DataEngine::Data>();
                m_tweetMap[i] = t;
                if (i > maxId) {
                    maxId = i;
                }
            }
        }
        //kDebug() << m_lastTweet << maxId << "<-- updated";
        m_lastTweet = maxId;
        m_newTweets = qMin(newCount, m_historySize);

        if (m_newTweets > 0) {
            m_flash->flash( i18np( "1 new tweet", "%1 new tweets", m_newTweets ), 20*1000 );
        }

        scheduleShowTweets();
    } else if (source == m_imageQuery) {
        foreach (const QString &user, data.keys()) {
            QPixmap pm = data[user].value<QPixmap>();

            if (!pm.isNull()) {
                if (user == m_username) {
                    QAction *profile = new QAction(QIcon(pm), m_username, this);
                    profile->setData(m_username);

                    QSizeF iconSize = m_icon->sizeFromIconSize(48);
                    m_icon->setAction(profile);
                    m_icon->setMinimumSize( iconSize );
                    m_icon->setMaximumSize( iconSize );
                    connect(profile, SIGNAL(triggered(bool)), this, SLOT(openProfile()));
                }
                m_pictureMap[user] = pm;
                //TODO it would be nice to check whether the updated image is actually in use
                scheduleShowTweets();
            }
        }
    } else if (source.startsWith("Error")) {
        QString desc = data["description"].toString();

        if (desc == "Authentication required"){
            setAuthRequired(true);
        }

        m_flash->flash(desc, 60 * 1000); //I'd really prefer it to stay there. and be red.
    }
    //updateGeometry();
}

void MicroBlog::themeChanged()
{
    delete m_colorScheme;
    m_colorScheme = new KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());
    showTweets();
}

void MicroBlog::scheduleShowTweets()
{
    if (!m_showTweetsTimer) {
        m_showTweetsTimer = new QTimer(this);
        m_showTweetsTimer->setInterval(100);
        m_showTweetsTimer->setSingleShot(true);
        connect(m_showTweetsTimer, SIGNAL(timeout()), this, SLOT(showTweets()));
    }

    m_showTweetsTimer->stop();
    m_showTweetsTimer->start();
}

void MicroBlog::showTweets()
{
    prepareGeometryChange();
    // Adjust the number of the TweetWidgets if the configuration has changed
    // Add more tweetWidgets if there are not enough

    if (m_tweetMap.count() > m_historySize) {
        QMap<qulonglong, Plasma::DataEngine::Data>::iterator it = m_tweetMap.begin();
        while (it != m_tweetMap.end() && m_tweetMap.count() > m_historySize) {
            it = m_tweetMap.erase(it);
        }
    }

    while (m_tweetWidgets.count() < m_tweetMap.count()) {
        Plasma::Frame *tweetFrame = new Plasma::Frame(m_tweetsWidget);
        tweetFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QGraphicsLinearLayout *tweetLayout = new QGraphicsLinearLayout( Qt::Horizontal, tweetFrame );
        tweetLayout->setContentsMargins( 0, 5, 0, 5 );
        tweetLayout->setSpacing( 5 );
        //m_layout->insertItem( m_layout->count()-1, tweetFrame );
        m_tweetsLayout->addItem(tweetFrame);

        Plasma::TextBrowser *tweetText = new Plasma::TextBrowser(tweetFrame);
        tweetText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        tweetText->nativeWidget()->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        tweetText->nativeWidget()->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        tweetText->nativeWidget()->setCursor( Qt::ArrowCursor );

        Plasma::IconWidget *icon = new Plasma::IconWidget(tweetFrame);
        QSizeF iconSize = icon->sizeFromIconSize(30);
        icon->setMinimumSize( iconSize );
        icon->setMaximumSize( iconSize );

        Plasma::IconWidget *favIcon = new Plasma::IconWidget(tweetFrame);
        QSizeF favIconSize = icon->sizeFromIconSize(16);
        favIcon->setMinimumSize( favIconSize );
        favIcon->setMaximumSize( favIconSize );

        tweetLayout->addItem(icon);
        tweetLayout->addItem(tweetText);
        tweetLayout->addItem(favIcon);

        Tweet t;
        t.frame = tweetFrame;
        t.icon = icon;
        t.content = tweetText;
        t.favIcon = favIcon;

        m_tweetWidgets.append( t );
    }

    //clear out tweet widgets if there are too many
    while (m_tweetWidgets.count() > m_tweetMap.count()) {
        Tweet t = m_tweetWidgets[m_tweetWidgets.size() - 1];
        m_layout->removeItem(t.frame);
        QAction *iconAction = t.icon->action();
        t.icon->setAction(0);
        delete iconAction;
        delete t.frame;
        m_tweetWidgets.removeAt(m_tweetWidgets.size() - 1);
    }

    int i = 0;
    QMap<qulonglong, Plasma::DataEngine::Data>::iterator it = m_tweetMap.end();
    while (it != m_tweetMap.begin()) {
        Plasma::DataEngine::Data &tweetData = *(--it);
        QString user = tweetData.value("User").toString();
        QPixmap favIcon = tweetData.value("SourceFavIcon").value<QPixmap>();

        if (i == 0) {
            m_popupIcon = m_pictureMap[user];
        }

        QAction *profile = new QAction(QIcon(m_pictureMap[user]), QString(), this);
        profile->setData(user);

        Tweet t = m_tweetWidgets[i];
        t.icon->setAction(profile);
        QSizeF iconSize = t.icon->sizeFromIconSize(30);
        t.icon->setMinimumSize(iconSize);
        t.icon->setMaximumSize(iconSize);
        connect(profile, SIGNAL(triggered(bool)), this, SLOT(openProfile()));

        QString sourceString;

        if (favIcon.isNull()) {
            sourceString = i18n(" from %1", tweetData.value( "Source" ).toString());
        }

        QLocale english(QLocale::English, QLocale::UnitedStates);
        QDateTime dt = english.toDateTime(tweetData.value( "Date" ).toString(), "ddd MMM dd HH:mm:ss +0000 yyyy");
        dt.setTimeSpec(Qt::UTC);
        QString html = "<table cellspacing='0' spacing='5' width='100%'>";
        html += QString("<tr height='1em'><td align='left' width='1%'><font color='%2'>%1</font></td><td align='right' width='auto'><p align='right'><font color='%2'>%3%4</font></p></td></tr></table>").arg( user).arg(m_colorScheme->foreground(KColorScheme::InactiveText).color().name())
                .arg(timeDescription( dt )).arg( sourceString);
        QString status = tweetData.value( "Status" ).toString();

        status.replace(QRegExp("((http|https)://[^\\s<>'\"]+[^!,\\.\\s<>'\"\\]])"), "<a href='\\1'>\\1</a>");

        html += QString( "<p><font color='%1'>%2</font></p>" )
                .arg( m_colorScheme->foreground().color().name()).arg( status );

        t.content->setText( html );
        t.content->nativeWidget()->document()->setDefaultStyleSheet(QString("a{color:%1} a:visited{color:%2}")
                                            .arg( m_colorScheme->foreground(KColorScheme::LinkText).color().name())
                                            .arg(m_colorScheme->foreground(KColorScheme::VisitedText).color().name()));


        t.content->update();

        if( !favIcon.isNull() ) {
            t.favIcon->setIcon( QIcon(favIcon) );
        }

        ++i;
    }

    //are we complete?
    if (layout()->itemAt(0) != m_graphicsWidget) {
        paintIcon();
    }
}

void MicroBlog::createConfigurationInterface(KConfigDialog *parent)
{
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    QWidget *configWidget = new QWidget();
    configUi.setupUi(configWidget);

    connect(configUi.historySizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpinBoxSuffix()));
    connect(configUi.historyRefreshSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpinBoxSuffix()));

    configUi.serviceUrlCombo->addItem("https://identi.ca/api/");
    configUi.serviceUrlCombo->addItem("https://twitter.com/");
    configUi.serviceUrlCombo->setEditText(m_serviceUrl);
    configUi.usernameEdit->setText(m_username);
    configUi.passwordEdit->setText(m_password);
    configUi.historySizeSpinBox->setValue(m_historySize);
    configUi.historyRefreshSpinBox->setValue(m_historyRefresh);
    configUi.checkIncludeFriends->setChecked(m_includeFriends);
    updateSpinBoxSuffix();

    parent->addPage(configWidget, i18n("General"), icon());
}

void MicroBlog::updateSpinBoxSuffix()
{
    configUi.historySizeSpinBox->setSuffix(i18np(" message", " messages", configUi.historySizeSpinBox->value()));
    configUi.historyRefreshSpinBox->setSuffix(i18np(" minute", " minutes", configUi.historyRefreshSpinBox->value()));
}

void MicroBlog::configAccepted()
{
    QString serviceUrl = configUi.serviceUrlCombo->currentText();
    QString username = configUi.usernameEdit->text();
    QString password = configUi.passwordEdit->text();
    int historyRefresh = configUi.historyRefreshSpinBox->value();
    int historySize = configUi.historySizeSpinBox->value();
    bool includeFriends = configUi.checkIncludeFriends->isChecked();
    bool changed = false;
    bool reloadRequired = false;

    KConfigGroup cg = config();

    m_engine->disconnectSource(m_imageQuery, this);

    if (m_serviceUrl != serviceUrl) {
        changed = true;
        reloadRequired = true;
        m_serviceUrl = serviceUrl;
        cg.writeEntry( "serviceUrl", m_serviceUrl );
    }


    if (m_username != username) {
        changed = true;
        reloadRequired = true;
        m_username = username;
        m_icon->setIcon( QIcon() );
        m_icon->setText( m_username );
        cg.writeEntry( "username", m_username );
        m_tweetMap.clear();
        m_lastTweet = 0;
    }

    m_imageQuery = "UserImages:"+m_serviceUrl;
    m_engine->connectSource(m_imageQuery, this);

    if (m_password != password) {
        changed = true;
        m_password = password;
    }

    //if (m_walletWait == Read)
    //then the user is a dumbass.
    //we're going to ignore that, which drops the read attempt
    //I hope that doesn't cause trouble.
    if (!m_username.isEmpty() && (changed || m_password.isEmpty())) {
        //a change in name *or* pass means we need to update the wallet
        //if the user doesn't set a password, see if it's already in our wallet
        m_walletWait = m_password.isEmpty() ? Read : Write;
        getWallet();
    }

    if (m_historyRefresh != historyRefresh) {
        changed = true;
        m_historyRefresh = historyRefresh;
        cg.writeEntry("historyRefresh", m_historyRefresh);
    }

    if (m_includeFriends != includeFriends) {
        changed = true;
        reloadRequired = true;
        m_includeFriends = includeFriends;
        m_tweetMap.clear();
        m_lastTweet = 0;
        cg.writeEntry("includeFriends", m_includeFriends);
    }

    if (m_historySize != historySize) {
        //kDebug() << m_historySize << historySize;
        if (m_historySize < historySize) {
            reloadRequired = true;
        } else if (!reloadRequired) {
            showTweets();
        }

        changed = true;
        m_historySize = historySize;
        cg.writeEntry("historySize", m_historySize);
    }

    //kDebug() << changed << reloadRequired;
    if (changed) {
        if (reloadRequired) {
            m_lastTweet = 0;
            downloadHistory();
        }

        emit configNeedsSaving();
    }

    setAuthRequired(m_username.isEmpty());
}

MicroBlog::~MicroBlog()
{
    delete m_colorScheme;
    delete m_service;
    delete m_profileService;
    delete m_statusService;
}

void MicroBlog::editTextChanged()
{
    m_flash->flash(i18np("%1 character left", "%1 characters left", 140 - m_statusEdit->nativeWidget()->toPlainText().length()), 2000);
}

bool MicroBlog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_statusEdit->nativeWidget()) {
        //FIXME:it's nevessary this eventfilter to intercept keypresses in
        // QTextEdit (or KTextedit) is it intended?
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            //use control modifiers to allow multiline input (even if twitter seems to flatten everything to a slingle line)
            if (!(keyEvent->modifiers() & Qt::ControlModifier) &&
                (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)) {
                updateStatus();
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return Plasma::Applet::eventFilter(obj, event);
    }
}

void MicroBlog::updateStatus()
{
    QString status = m_statusEdit->nativeWidget()->toPlainText();

    if (!m_statusService) {
        m_statusService = m_engine->serviceForSource(m_curTimeline);
    }

    KConfigGroup cg = m_service->operationDescription("update");
    cg.writeEntry("password", m_password);
    cg.writeEntry("status", status);
    m_service->startOperationCall(cg);
    //m_statusUpdates.insert(m_service->startOperationCall(cg), status);
    connect(m_service, SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(updateCompleted(Plasma::ServiceJob*)));
    connect(m_service, SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(serviceFinished(Plasma::ServiceJob*)));

    m_statusEdit->nativeWidget()->setPlainText("");
}

void MicroBlog::updateCompleted(Plasma::ServiceJob *job)
{
    if (!job->error()) {
        //m_statusUpdates.value(job);
        downloadHistory();
    }

    //m_statusUpdates.remove(job);
}

//what this really means now is 'reconnect to the timeline source'
void MicroBlog::downloadHistory()
{
    //kDebug() ;
    if (m_username.isEmpty() || m_password.isEmpty()) {
        if (!m_curTimeline.isEmpty()) {
            m_engine->disconnectSource(m_curTimeline, this);
            m_engine->disconnectSource("Error:" + m_curTimeline, this);
        }
        return;
    }

    m_flash->flash(i18n("Refreshing timeline..."), -1);

    QString query;
    if (m_includeFriends) {
        query = QString("TimelineWithFriends:%1@%2");
    } else {
        query = QString("Timeline:%1@%2");
    }

    query = query.arg(m_username, m_serviceUrl);
    //kDebug() << m_curTimeline << query;
    bool newQuery = m_curTimeline != query;
    if (newQuery) {
        //ditch the old one, if needed
        if (!m_curTimeline.isEmpty()) {
            m_engine->disconnectSource(m_curTimeline, this);
            m_engine->disconnectSource("Error:" + m_curTimeline, this);
        }

        delete m_statusService;
        m_statusService = 0;

        m_curTimeline = query;
    }

    //kDebug() << "Connecting to source " << query << "with refresh rate" << m_historyRefresh * 60 * 1000;
    m_engine->connectSource(query, this, m_historyRefresh * 60 * 1000);
    m_engine->connectSource("Error:" + query, this);

    if (m_service) {
        m_service->deleteLater();
    }
    m_service = m_engine->serviceForSource(m_curTimeline);
    connect(m_service, SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(serviceFinished(Plasma::ServiceJob*)));
    KConfigGroup cg = m_service->operationDescription("auth");
    cg.writeEntry("password", m_password);
    m_service->startOperationCall(cg);

    //get the profile to retrieve the user icon
    QString profileQuery(QString("Profile:%1@%2").arg(m_username, m_serviceUrl));

    m_engine->connectSource(m_imageQuery, this);
    m_engine->connectSource(profileQuery, this, m_historyRefresh * 60 * 1000);

    if (m_profileService) {
        m_profileService->deleteLater();
    }
    m_profileService = m_engine->serviceForSource(profileQuery);
    connect(m_profileService, SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(serviceFinished(Plasma::ServiceJob*)));
    KConfigGroup profileConf = m_profileService->operationDescription("auth");
    profileConf.writeEntry("password", m_password);
    m_profileService->startOperationCall(profileConf);
}

void MicroBlog::serviceFinished(Plasma::ServiceJob *job)
{
    if (job->error()) {
        m_flash->flash(job->errorString(), 2000);
    }
}

void MicroBlog::openProfile()
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (action) {
        QString url = m_serviceUrl;
        url.remove("api/");

        KRun::runUrl( KUrl(KUrl(url), action->data().toString()), "text/html", 0 );
    }
}

QString MicroBlog::timeDescription( const QDateTime &dt )
{
    int diff = dt.secsTo(KDateTime::currentDateTime(m_tz).dateTime());

    if (diff < 60) {
        return i18n("Less than a minute ago");
    } else if (diff < 60*60) {
        return i18np("1 minute ago", "%1 minutes ago", diff/60);
    } else if (diff < 2*60*60) {
        return i18n("Over an hour ago");
    } else if (diff < 24*60*60) {
        return i18np("1 hour ago", "%1 hours ago", diff/3600);
    }

    return dt.toString(Qt::LocaleDate);
}

#include "microblog.moc"
