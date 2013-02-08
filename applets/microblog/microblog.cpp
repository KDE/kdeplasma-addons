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
#include "postwidget.h"

#include <QFontMetrics>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>
#include <QGraphicsAnchorLayout>
#include <QAction>
#include <QTimer>

#include <KColorScheme>
#include <KConfigDialog>
#include <KDebug>
#include <KIcon>
#include <KMessageBox>
#include <KStringHandler>
#include <KTextBrowser>
#include <KTextEdit>
#include <KWallet/Wallet>
#include <KToolInvocation>

#include <Plasma/Label>
#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>
#include <Plasma/Service>
#include <Plasma/FlashingLabel>
#include <Plasma/IconWidget>
#include <Plasma/SvgWidget>
#include <Plasma/TabBar>
#include <Plasma/TextBrowser>
#include <Plasma/ToolTipManager>
#include <Plasma/ScrollWidget>
#include <Plasma/TextEdit>
#include <Plasma/Frame>
#include <Plasma/ServiceJob>


Q_DECLARE_METATYPE(Plasma::DataEngine::Data)

MicroBlog::MicroBlog(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_graphicsWidget(0),
      m_historySize(0),
      m_historyRefresh(0),
      m_newTweets(0),
      m_includeFriends(false),
      m_lastMode(0),
      m_profileService(0),
      m_lastTweet(0),
      m_wallet(0),
      m_walletWait(None),
      m_colorScheme(0),
      m_showTweetsTimer(0),
      m_getWalletDelayTimer(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setPopupIcon("view-pim-journal");
    setAssociatedApplication("choqok");
}

void MicroBlog::init()
{
    m_engine = dataEngine("microblog");
    m_flash = new Plasma::FlashingLabel(this);
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/microblog");
    m_theme->setContainsMultipleImages(true);
    configChanged();
}

void MicroBlog::constraintsEvent(Plasma::Constraints constraints)
{
    //i am an icon?
    if ((constraints|Plasma::SizeConstraint || constraints|Plasma::FormFactorConstraint) &&
        layout() && layout()->itemAt(0) != m_graphicsWidget) {
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

    //ui setup
    m_layout = new QGraphicsLinearLayout( Qt::Vertical, m_graphicsWidget );
    m_layout->setSpacing( 3 );

    QGraphicsLinearLayout *flashLayout = new QGraphicsLinearLayout( Qt::Horizontal );
    flashLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_flash->setAutohide( true );
    m_flash->setMinimumSize( 0, 20 );
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

    m_headerFrame = new Plasma::Frame(this);
    m_headerLayout = new QGraphicsAnchorLayout(m_headerFrame);
    m_headerLayout->setSpacing( 5 );

    m_icon = new Plasma::IconWidget(m_headerFrame);
    m_icon->setIcon(KIcon("user-identity"));
    m_icon->setText(m_username);
    m_icon->setTextBackgroundColor(QColor());
    QSizeF iconSize = m_icon->sizeFromIconSize(48);
    m_icon->setMinimumSize( iconSize );
    m_icon->setMaximumSize( iconSize );
    m_headerLayout->addAnchor(m_icon, Qt::AnchorVerticalCenter, m_headerLayout, Qt::AnchorVerticalCenter);
    m_headerLayout->addAnchor(m_icon, Qt::AnchorLeft, m_headerLayout, Qt::AnchorLeft);

    Plasma::Frame *statusEditFrame = new Plasma::Frame(m_headerFrame);

    statusEditFrame->setFrameShadow(Plasma::Frame::Sunken);
    QGraphicsLinearLayout *statusEditLayout = new QGraphicsLinearLayout(statusEditFrame);
    m_statusEdit = new Plasma::TextEdit(this);
    m_statusEdit->setPreferredHeight(fm.height() * 4);
    m_statusEdit->setEnabled(!configurationRequired());

    statusEditLayout->addItem(m_statusEdit);

    //FIXME: m_statusEdit->setTextColor( m_colorScheme->foreground().color() );
    // seems to have no effect
    QPalette editPal = m_statusEdit->palette();
    editPal.setColor(QPalette::Text, m_colorScheme->foreground().color());
    m_statusEdit->nativeWidget()->setPalette(editPal);
    m_statusEdit->nativeWidget()->installEventFilter(this);
    m_headerLayout->addAnchor(m_icon, Qt::AnchorRight, statusEditFrame, Qt::AnchorLeft);
    m_headerLayout->addAnchors(statusEditFrame, m_headerLayout, Qt::Vertical);
    m_headerLayout->addAnchor(statusEditFrame, Qt::AnchorRight, m_headerLayout, Qt::AnchorRight);
    m_headerLayout->activate();
    m_headerLayout->setMaximumHeight(m_headerLayout->effectiveSizeHint(Qt::PreferredSize).height());

    m_tabBar = new Plasma::TabBar(this);
    m_tabBar->addTab(i18n("Timeline"));
    m_tabBar->addTab(i18n("Replies"));
    m_tabBar->addTab(i18n("Messages"));
    m_layout->addItem(m_tabBar);

    m_scrollWidget = new Plasma::ScrollWidget(this);
    m_tweetsWidget = new QGraphicsWidget(m_scrollWidget);
    m_scrollWidget->setWidget(m_tweetsWidget);
    m_tweetsLayout = new QGraphicsLinearLayout(Qt::Vertical, m_tweetsWidget);
    m_tweetsLayout->setSpacing(0);
    m_tweetsLayout->addItem(m_headerFrame);

    m_layout->addItem(m_scrollWidget);

    m_graphicsWidget->setPreferredSize(300, 400);

    if(m_engine->isValid()) {
        connect(m_statusEdit, SIGNAL(textChanged()), this, SLOT(editTextChanged()));
        connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(modeChanged(int)));

        m_tabBar->nativeWidget()->installEventFilter(this);

        if (!m_imageQuery.isEmpty()) {
            m_engine->connectSource(m_imageQuery, this);
        }
    }
    else {
        const QString failureMessage = i18n("Failed to load twitter DataEngine");
        QGraphicsWidget *failureWidget = new QGraphicsWidget(this);
        QGraphicsLinearLayout *failureLayout = new QGraphicsLinearLayout(failureWidget);
        Plasma::IconWidget *failureIcon = new Plasma::IconWidget(this);
        Plasma::Label *failureLabel = new Plasma::Label(this);

        failureLayout->setContentsMargins(0, 0, 0, 0);
        failureIcon->setIcon(KIcon("dialog-error"));
        failureLayout->addItem(failureIcon);
        failureLabel->setText(failureMessage);
        failureLabel->nativeWidget()->setWordWrap(true);
        failureLayout->addItem(failureLabel);

        Plasma::ToolTipManager::self()->registerWidget(failureIcon);
        Plasma::ToolTipContent data(i18n("Unable to load the widget"), failureMessage, KIcon("dialog-error"));
        Plasma::ToolTipManager::self()->setContent(failureIcon, data);

        m_tweetsLayout->addItem(failureWidget);
    }

    return m_graphicsWidget;
}

void MicroBlog::configChanged()
{
    //config stuff
    KConfigGroup cg = config();
    QString serviceUrl = cg.readEntry("serviceUrl", "https://identi.ca/api/");
    QString username = cg.readEntry("username");
    QString password = KStringHandler::obscure(cg.readEntry("password"));
    int historySize = cg.readEntry("historySize", 6);
    int historyRefresh = cg.readEntry("historyRefresh", 5);
    bool includeFriends = cg.readEntry("includeFriends", true);
    bool changed = false;
    bool reloadRequired = false;

    if (m_serviceUrl != serviceUrl) {
        m_serviceUrl = serviceUrl;
        changed = true;
        reloadRequired = true;
    }

    if (username != m_username) {
        m_username = username;
        changed = true;
        reloadRequired = true;
        m_tweetMap.clear();
        m_lastTweet = 0;

        if (m_graphicsWidget) {
            m_icon->setIcon(KIcon("user-identity"));
            m_icon->setText(m_username);
        }
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
        reloadRequired = true;
    }

    if (m_historyRefresh != historyRefresh) {
        changed = true;
        m_historyRefresh = historyRefresh;
    }

    if (m_includeFriends != includeFriends) {
        changed = true;
        reloadRequired = true;
        m_includeFriends = includeFriends;
        m_tweetMap.clear();
        m_lastTweet = 0;
    }

    if (m_historySize != historySize) {
        //kDebug() << "m_historysize,historysize: "<<m_historySize << historySize;
        if (m_historySize < historySize) {
            reloadRequired = true;
        } else if (!reloadRequired) {
            showTweets();
        }

        changed = true;
        m_historySize = historySize;
    }

    //kDebug() << changed << reloadRequired;
    //set things in motion
    //hook up some sources
    if (!m_imageQuery.isEmpty()) {
        m_engine->disconnectSource(m_imageQuery, this);
    }

    m_imageQuery = "UserImages:" + m_serviceUrl;
    if (m_graphicsWidget) {
        m_engine->connectSource(m_imageQuery, this);
    }

    if (m_username.isEmpty()) {
        setAuthRequired(true);
    } else if (m_password.isEmpty()) {
        //kDebug() << "started, password is not in config file, trying wallet";
        m_walletWait = Read;
        getWallet();
    } else { //use config value
        //kDebug() << "password was in config file, using that to get twitter history";
        reloadRequired = true;
    }

    if (changed) {
        if (m_service) {
            m_service.data()->deleteLater();
        }

        if (m_profileService) {
            m_profileService->deleteLater();
            m_profileService = 0;
        }
    }

    if (reloadRequired) {
        m_lastTweet = 0;
        downloadHistory();
    }

    setAuthRequired(m_username.isEmpty());
}

void MicroBlog::modeChanged(int)
{
    m_tweetMap.clear();
    m_lastTweet = 0;
    downloadHistory();
}

void MicroBlog::reply(const QString &replyToId, const QString &to)
{
    m_replyToId = replyToId;
    m_scrollWidget->ensureItemVisible(m_headerFrame);
    m_statusEdit->nativeWidget()->setPlainText(to);
    QTextCursor cursor = m_statusEdit->nativeWidget()->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_statusEdit->nativeWidget()->setTextCursor(cursor);
    m_statusEdit->setFocus();
}

void MicroBlog::forward(const QString &messageId)
{
    createTimelineService();

    if (!m_service) {
        return;
    }

    KConfigGroup cg = m_service.data()->operationDescription("statuses/retweet");
    cg.writeEntry("id", messageId);

    connect(m_service.data(), SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(retweetCompleted(Plasma::ServiceJob*)), Qt::UniqueConnection);

    m_retweetJobs.insert(m_service.data()->startOperationCall(cg));
    setBusy(true);
}

void MicroBlog::favorite(const QString &messageId, const bool isFavorite)
{
    QString operation;
    if (isFavorite) {
        operation = "favorites/create";
    } else {
        operation = "favorites/destroy";
    }

    KConfigGroup cg = m_service.data()->operationDescription(operation);
    cg.writeEntry("id", messageId);

    connect(m_service.data(), SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(favoriteCompleted(Plasma::ServiceJob*)), Qt::UniqueConnection);

    m_favoriteJobs.insert(m_service.data()->startOperationCall(cg));
    setBusy(true);
}

void MicroBlog::getWallet()
{
    //TODO: maybe Plasma in general should handle the wallet
    delete m_wallet;
    m_wallet = 0;

    QGraphicsView *v = view();
    WId w = 0;
    if (v) {
        w = v->winId();
    }

    if (!w) {
        //KWallet requires a valid window id to work, wait until we have one
        if (!m_getWalletDelayTimer) {
            m_getWalletDelayTimer = new QTimer(this);
            m_getWalletDelayTimer->setSingleShot(true);
            m_getWalletDelayTimer->setInterval(100);
            connect(m_getWalletDelayTimer, SIGNAL(timeout()), this, SLOT(getWallet()));
        }

        if (!m_getWalletDelayTimer->isActive()) {
            m_getWalletDelayTimer->start();
        }

        return;
    } else {
        delete m_getWalletDelayTimer;
        m_getWalletDelayTimer = 0;
    }

    //kDebug() << "opening wallet";
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
    //kDebug() << success;
    if (success &&
        enterWalletFolder(QString::fromLatin1("Plasma-MicroBlog")) &&
        (m_wallet->writePassword(m_username, m_password) == 0)) {
        //kDebug() << "successfully put password in wallet, removing from config file";
        config().deleteEntry("password");
        emit configNeedsSaving();
    } else {
        //kDebug() << "failed to store password in wallet, putting into config file instead";
        writeConfigPassword();
    }
    m_walletWait = None;
    delete m_wallet;
    m_wallet = 0;
}

void MicroBlog::readWallet(bool success)
{
    //kDebug() << success;
    QString pwd;
    if (success &&
        enterWalletFolder(QString::fromLatin1("Plasma-MicroBlog")) &&
        (m_wallet->readPassword(m_username, pwd) == 0)) {
        //kDebug() << "successfully retrieved password from wallet";
        m_password = pwd;
        downloadHistory();
    } else if (m_password.isEmpty()) {
        //FIXME: when out of string freeze, tell the user WHY they need
        //       to configure the widget;
		m_password = KStringHandler::obscure(config().readEntry("password"));
		if(m_password.isEmpty()){
	        setConfigurationRequired(true, i18n("Your password is required."));
		}else{
			//kDebug() << "reading from config";
        	//kDebug() << "failed to read password";
			downloadHistory();
		}
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
        //kDebug() << "failed to open folder" << folder;
        return false;
    }

    //kDebug() << "wallet now on folder" << folder;
    return true;
}

void MicroBlog::setAuthRequired(bool required)
{
    setConfigurationRequired(required, i18n("Your account information is incomplete."));
    if (m_graphicsWidget) {
        m_statusEdit->setEnabled(!required);
    }
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
        if (source.startsWith(QLatin1String("Error"))) {
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
            }

            QVariant v = data.value(id);
            //Warning: This function is not available with MSVC 6
            Plasma::DataEngine::Data t = v.value<Plasma::DataEngine::Data>();
            m_tweetMap[i] = t;

            if (i > maxId) {
                maxId = i;
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
                if (!user.compare(m_username, Qt::CaseInsensitive)) {
                    QAction *profile = new QAction(QIcon(pm), m_username, this);
                    profile->setData(m_username);

                    QSizeF iconSize = m_icon->sizeFromIconSize(48);
                    m_icon->setAction(profile);
                    m_icon->setMinimumSize( iconSize );
                    m_icon->setMaximumSize( iconSize );
                    connect(profile, SIGNAL(triggered()), this, SLOT(openProfile()));
                }
                m_pictureMap[user] = pm;
                m_avatarHistory.removeAll(user);
                m_avatarHistory.append(user);
                while (m_avatarHistory.size() > 30) {
                    QString oldestUser = m_avatarHistory.first();
                    m_avatarHistory.pop_front();
                    m_pictureMap.remove(oldestUser);
                }
                //TODO it would be nice to check whether the updated image is actually in use
                scheduleShowTweets();
            }
        }
    } else if (source.startsWith(QLatin1String("Error"))) {
        QString desc = data["description"].toString();

        if (desc == "Authentication required"){
            setAuthRequired(true);
        }

        m_flash->flash(desc, 60 * 1000); //I'd really prefer it to stay there. and be red.
    }
    m_graphicsWidget->setPreferredSize(-1, -1);
    emit sizeHintChanged(Qt::PreferredSize);
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
    if (!m_graphicsWidget) {
        return;
    }

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
        PostWidget *postWidget = new PostWidget(m_tweetsWidget);
        connect(postWidget, SIGNAL(reply(QString,QString)), this, SLOT(reply(QString,QString)));
        connect(postWidget, SIGNAL(forward(QString)), this, SLOT(forward(QString)));
        connect(postWidget, SIGNAL(favorite(QString,bool)), this, SLOT(favorite(QString,bool)));
        connect(postWidget, SIGNAL(openProfile(QString)), this, SLOT(openProfile(QString)));
        m_tweetWidgets.append(postWidget);
    }

    //clear out tweet widgets if there are too many
    while (m_tweetWidgets.count() > m_tweetMap.count()) {
        PostWidget *t = m_tweetWidgets[m_tweetWidgets.size() - 1];
        m_layout->removeItem(t);
        m_tweetWidgets.removeAt(m_tweetWidgets.size() - 1);
        t->deleteLater();
        m_tweetsWidget->resize(m_tweetsWidget->effectiveSizeHint(Qt::PreferredSize));
    }

    int i = 0;
    QMap<qulonglong, Plasma::DataEngine::Data>::iterator it = m_tweetMap.end();

    while (it != m_tweetMap.begin()) {
        Plasma::DataEngine::Data &tweetData = *(--it);

        QString user = tweetData.value("User").toString();

        if (i == 0) {
            m_popupIcon = m_pictureMap[user];
        }

        PostWidget *t = m_tweetWidgets[i];
        t->setColorScheme(m_colorScheme);
        t->setData(tweetData);
        t->setPicture(m_pictureMap[user]);
        //you don't want to reply or RT yourself
        t->setActionsShown(user != m_username);
        m_tweetsLayout->addItem(t);

        ++i;
    }

    //are we complete?
    if (layout() && layout()->itemAt(0) != m_graphicsWidget) {
        paintIcon();
    }
}

