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

#include "twitter.h"

#include <QApplication>
#include <QPainter>
#include <QGridLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QTextDocument>
#include <QCheckBox>
#include <QGradient>
#include <QFontMetrics>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>

#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KDialog>
#include <KLineEdit>
#include <KStringHandler>
#include <KWallet/Wallet>
#include <KMessageBox>
#include <KColorScheme>

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/flash.h>
#include <plasma/widgets/icon.h>

Q_DECLARE_METATYPE(Plasma::DataEngine::Data)

Twitter::Twitter(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0), m_lastTweet(0), m_wallet(0), m_walletWait(None)
{
    setHasConfigurationInterface(true);
    setContentSize(300,150); //should be an ok default
}

void Twitter::init()
{
    m_theme = new Plasma::Svg(this);
    m_theme.setImagePath("widgets/twitter");
    //config stuff
    KConfigGroup cg = config();
    m_username = cg.readEntry( "username" );
    m_password = KStringHandler::obscure( cg.readEntry( "password" ) );
    m_historySize = cg.readEntry( "historySize", 2 );
    m_historyRefresh = cg.readEntry( "historyRefresh", 5 );
    m_includeFriends = cg.readEntry( "includeFriends", true );

    m_engine = dataEngine("twitter");
    if (! m_engine->isValid()) {
        setFailedToLaunch(true, "Failed to load twitter DataEngine");
        return;
    }

    //ui setup
    m_layout = new QGraphicsLinearLayout( Qt::Vertical, this );
    m_layout->setMargin( 0 );
    m_layout->setSpacing( 0 );

    m_flash = new Plasma::Flash( this );
    m_flash->setColor( Qt::gray );
    QFont fnt = qApp->font();
    fnt.setBold( true );
    QFontMetrics fm( fnt );
    m_flash->resize( QSize(250, fm.height()) );
    m_flash->setMaximumSize( QSizeF(250, fm.height()+4 ) );
    m_flash->setFont( fnt );
    m_layout->addItem( m_flash );


    m_headerLayout = new QGraphicsLinearLayout( Qt::Horizontal );
    m_headerLayout->setMargin( 5 );
    m_headerLayout->setMargin( Plasma::BottomMargin, 10 );
    m_headerLayout->setSpacing( 5 );
    m_layout->addItem( m_headerLayout );


    m_icon = new Plasma::Icon( this );
    m_icon->setIcon( KIcon( "user-identity" ) );
    m_icon->setText( m_username );
    QSizeF iconSize = m_icon->sizeFromIconSize(48);
    m_icon->setMinimumSize( iconSize );
    m_icon->setMaximumSize( iconSize );
    m_headerLayout->addItem( m_icon );

    m_statusEdit = new Plasma::LineEdit( this );
    m_statusEdit->hide();
    m_statusEdit->setStyled( true );
    m_statusEdit->setTextWidth( 250 );
    connect( m_statusEdit->document(), SIGNAL(contentsChanged()), SLOT(geometryChanged()) ); //FIXME no such slot
    connect( m_statusEdit, SIGNAL(editingFinished()), SLOT(updateStatus()) );
    m_headerLayout->addItem( m_statusEdit );

    //hook up some sources
    m_engine->connectSource("LatestImage", this);
    m_engine->connectSource("Error:UserImages", this);
    m_engine->connectSource("Error", this);

    //set things in motion
    if(! m_username.isEmpty()) {
        if (m_password.isEmpty()) {
            m_walletWait = Read;
            getWallet();
        } else { //use config value
            setAuth();
        }
    }
}

void Twitter::getWallet()
{
    //TODO: maybe Plasma in general should handle the wallet
    if (m_wallet) {
        //user must be a dumbass. kill that old attempt.
        delete m_wallet;
    }
    QGraphicsView *v = view();
    WId w;
    if (!v) {
        kDebug() << "eek! no view!";
        w=0;
    } else {
        w=v->winId();
    }
    m_wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),
            w, KWallet::Wallet::Asynchronous );
    if (m_walletWait == Write) {
        connect( m_wallet, SIGNAL( walletOpened(bool) ), SLOT( writeWallet(bool) ) );
    } else {
        connect( m_wallet, SIGNAL( walletOpened(bool) ), SLOT( readWallet(bool) ) );
    }
}

