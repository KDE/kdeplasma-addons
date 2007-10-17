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
#include <QTcpSocket>
#include <QCheckBox>

#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KDialog>
#include <ksocketfactory.h>
#include <KLineEdit>
#include <KStringHandler>
#include <KCodecs>
#include <KUrl>
#include <KIO/Job>
#include <KIO/TransferJob>

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/flash.h>
#include <plasma/widgets/boxlayout.h>
#include <plasma/widgets/icon.h>

Twitter::Twitter(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0), m_lastTweet( 0 )
{
    kDebug() << "Loading applet twitter";
    setHasConfigurationInterface(true);
    setDrawStandardBackground(true);
    m_theme = new Plasma::Svg("widgets/twitter", this);


    KConfigGroup cg = config();
    m_username = cg.readEntry( "username" );
    m_password = KStringHandler::obscure( cg.readEntry( "password" ) );
    m_historySize = cg.readEntry( "historySize", 2 );
    m_historyRefresh = cg.readEntry( "historyRefresh", 5 );
    m_includeFriends = cg.readEntry( "includeFriends", true );

    m_engine = dataEngine("twitter");
    m_engine->setProperty( "username", m_username );
    m_engine->setProperty( "password", m_password );
    m_engine->setProperty( "interval", m_historyRefresh*60*1000 );
    connect( m_engine, SIGNAL(newSource(const QString&)), SLOT(newSource(const QString&)) );


    m_layout = new Plasma::VBoxLayout( this );
    m_layout->setMargin( 0 );
    m_layout->setSpacing( 0 );

    m_flash = new Plasma::Flash( this );
    m_flash->setColor( Qt::gray );
    m_flash->setSize( QSize(145, 20) );
    QFont fnt = qApp->font();
    fnt.setBold( true );
    m_flash->setFont( fnt );
    m_layout->addItem( m_flash );


    m_headerLayout = new Plasma::HBoxLayout( m_layout );
    m_headerLayout->setMargin( 0 );
    m_layout->addItem( m_headerLayout );


    m_icon = new Plasma::Icon( this );
    m_icon->setIcon( KIcon( "user" ) );
    m_icon->setIconSize( 48, 48 );
    m_icon->setText( m_username );
    m_headerLayout->addItem( m_icon );


    m_statusEdit = new Plasma::LineEdit( this );
    if ( m_username.isEmpty() || m_password.isEmpty() ) {
        m_statusEdit->hide();
    }
    m_statusEdit->setStyled( true );
    m_statusEdit->setTextWidth( 200 );
    connect( m_statusEdit->document(), SIGNAL(contentsChanged()), SLOT(geometryChanged()) );
    connect( m_statusEdit, SIGNAL(editingFinished()), SLOT(updateStatus()) );
    m_headerLayout->addItem( m_statusEdit );


    downloadHistory();
}

void Twitter::updated(const QString& source, const Plasma::DataEngine::Data &data)
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
    } else if( source.startsWith( "UserInfo" ) ) {
        QPixmap pm = data.value( "Image" ).value<QPixmap>();
        if( !pm.isNull() ) {
            m_icon->setIcon( QIcon( pm ) );
        }
    }
    updateGeometry();
}