void MicroBlog::createConfigurationInterface(KConfigDialog *parent)
{
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    QWidget *configWidget = new QWidget();
    configUi.setupUi(configWidget);

    configUi.serviceUrlCombo->addItem("https://identi.ca/api/");
    configUi.serviceUrlCombo->addItem("https://api.twitter.com/1/");
    configUi.serviceUrlCombo->setEditText(m_serviceUrl);
    configUi.usernameEdit->setText(m_username);
    configUi.passwordEdit->setText(m_password);
    configUi.historySizeSpinBox->setValue(m_historySize);
    configUi.historySizeSpinBox->setSuffix(ki18np(" message", " messages"));
    configUi.historyRefreshSpinBox->setValue(m_historyRefresh);
    configUi.historyRefreshSpinBox->setSuffix(ki18np(" minute", " minutes"));
    configUi.checkIncludeFriends->setChecked(m_includeFriends);

    parent->addPage(configWidget, i18n("General"), icon());
    connect(configUi.serviceUrlCombo, SIGNAL(editTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(configUi.usernameEdit, SIGNAL(userTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(configUi.passwordEdit, SIGNAL(userTextChanged(QString)), parent, SLOT(settingsModified()));
    connect(configUi.historySizeSpinBox, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(configUi.historyRefreshSpinBox, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(configUi.checkIncludeFriends, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
}

void MicroBlog::configAccepted()
{
    KConfigGroup cg = config();
	//kDebug()<<"Inside configAccepted";
	//kDebug()<<"username: "<<configUi.usernameEdit->text();
    cg.writeEntry("serviceUrl", configUi.serviceUrlCombo->currentText());
    cg.writeEntry("username", configUi.usernameEdit->text());
    cg.writeEntry("historyRefresh", configUi.historyRefreshSpinBox->value());
    cg.writeEntry("includeFriends", configUi.checkIncludeFriends->isChecked());
    cg.writeEntry("historySize", configUi.historySizeSpinBox->value());

    QString password = configUi.passwordEdit->text();
    if (m_password != password) {
        m_password = password;

        if (m_service) {
            m_service.data()->deleteLater();
        }

        if (m_profileService) {
            m_profileService->deleteLater();
            m_profileService = 0;
        }
    }

    emit configNeedsSaving();
}

MicroBlog::~MicroBlog()
{
    delete m_colorScheme;
    delete m_service.data();
    delete m_profileService;
}

void MicroBlog::editTextChanged()
{
    m_flash->flash(i18np("%1 character left", "%1 characters left", 140 - m_statusEdit->nativeWidget()->toPlainText().length()), 2000);
    //if the text has been cleared, discard
    if (m_statusEdit->nativeWidget()->toPlainText().length() == 0) {
        m_replyToId = QString();
    }
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
    } else if (obj == m_tabBar->nativeWidget() && event->type() == QEvent::MouseButtonPress) {
        m_scrollWidget->ensureItemVisible(m_headerFrame);
        m_statusEdit->setFocus();
        return false;
    } else {
        return Plasma::Applet::eventFilter(obj, event);
    }
}

void MicroBlog::updateStatus()
{
    createTimelineService();

    if (!m_service) {
        return;
    }

    QString status = m_statusEdit->nativeWidget()->toPlainText();

    KConfigGroup cg = m_service.data()->operationDescription("update");
    cg.writeEntry("status", status);
    if (!m_replyToId.isEmpty()) {
        cg.writeEntry("in_reply_to_status_id", m_replyToId);
    }

    connect(m_service.data(), SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(updateCompleted(Plasma::ServiceJob*)), Qt::UniqueConnection);

    m_updateJobs.insert(m_service.data()->startOperationCall(cg));
    m_statusEdit->nativeWidget()->setPlainText("");
    setBusy(true);
}

void MicroBlog::updateCompleted(Plasma::ServiceJob *job)
{
    if (!m_updateJobs.contains(job)) {
        return;
    }

    m_updateJobs.remove(job);
    if (m_updateJobs.isEmpty()) {
        disconnect(m_service.data(), SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(updateCompleted(Plasma::ServiceJob*)));
    }

    if (!job->error()) {
        //m_statusUpdates.value(job);
        downloadHistory();
    }

    //m_statusUpdates.remove(job);
    setBusy(false);
}

void MicroBlog::retweetCompleted(Plasma::ServiceJob *job)
{
    if (!m_retweetJobs.contains(job)) {
        return;
    }

    m_retweetJobs.remove(job);
    if (m_retweetJobs.isEmpty()) {
        disconnect(m_service.data(), SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(retweetCompleted(Plasma::ServiceJob*)));
    }

    if (!job->error()) {
        //m_statusUpdates.value(job);
        downloadHistory();
        m_flash->flash(i18nc("Repeat of the post also called retweet", "Repeat completed"));
    } else {
        m_flash->flash(i18n("Repeat failed"));
    }

    setBusy(false);
}

void MicroBlog::favoriteCompleted(Plasma::ServiceJob *job)
{
    if (!m_favoriteJobs.contains(job)) {
        return;
    }

    m_favoriteJobs.remove(job);
    if (m_favoriteJobs.isEmpty()) {
        disconnect(m_service.data(), SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(favoriteCompleted(Plasma::ServiceJob*)));
    }

    if (!job->error()) {
        //m_statusUpdates.value(job);
        downloadHistory();
    }

    setBusy(false);
}

//what this really means now is 'reconnect to the timeline source'
void MicroBlog::downloadHistory()
{
   // kDebug() << "Inside downloadhistory";
    if (m_username.isEmpty() || m_password.isEmpty()) {
        //kDebug() << "BOOHYA got empty password";
        if (!m_curTimeline.isEmpty()) {
            m_engine->disconnectSource(m_curTimeline, this);
            m_engine->disconnectSource("Error:" + m_curTimeline, this);
        }
        return;
    }

    m_flash->flash(i18n("Refreshing timeline..."), -1);

    createTimelineService();
    if (m_service) {
        KConfigGroup cg = m_service.data()->operationDescription("auth");
        cg.writeEntry("password", m_password);
        
        bool ok = m_service.data()->startOperationCall(cg);
        kDebug() << "operation OK" << ok;
    }

    //get the profile to retrieve the user icon

    if (m_profileService) {
        KConfigGroup cg = m_profileService->operationDescription("refresh");
        m_profileService->startOperationCall(cg);
    } else {
        QString profileQuery(QString("Profile:%1@%2").arg(m_username, m_serviceUrl));
        m_engine->connectSource(m_imageQuery, this);
        m_engine->connectSource(profileQuery, this, m_historyRefresh * 60 * 1000);
        m_profileService = m_engine->serviceForSource(profileQuery);
        connect(m_profileService, SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(serviceFinished(Plasma::ServiceJob*)));
        KConfigGroup profileConf = m_profileService->operationDescription("auth");
        profileConf.writeEntry("password", m_password);
        m_profileService->startOperationCall(profileConf);
    }
}

void MicroBlog::createTimelineService()
{
    if (!m_tabBar || (m_service && m_lastMode == m_tabBar->currentIndex())) {
        return;
    }

    delete m_service.data();
    m_lastMode = m_tabBar->currentIndex();

    QString query;
    switch (m_tabBar->currentIndex()) {
    case 2:
        query = "Messages:%1@%2";
        break;
    case 1:
        query = "Replies:%1@%2";
        break;
    default:
        if (m_includeFriends) {
            query = QString("TimelineWithFriends:%1@%2");
        } else {
            query = QString("Timeline:%1@%2");
        }
        break;
    }

    query = query.arg(m_username, m_serviceUrl);
    //kDebug() << m_curTimeline << query;
    if (m_curTimeline != query) {
        //ditch the old one, if needed
        if (!m_curTimeline.isEmpty()) {
            m_engine->disconnectSource(m_curTimeline, this);
            m_engine->disconnectSource("Error:" + m_curTimeline, this);
        }

        m_curTimeline = query;
    }

    //kDebug() << "Connecting to source " << query << "with refresh rate" << m_historyRefresh * 60 * 1000;
    m_engine->connectSource(query, this, m_historyRefresh * 60 * 1000);
    m_engine->connectSource("Error:" + query, this);
    m_service = m_engine->serviceForSource(m_curTimeline);
    connect(m_service.data(), SIGNAL(finished(Plasma::ServiceJob*)), this, SLOT(serviceFinished(Plasma::ServiceJob*)));
}

void MicroBlog::serviceFinished(Plasma::ServiceJob *job)
{
    if (job->error()) {
        m_flash->flash(job->errorString(), 2000);
        kDebug() << "Job failed.";

        // reset the service objects to give it a chance
        // to re-authenticate
        if (m_service) {
            m_service.data()->deleteLater();
        }

        if (m_profileService) {
            m_profileService->deleteLater();
            m_profileService = 0;
        }
    } else {
        kDebug() << "Job succeeded.";

    }
}

void MicroBlog::openProfile(const QString &profile)
{
    QString url = m_serviceUrl;
    url.remove("api/");
    if (!profile.isEmpty()) {
        KToolInvocation::invokeBrowser(KUrl(KUrl(url), profile).prettyUrl());
    } else {
        KToolInvocation::invokeBrowser(KUrl(KUrl(url), m_username).prettyUrl());
    }
}



#include "microblog.moc"