void Twitter::writeWallet(bool success)
{
    //kDebug();
    if (!(success && enterWalletFolder(QString::fromLatin1("Plasma-Twitter"))
                && (m_wallet->writePassword(m_username, m_password) == 0))) {
        kDebug() << "failed to write password";
        writeConfigPassword();
    }
    m_walletWait = None;
    delete m_wallet;
    m_wallet = 0;
}

void Twitter::readWallet(bool success)
{
    //kDebug();
    QString pwd;
    if (success && enterWalletFolder(QString::fromLatin1("Plasma-Twitter")) 
            && (m_wallet->readPassword(m_username, pwd) == 0)) {
        m_password = pwd;
        setAuth();
    } else {
        kDebug() << "failed to read password";
    }
    m_walletWait = None;
    delete m_wallet;
    m_wallet = 0;
}

bool Twitter::enterWalletFolder(const QString &folder)
{
    //TODO: seems a bit silly to have a function just for this here
    //why doesn't kwallet have this itself?
    m_wallet->createFolder(folder);
    if (! m_wallet->setFolder(folder)) {
        kDebug() << "failed to open folder" << folder;
        return false;
    }
    return true;
}

void Twitter::setAuth()
{
    Plasma::DataEngine::Data conf = m_engine->property("config").value<Plasma::DataEngine::Data>();
    conf[m_username] = m_password;
    m_engine->setProperty("config", QVariant::fromValue(conf));
    m_statusEdit->setVisible( !( m_username.isEmpty() || m_password.isEmpty() ) );
    downloadHistory();
}

void Twitter::writeConfigPassword()
{
    kDebug();
    if (KMessageBox::warningYesNo(0, i18n("Failed to access kwallet. Store password in config file instead?"))
            == KMessageBox::Yes) {
        KConfigGroup cg = config();
        cg.writeEntry( "password", KStringHandler::obscure(m_password) );
    }
}

void Twitter::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    kDebug() << source;
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
        uint maxId = m_lastTweet;
        foreach (QString id, data.keys()) {
            uint i=id.toUInt();
            if (i > m_lastTweet) {
                newCount++;
                QVariant v = data.value(id);
                //Warning: This function is not available with MSVC 6
                Plasma::DataEngine::Data t = v.value<Plasma::DataEngine::Data>();
                m_tweetMap[id] = t;
                if (i > maxId) {
                    maxId = i;
                }
            }
        }
        m_lastTweet = maxId;
        m_flash->flash( i18n("%1 new tweets", qMin(newCount, m_historySize) ), 20*1000 );
        showTweets();
    } else if (source == "LatestImage") {
        QString user = data.begin().key();
        if (user.isEmpty()) {
            return;
        }
        QPixmap pm = data[user].value<QPixmap>();
        if( !pm.isNull() ) {
            if( user == m_username ) {
                m_icon->setIcon( QIcon( pm ) );
                QSizeF iconSize = m_icon->sizeFromIconSize(48);
                m_icon->setMinimumSize( iconSize );
                m_icon->setMaximumSize( iconSize );
            }
            m_pictureMap[user] = pm;
            //TODO it would be nice to check whether the updated image is actually in use
            showTweets();
        }
    } else if (source.startsWith("Error")) {
        QString desc = data["description"].toString();
        m_flash->flash(desc, 60 * 1000); //I'd really prefer it to stay there. and be red.
    }
    updateGeometry();
}

