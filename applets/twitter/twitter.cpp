/***************************************************************************
 *   Copyright (C) 2007 by André Duffeck <duffeck@kde.org>                 *
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
#include <QTimer>
#include <QGradient>
#include <QFontMetrics>
#include <QGraphicsView>

#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KDialog>
#include <KLineEdit>
#include <KStringHandler>
#include <KWallet/Wallet>
#include <KMessageBox>

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/flash.h>
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>

Twitter::Twitter(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0), m_lastTweet(0), m_wallet(0), m_walletWait(None)
{
    setHasConfigurationInterface(true);
    setContentSize(300,150); //should be an ok default
}

void Twitter::init()
{
    m_theme = new Plasma::Svg("widgets/twitter", this);

    KConfigGroup cg = config();
    m_username = cg.readEntry( "username" );
    m_password = KStringHandler::obscure( cg.readEntry( "password" ) );
    m_historySize = cg.readEntry( "historySize", 2 );
    m_historyRefresh = cg.readEntry( "historyRefresh", 5 );
    m_includeFriends = cg.readEntry( "includeFriends", true );

    m_engine = dataEngine("twitter");
    m_engine->setProperty( "username", m_username );
    m_engine->setProperty( "interval", m_historyRefresh*60*1000 );
    connect( m_engine, SIGNAL(newSource(const QString&)), SLOT(newSource(const QString&)) );


    m_layout = new Plasma::VBoxLayout( this );
    m_layout->setMargin( 0 );
    m_layout->setSpacing( 0 );

    m_flash = new Plasma::Flash( this );
    m_flash->setColor( Qt::gray );
    QFont fnt = qApp->font();
    fnt.setBold( true );
    QFontMetrics fm( fnt );
    m_flash->resize( QSize(200, fm.height()) );
    m_flash->setMaximumSize( QSizeF(200, fm.height()+10 ) );
    m_flash->setFont( fnt );
    m_layout->addItem( m_flash );


    m_headerLayout = new Plasma::HBoxLayout( m_layout );
    m_headerLayout->setMargin( 5 );
    m_headerLayout->setSpacing( 5 );
    m_layout->addItem( m_headerLayout );


    m_icon = new Plasma::Icon( this );
    m_icon->setIcon( KIcon( "user-identity" ) );
    m_icon->setText( m_username );
    QSizeF iconSize = m_icon->sizeFromIconSize(32);
    m_icon->setMinimumSize( iconSize );
    m_icon->setMaximumSize( iconSize );
    m_headerLayout->addItem( m_icon );

    m_statusEdit = new Plasma::LineEdit( this );
    m_statusEdit->hide();
    m_statusEdit->setStyled( true );
    m_statusEdit->setTextWidth( 250 );
    connect( m_statusEdit->document(), SIGNAL(contentsChanged()), SLOT(geometryChanged()) );
    connect( m_statusEdit, SIGNAL(editingFinished()), SLOT(updateStatus()) );
    m_headerLayout->addItem( m_statusEdit );

    m_refreshTimer = new QTimer( this );
    connect( m_refreshTimer, SIGNAL(timeout()), SLOT(showTweets()) );

    if(! m_username.isEmpty()) {
        if (m_password.isEmpty()) {
            m_walletWait = Read;
            getWallet();
        } else { //use config value
            gotPassword();
        }
    }
}

void Twitter::getWallet()
{
    kDebug();
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
    connect( m_wallet, SIGNAL( walletOpened(bool) ), SLOT( gotWallet(bool) ) );
}

void Twitter::gotWallet(bool success)
{
    if (success && enterWalletFolder(QString::fromLatin1("Plasma-Twitter"))){
        if (m_walletWait == Read) {
            QString pwd;
            if (m_wallet->readPassword(m_username, pwd) == 0) {
                m_password = pwd;
                gotPassword();
            }
        } else if (m_walletWait == Write) {
            if (m_wallet->writePassword(m_username, m_password) != 0) {
                kDebug() << "failed to write password";
                writeConfigPassword();
            }
        }
    } else {
        kDebug() << "failed to get wallet";
        if (m_walletWait == Write) {
            writeConfigPassword();
        }
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

void Twitter::gotPassword()
{
    m_engine->setProperty( "password", m_password );
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
    if( source == m_curTimeline ) {
        m_flash->flash( i18n("Refreshing timeline...") );
        QVariantList l = data.value( source ).toList();

        int newCount = 0;
        foreach( QVariant id, l ) {
            if( id.toUInt() > m_lastTweet ) {
                newCount++;
            }
        }
        foreach( QVariant id, l ) {
            if( id.toUInt() > m_lastTweet )
                m_lastTweet = id.toUInt();
        }
        m_flash->flash( i18n("%1 new tweets", newCount ), 20*1000 );
        showTweets();
    } else if( source.startsWith( "Update" ) ) {
        m_tweetMap[source] = data;
        if( m_waitingForData ) {
            m_waitingForData = false;
            showTweets();
        }
    } else if( source.startsWith( "UserInfo" ) ) {
        QString user = source.split( ':' ).at(1);
        QPixmap pm = data.value( "Image" ).value<QPixmap>();
        if( !pm.isNull() ) {
            if( user == m_username ) {
                m_icon->setIcon( QIcon( pm ) );
                QSizeF iconSize = m_icon->sizeFromIconSize(32);
                m_icon->setMinimumSize( iconSize );
                m_icon->setMaximumSize( iconSize );
            }
            m_pictureMap[user] = pm;
        }
        if( m_refreshTimer->isActive() )
            m_refreshTimer->stop();
        m_refreshTimer->start( 500 );
    }
    updateGeometry();
}

void Twitter::showTweets()
{
    if( m_refreshTimer->isActive() )
        m_refreshTimer->stop();

    // Verify that all tweets have arrived already
    int i = 0;
    int pos = m_tweetMap.keys().size() - 1;
    while(i < m_historySize && i < m_tweetMap.keys().size() ) {
        Plasma::DataEngine::Data tweetData = m_tweetMap[m_tweetMap.keys()[pos]];
        if( tweetData.value( "User" ).toString().isEmpty() ) {
            m_waitingForData = true;
            return;
        }
        ++i;
        --pos;
    }

    for( int i = m_tweetWidgets.size()-1; i >= 0; --i ) {
        Tweet t = m_tweetWidgets[i];
        m_layout->removeItem( t.layout );
        delete t.icon;
        delete t.edit;
        delete t.layout;
    }
    m_tweetWidgets.clear();

    i = 0;
    pos = m_tweetMap.keys().size() - 1;
    while(i < m_historySize && i < m_tweetMap.keys().size() ) {
        Plasma::DataEngine::Data tweetData = m_tweetMap[m_tweetMap.keys()[pos]];
        QString user = tweetData.value( "User" ).toString();

        Plasma::HBoxLayout *tweetLayout = new Plasma::HBoxLayout( 0 );
        tweetLayout->setMargin( 5 );
        tweetLayout->setSpacing( 5 );
        m_layout->addItem( tweetLayout );

        Plasma::LineEdit *e = new Plasma::LineEdit( this );
        e->setTextWidth( 250 );
        e->setStyled( false );
        e->setEnabled( false );
        e->setCursor( Qt::ArrowCursor );
        e->setAcceptedMouseButtons( Qt::NoButton );

        Plasma::Icon *icon = new Plasma::Icon( this );
        icon->setIcon( QIcon(m_pictureMap[user]) );
        icon->setText( user );
        QSizeF iconSize = icon->sizeFromIconSize(32);
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

        QString html = "<table cellspacing='0'>";
        html += i18n( "<tr><td align='right' width='99%'><font color='#9c9c9c'>%1 from %2</font></td></tr>", 
                timeDescription( tweetData.value( "Date" ).toDateTime() ),
                tweetData.value( "Source" ).toString() );
        html += QString( "<tr><td><font color='#fcfcfc'>%1<br></font></td></tr>" )
                .arg( tweetData.value( "Status" ).toString() );
        html += "</table>";
        e->setHtml( html );
        ++i;
        --pos;
    }

    m_layout->invalidate();
    updateGeometry();
}


void Twitter::newSource( const QString &source )
{
    m_engine->connectSource( source, this );
}

void Twitter::showConfigurationInterface()
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

        QLabel *usernameLabel = new QLabel( i18n("Username"), configWidget );
        QLabel *passwordLabel = new QLabel( i18n("Password"), configWidget );
        QLabel *historyLabel = new QLabel( i18n("Timeline size"), configWidget );
        QLabel *historyRefreshLabel = new QLabel( i18n("Timeline refresh"), configWidget );
        QLabel *friendsLabel = new QLabel( i18n("Show messages of friends"), configWidget );

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
    if( m_username != m_usernameEdit->text() ) {
        m_icon->setIcon( QIcon() );
    }
    bool refresh = ( m_historySize != m_historySizeSpinBox->value() );
    bool changed = false;

    KConfigGroup cg = config();

    if( m_username != m_usernameEdit->text() )
        changed = true;
    m_username = m_usernameEdit->text();
    cg.writeEntry( "username", m_username );

    if( m_password != m_passwordEdit->text() )
        changed = true;
    m_password = m_passwordEdit->text();
    //if (m_walletWait == Read)
    //then the user is a dumbass.
    //we're going to ignore that, which drops the read attempt
    //I hope that doesn't cause trouble.
    //XXX if there's a value in the config, the wallet will never be read
    //if a user saves their password in the config and later changes their mind
    //then they might not understand that they have to delete the password from plasma-appletsrc
    //TODO if the password is blank, it'd be convenient to try and *read* from the wallet instead
    if (changed) {
        //a change in name *or* pass means we need to update the wallet
        m_walletWait = Write;
        getWallet();
    }

    m_historySize = m_historySizeSpinBox->value();
    cg.writeEntry("historySize", m_historySize);
    m_historyRefresh = m_historyRefreshSpinBox->value();
    cg.writeEntry("historyRefresh", m_historyRefresh);

    if( m_includeFriends != (m_checkIncludeFriends->checkState() == Qt::Checked) )
        changed = true;

    m_includeFriends = (m_checkIncludeFriends->checkState() == Qt::Checked);

    m_statusEdit->setVisible( !( m_username.isEmpty() || m_password.isEmpty() ) );

    m_engine->setProperty( "username", m_username );
    m_engine->setProperty( "password", m_password );
    m_engine->setProperty( "interval", m_historyRefresh*60*1000 );
    m_icon->setText( m_username );

    if( refresh ) {
        showTweets();
    }
    if( changed ) {
        m_tweetMap.clear();
        downloadHistory();
    }
}

Twitter::~Twitter()
{
    delete m_dialog;
}

void Twitter::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);

    p->setRenderHint(QPainter::SmoothPixmapTransform);

    m_theme->resize();
    m_theme->paint( p, QRect(contentsRect.x()+contentsRect.width()-75, contentsRect.y(), 75, 12), "twitter" );

    foreach( Tweet t, m_tweetWidgets ) {
        QLinearGradient g( t.layout->geometry().topLeft(), t.layout->geometry().bottomRight() );
        g.setColorAt( 0, QColor( 30, 30, 30 ) );
        g.setColorAt( 1, QColor( 70, 70, 70 ) );
        p->setBrush( QBrush( g ));
        p->drawRect( 0, t.layout->geometry().y(), contentSize().width(), t.layout->geometry().height() );
    }
    QLinearGradient g( m_headerLayout->geometry().topLeft(), m_headerLayout->geometry().bottomRight() );
    g.setColorAt( 0, QColor( 30, 30, 30 ) );
    g.setColorAt( 1, QColor( 70, 70, 70 ) );
    p->setBrush( QBrush( g ));
    p->drawRect( m_headerLayout->geometry() );
}


void Twitter::updateStatus()
{
    m_engine->setProperty( "status", m_statusEdit->toPlainText() );
    m_statusEdit->setPlainText("");
}

void Twitter::downloadHistory()
{
    kDebug() ;
    m_flash->flash( i18n("Refreshing timeline..."), -1 );

    if ( m_username.isEmpty() || m_password.isEmpty() )
        return;

//     if( !m_curTimeline.isEmpty() )
//         m_engine->disconnectSource( m_curTimeline, this );

    QString query;
    if( m_includeFriends) {
        query = QString( "TimelineWithFriends:%1" ).arg( m_username );
    } else {
        query = QString( "Timeline:%1" ).arg( m_username );
    }
    m_curTimeline = query;
    kDebug() << "Connecting to source " << query;
    m_engine->query( query );
}

QString Twitter::timeDescription( const QDateTime &dt )
{
    int diff = dt.secsTo( QDateTime::currentDateTime() );
    QString desc;

    if( diff < 5 ) {
        desc = i18n( "less than 5 seconds ago" );
    } else if( diff < 60 ) {
        desc = i18n( "less than %1 seconds ago", QString::number( diff ) );
    } else if( diff < 60*60 ) {
        desc = i18n( "%1 minutes ago", QString::number( diff/60 ) );
    } else if( diff < 24*60*60 ) {
        desc = i18n( "about %1 hours ago", QString::number( diff/3600 ) );
    } else {
        desc = dt.toString( Qt::LocaleDate );
    }
    return desc;
}

#include "twitter.moc"