void Twitter::showTweets()
{
    foreach( Plasma::LineEdit *e, m_tweetWidgets ) {
        m_layout->removeItem( e );
        delete e;
    }
    m_tweetWidgets.clear();

    int i = 0;
    int pos = m_tweetMap.keys().size() - 1;
    while(i < m_historySize && i < m_tweetMap.keys().size() ) {
        Plasma::LineEdit *e = new Plasma::LineEdit( this );
        e->setTextWidth( 250 );
        e->setStyled( false );
        e->setEnabled( false );
        e->setCursor( Qt::ArrowCursor );
        e->setAcceptedMouseButtons( Qt::NoButton );
        m_layout->addItem( e );
        m_tweetWidgets.append( e );
    kDebug();
        Plasma::DataEngine::Data tweetData = m_tweetMap[m_tweetMap.keys()[pos]];
    kDebug();
        QString html = "<table cellspacing='0'>";
        html += i18n( "<tr><td width='1%'><font color='#fcfcfc'><b>%1</b></font></td>"
                "<td align='right' width='99%'><font color='#fcfcfc'>%2 from %3</font></td></tr>", 
                tweetData.value( "User" ).toString(), timeDescription( tweetData.value( "Date" ).toDateTime() ),
                tweetData.value( "Source" ).toString() );
        html += QString( "<tr><td colspan='2'><font color='#fcfcfc'>%1<br></font></td></tr>" )
                .arg( tweetData.value( "Status" ).toString() );
        html += "</table>";
        e->setHtml( html );
        ++i;
        --pos;
    }
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

    KConfigGroup cg = config();
    m_username = m_usernameEdit->text();
    cg.writeEntry( "username", m_username );
    m_password = m_passwordEdit->text();
    cg.writeEntry( "password", KStringHandler::obscure(m_password) );
    m_historySize = m_historySizeSpinBox->value();
    cg.writeEntry("historySize", m_historySize);
    m_historyRefresh = m_historyRefreshSpinBox->value();
    cg.writeEntry("historyRefresh", m_historyRefresh);
    m_includeFriends = (m_checkIncludeFriends->checkState() == Qt::Checked);
    cg.config()->sync();

    m_statusEdit->setVisible( !( m_username.isEmpty() || m_password.isEmpty() ) );
    if( !m_username.isEmpty() && !m_password.isEmpty() )
      m_statusEdit->show();
    else
      m_statusEdit->hide();

    m_engine->setProperty( "username", m_username );
    m_engine->setProperty( "password", m_password );
    m_engine->setProperty( "interval", m_historyRefresh*60*1000 );

    if( refresh ) {
        showTweets();
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

    p->setBrush( QColor( 32, 32, 32 ) );
    foreach( Plasma::LineEdit *e, m_tweetWidgets ) {
        p->drawRect( 0, e->geometry().y(), contentSize().width(), e->geometry().height() );
    }
    p->drawRect( m_headerLayout->geometry() );
}


void Twitter::updateStatus()
{
    kDebug() ;
    m_action = Upload;

    m_socket = KSocketFactory::connectToHost( "http", "twitter.com", 80 );
    connect( m_socket, SIGNAL(connected()), SLOT(slotConnected()) );
    connect( m_socket, SIGNAL(readyRead()), SLOT(slotRead()) );
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

void Twitter::slotConnected()
{
    kDebug() ;
    QString auth = QString( "%1:%2" ).arg( m_username, m_password );
    auth = QString( "Basic " ) + KCodecs::base64Encode( auth.toAscii() );
    QString data;
    if( m_action == Upload ) {
        QString status = QString( "source=kdetwitter&status=%1" ).arg( m_statusEdit->toPlainText() );
        data = QString("POST /statuses/update.json HTTP/1.1\r\n"
                "Authorization: %1\r\n"
                "User-Agent: Mozilla/5.0\r\n"
                "Host: twitter.com\r\n"
                "Accept: */*\r\n"
                "Content-Length: %2\r\n"
                "Content-Type: application/x-www-form-urlencoded\r\n\r\n"
                "%3" )
                .arg( auth, QString::number(status.toUtf8().length()), status );
    }

    m_header = QHttpResponseHeader();
    m_data.truncate( 0 );
    m_socket->write( data.toUtf8(), data.toUtf8().length() );
}

void Twitter::slotRead()
{
    kDebug() ;
    QString read = m_socket->readAll();

    QString data;
    if( !m_header.isValid() ) {
        m_header = read.section( "\r\n\r\n", 0, 0 );
        m_data = read.section( "\r\n\r\n", 1, 1 );
    } else {
        m_data.append( read );
    }

    if( m_header.statusCode() == 401 ) {
        m_flash->flash( i18n("Authentication failed") );
        m_action = Idle;
        return;
    }

    if( m_action == Upload ) {
        kDebug() << "Status upload succeeded.";
        m_flash->flash( i18n("Status upload succeeded") );
        m_action = Idle;
        m_statusEdit->setPlainText("");
    }
}


#include "twitter.moc"