void Twitter::showTweets()
{
    // Adjust the number of the TweetWidgets if the configuration has changed
    // Add more tweetWidgets if there are not enough
    while( m_tweetWidgets.size() < m_historySize ) {
        QGraphicsLinearLayout *tweetLayout = new QGraphicsLinearLayout( Qt::Horizontal );
        tweetLayout->setMargin( 5 );
        tweetLayout->setSpacing( 5 );
        m_layout->addItem( tweetLayout );

        Plasma::LineEdit *e = new Plasma::LineEdit( this );
        e->setTextWidth( 300 );
        e->setStyled( false );
        e->setEnabled( false );
        e->setCursor( Qt::ArrowCursor );
        e->setAcceptedMouseButtons( Qt::NoButton );

        Plasma::Icon *icon = new Plasma::Icon( this );
        QSizeF iconSize = icon->sizeFromIconSize(30);
        icon->setMinimumSize( iconSize );
        icon->setMaximumSize( iconSize );
        tweetLayout->addItem( icon );
        tweetLayout->addItem( e );
        tweetLayout->updateGeometry();

        Tweet t;
        t.layout = tweetLayout;
        t.icon = icon;
        t.edit = e;

        m_tweetWidgets.append( t );
    }
    //clear out tweet widgets if there are too many
    while( m_tweetWidgets.size() > m_historySize ) {
        Tweet t = m_tweetWidgets[m_tweetWidgets.size()-1];
        m_layout->removeItem( t.layout );
        delete t.icon;
        delete t.edit;
        delete t.layout;
        m_tweetWidgets.removeAt( m_tweetWidgets.size()-1 );
    }

    int i = 0;
    int pos = m_tweetMap.keys().size() - 1;
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::self()->colors());
    while(i < m_historySize && pos >= 0 ) {
        Plasma::DataEngine::Data tweetData = m_tweetMap[m_tweetMap.keys()[pos]];
        QString user = tweetData.value( "User" ).toString();

        Tweet t = m_tweetWidgets[i];
        t.icon->setIcon( QIcon(m_pictureMap[user]) );
        QSizeF iconSize = t.icon->sizeFromIconSize(30);
        t.icon->setMinimumSize( iconSize );
        t.icon->setMaximumSize( iconSize );
        t.icon->updateGeometry();

        QString html = "<table cellspacing='0' spacing='5'>";
        html += i18n( "<tr><td align='left' width='1%'><font color='%2'>%1</font></td><td align='right' width='99%'><font color='%2'>%3 from %4</font></td></tr>", user, colorScheme.foreground(KColorScheme::InactiveText).color().name(),
                timeDescription( tweetData.value( "Date" ).toDateTime() ),
                tweetData.value( "Source" ).toString() );
        html += QString( "<tr><td colspan='2'><font color='%1'>%2</font></td></tr>" )
                .arg( colorScheme.foreground().color().name()).arg( tweetData.value( "Status" ).toString() );
        html += "</table>";
        t.edit->setHtml( html );
        ++i;
        --pos;
    }

    m_layout->invalidate();
    updateGeometry();
}

void Twitter::createConfigurationInterface()
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setPlainCaption(i18n("Configure Twitter Applet"));

        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );

        QWidget *configWidget = new QWidget(m_dialog);

        m_usernameEdit = new KLineEdit( configWidget );
        m_passwordEdit = new KLineEdit( configWidget );
        m_historySizeSpinBox = new QSpinBox( configWidget );
        m_historySizeSpinBox->setSuffix(i18n(" tweets"));
        m_historyRefreshSpinBox = new QSpinBox( configWidget );
        m_historyRefreshSpinBox->setSuffix(i18n(" minutes"));
        m_checkIncludeFriends = new QCheckBox( configWidget );

        QLabel *usernameLabel = new QLabel( i18n("User name:"), configWidget );
        QLabel *passwordLabel = new QLabel( i18n("Password:"), configWidget );
        QLabel *historyLabel = new QLabel( i18n("Timeline size:"), configWidget );
        QLabel *historyRefreshLabel = new QLabel( i18n("Timeline refresh:"), configWidget );
        QLabel *friendsLabel = new QLabel( i18n("Show messages of friends:"), configWidget );

        m_passwordEdit->setPasswordMode( true );

        QGridLayout *layout = new QGridLayout( configWidget );
        layout->addWidget( usernameLabel, 0, 0 );
        layout->addWidget( m_usernameEdit, 0, 1 );
        layout->addWidget( passwordLabel, 1, 0 );
        layout->addWidget( m_passwordEdit, 1, 1 );
        layout->addWidget( historyLabel, 2, 0 );
        layout->addWidget( m_historySizeSpinBox, 2, 1 );
        layout->addWidget( historyRefreshLabel, 3, 0 );
        layout->addWidget( m_historyRefreshSpinBox, 3, 1 );
        layout->addWidget( friendsLabel, 4, 0);
        layout->addWidget( m_checkIncludeFriends, 4, 1 );

        m_dialog->setMainWidget(configWidget);
    }

    m_usernameEdit->setText( m_username );
    m_passwordEdit->setText( m_password );
    m_historySizeSpinBox->setValue( m_historySize );
    m_historyRefreshSpinBox->setValue( m_historyRefresh );
    m_checkIncludeFriends->setCheckState( m_includeFriends ? Qt::Checked : Qt::Unchecked );

    m_dialog->show();
}

