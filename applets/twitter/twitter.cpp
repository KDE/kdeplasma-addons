/***************************************************************************
 *   Copyright (C) 2007 by André Duffeck                                   *
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

Twitter::Twitter(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0), m_lastTweet( 0 )
{
    kDebug() << "Loading applet twitter";
    setHasConfigurationInterface(true);
    setDrawStandardBackground(true);

    KConfigGroup cg = config();
    m_username = cg.readEntry( "username" );
    m_password = KStringHandler::obscure( cg.readEntry( "password" ) );
    m_historySize = cg.readEntry( "historySize", 2 );
    m_historyRefresh = cg.readEntry( "historyRefresh", 5 );
    m_includeFriends = cg.readEntry( "includeFriends", true );


    m_statusEdit = new Plasma::LineEdit( this );
    if ( m_username.isEmpty() || m_password.isEmpty() ) {
        m_statusEdit->hide();
    }
    connect( m_statusEdit->document(), SIGNAL(contentsChanged()), SLOT(geometryChanged()) );
    connect( m_statusEdit, SIGNAL(editingFinished()), SLOT(updateStatus()) );

    m_historyEdit = new Plasma::LineEdit( this );
    m_historyEdit->setStyled( false );
    m_historyEdit->setEnabled( false );
    m_historyEdit->setCursor( Qt::ArrowCursor );
    m_historyEdit->setAcceptedMouseButtons( Qt::NoButton );

    m_flash = new Plasma::Flash( this );
    m_flash->setColor( Qt::gray );
    QFont fnt = qApp->font();
    fnt.setBold( true );
    m_flash->setFont( fnt );

    m_theme = new Plasma::Svg("widgets/twitter", this);

    m_engine = dataEngine("twitter");
    m_engine->setProperty( "username", m_username );
    m_engine->setProperty( "password", m_password );
    connect( m_engine, SIGNAL(newSource(const QString&)), SLOT(newSource(const QString&)) );

    m_size = QSizeF( 250, 250 );

    geometryChanged();
    downloadHistory();
}

void Twitter::updated(const QString& source, const Plasma::DataEngine::Data &data)
{
    kDebug() << source;
    if( source == QString( "Timeline:%1" ).arg( m_username ) ||
        source == QString( "TimelineWithFriends:%1" ).arg( m_username ) ) {
        m_flash->flash( i18n("Refreshing timeline...") );
        QVariantList l = data.value( source ).toList();

        int newCount = 0;
        foreach( QVariant id, l ) {
            if( id.toUInt() > m_lastTweet ) {
                newCount++;
            }
        }

        int i = 0;
        QString html = "<table cellspacing='0'>";
        while(i < m_historySize && i < l.size() ) {
            uint id = l[i].toUInt();
            QString tweet = QString( "Update:%1" ).arg( l[i].toString() );
            if( id > m_lastTweet )
                m_lastTweet = id;
            Plasma::DataEngine::Data tweetData = m_engine->query( tweet );

            html += i18n( "<tr><td width='1%'><font color='#fcfcfc'><b>%1</b></font></td>"
                    "<td align='right' width='99%'><font color='#fcfcfc'>%2 from %3</font></td></tr>", 
                    tweetData.value( "User" ).toString(), timeDescription( tweetData.value( "Date" ).toDateTime() ),
                    tweetData.value( "Source" ).toString() );
            html += QString( "<tr><td colspan='2'><font color='#fcfcfc'>%1<br></font></td></tr>" )
                    .arg( tweetData.value( "Status" ).toString() );
            ++i;
            }
        html += "</table>";
        m_historyEdit->setHtml( html );
        geometryChanged();
        m_flash->flash( i18n("%1 new tweets", newCount ), 20*1000 );
    } else if( source.startsWith( "UserInfo" ) ) {
        QPixmap pm = data.value( "Image" ).value<QPixmap>();
        if( !pm.isNull() ) {
            m_picture = pm;
            geometryChanged();
        }
    }
    update();
}

void Twitter::newSource( const QString &source )
{
    if( !source.startsWith( "Timeline" ) )
        m_engine->connectSource( source, this );
}


QSizeF Twitter::contentSizeHint() const
{
    return m_size;
}

void Twitter::geometryChanged()
{
    prepareGeometryChange();

    const int cx = 0;
    const int cy = 0;
    const int width = (int)contentSize().width();

    int y = cy+21;
    int w = 300;
    int h = 250;

    if( m_picture.isNull() ) {
        m_statusEdit->setTextWidth( width-6 );
        m_statusEdit->setPos( cx+3, y );
        m_statusEdit->setGeometry( QRectF( cx, y, width-6, cy+72-21 ) );
    } else {
        m_statusEdit->setTextWidth( width-61 );
        m_statusEdit->setPos( cx+58, y );
        m_statusEdit->setGeometry( QRectF( cx+55, y, width-61, cy+72-21 ) );
    }
    y = cy+78;

    m_historyEdit->setTextWidth( width-6 );
    m_historyEdit->setPos( cx+3, y );

    if( h < y+m_historyEdit->geometry().height()+6 )
        h = (int)(y+m_historyEdit->geometry().height()+6);

    m_size = QSizeF( w, h  );

    m_flash->setSize( QSize( width-75, 20 ) );
    m_flash->setPos( cx, cy);

    update();
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
        m_picture = QPixmap();
        geometryChanged();
    }

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

    m_engine->setProperty( "username", m_username );
    m_engine->setProperty( "password", m_password );

    downloadHistory();
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
    p->drawRect( contentsRect.x(), contentsRect.y() + 18, contentsRect.width(), 54);
    p->drawRect( contentsRect.x(), contentsRect.y() + 75, contentsRect.width(), contentsRect.height()-75);

    if( !m_picture.isNull() )
        p->drawPixmap( contentsRect.x()+3, contentsRect.y()+21, m_picture );
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

    if( !m_curTimeline.isEmpty() )
        m_engine->disconnectSource( m_curTimeline, this );

    QString query;
    if( m_includeFriends) {
        query = QString( "TimelineWithFriends:%1" ).arg( m_username );
    } else {
        query = QString( "Timeline:%1" ).arg( m_username );
    }
    m_curTimeline = query;
    m_engine->connectSource( query, this );
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