void Twitter::configAccepted()
{
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();
    int historyRefresh = m_historyRefreshSpinBox->value();
    int historySize = m_historySizeSpinBox->value();
    bool includeFriends = (m_checkIncludeFriends->checkState() == Qt::Checked);
    bool changed = false;

    KConfigGroup cg = config();

    if (m_username != username) {
        changed = true;
        m_username = username;
        m_icon->setIcon( QIcon() );
        m_icon->setText( m_username );
        cg.writeEntry( "username", m_username );
    }

    if (m_password != password) {
        changed = true;
        m_password = password;
    }
    //if (m_walletWait == Read)
    //then the user is a dumbass.
    //we're going to ignore that, which drops the read attempt
    //I hope that doesn't cause trouble.
    //XXX if there's a value in the config, the wallet will never be read
    //if a user saves their password in the config and later changes their mind
    //then they might not understand that they have to delete the password from plasma-appletsrc
    //FIXME if the username is blank, don't connect to the engine, set needsconfiguring, etc
    if (! m_username.isEmpty() && (changed || m_password.isEmpty())) {
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
        m_includeFriends = includeFriends;
    }

    if (m_historySize != historySize) {
        m_historySize = historySize;
        cg.writeEntry("historySize", m_historySize);
        if (! changed) {
            showTweets();
        }
    }

    if (changed) {
        //TODO we only *need* to wipe the map if name or includeFriends changed
        m_tweetMap.clear();
        m_lastTweet=0;
        setAuth();
    }
}

Twitter::~Twitter()
{
    delete m_dialog;
}

void Twitter::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);

    m_theme->resize();
    m_theme->paint( p, QRect(contentsRect.x()+contentsRect.width()-75, m_flash->geometry().y(), 75, 14), "twitter" );

    foreach( Tweet t, m_tweetWidgets ) {
        QRectF tweetRect(0, t.layout->geometry().y(), contentSize().width(), t.layout->geometry().height());
        m_theme->paint( p, tweetRect, "tweet" );
    }
    QRectF headerRect(0, m_headerLayout->geometry().y(), contentSize().width(), m_headerLayout->geometry().height()-5);
    m_theme->paint( p, headerRect, "tweet" );
}


void Twitter::updateStatus()
{
    QString status = m_username + ":" + m_statusEdit->toPlainText();
    m_engine->setProperty( "status", status );
    m_statusEdit->setPlainText("");
}

//what this really means now is 'reconnect to the timeline source'
void Twitter::downloadHistory()
{
    kDebug() ;
    if (m_username.isEmpty() || m_password.isEmpty()) {
        if (!m_curTimeline.isEmpty()) {
            m_engine->disconnectSource(m_curTimeline, this);
            m_engine->disconnectSource("Error:" + m_curTimeline, this);
        }
        return;
    }

    m_flash->flash( i18n("Refreshing timeline..."), -1 );

    QString query;
    if( m_includeFriends) {
        query = QString("TimelineWithFriends:%1");
    } else {
        query = QString("Timeline:%1");
    }
    query = query.arg(m_username);
    if (m_curTimeline != query) {
        //ditch the old one, if needed
        if (!m_curTimeline.isEmpty()) {
            m_engine->disconnectSource(m_curTimeline, this);
            m_engine->disconnectSource("Error:" + m_curTimeline, this);
        }
        m_curTimeline = query;
    }
    kDebug() << "Connecting to source " << query;
    m_engine->connectSource(query, this, m_historyRefresh * 60 * 1000);
    m_engine->connectSource("Error:" + query, this);
}

QString Twitter::timeDescription( const QDateTime &dt )
{
    int diff = dt.secsTo( QDateTime::currentDateTime() );
    QString desc;

    if( diff < 60 ) {
        desc = i18n( "1 minute ago" );
    }else if( diff < 60*60 ) {
        desc = i18n( "%1 minutes ago", QString::number( diff/60 ) );
    } else if( diff < 2*60*60 ) {
        desc = i18n( "1 hour ago");
    } else if( diff < 24*60*60 ) {
        desc = i18n( "%1 hours ago", QString::number( diff/3600 ) );
    } else {
        desc = dt.toString( Qt::LocaleDate );
    }
    return desc;
}

#include "twitter.moc"
