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

#ifndef TWITTER_H
#define TWITTER_H

#include <QLabel>
#include <QDomDocument>
#include <QMap>

#include <plasma/applet.h>
#include <plasma/dataengine.h>


class KDialog;
class KLineEdit;
class QSpinBox;
class KJob;
class QCheckBox;

namespace KWallet
{
    class Wallet;
}

namespace Plasma
{
    class Svg;
    class LineEdit;
    class Flash;
    class Icon;
    class VBoxLayout;
    class HBoxLayout;
}

struct Tweet {
    Plasma::HBoxLayout *layout;
    Plasma::Icon *icon;
    Plasma::LineEdit *edit;
};

class Twitter : public Plasma::Applet
{
    Q_OBJECT
    public:
        Twitter(QObject *parent, const QVariantList &args);
        ~Twitter();
        void init();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            const QRect &contentsRect);

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void showConfigurationInterface();
        void gotWallet(bool success);

    protected slots:
        void newSource( const QString & );

        void configAccepted();
        void updateStatus();
        void downloadHistory();

        void showTweets();

    protected:
        QString timeDescription( const QDateTime &dt );

    Q_SIGNALS:
        void pictureDownloaded( const QString &nick );

    private:
        void getWallet();
        bool enterWalletFolder(const QString &folder);
        void gotPassword();
        void writeConfigPassword();

        Plasma::Svg *m_theme;
        KDialog *m_dialog;
        Plasma::LineEdit *m_title;
        Plasma::LineEdit *m_statusEdit;
        Plasma::LineEdit *m_historyEdit;
        Plasma::Flash *m_flash;
        Plasma::Icon *m_icon;
        Plasma::VBoxLayout *m_layout;
        Plasma::HBoxLayout *m_headerLayout;

        KLineEdit *m_usernameEdit;
        KLineEdit *m_passwordEdit;
        QSpinBox *m_historySizeSpinBox;
        QSpinBox *m_historyRefreshSpinBox;
        QCheckBox *m_checkIncludeFriends;

        QString m_username;
        QString m_password;
        int m_historySize;
        int m_historyRefresh;
        bool m_includeFriends;

        Plasma::DataEngine *m_engine;
        QString m_curTimeline;

        QMap< QString, QPixmap > m_pictureMap;
        QMap< KJob *, QString > m_pictureDownloadMap;
        QMap< KJob *, QByteArray > m_bufferMap;
        QMap< QString, Plasma::DataEngine::Data > m_tweetMap;
        QList< Tweet > m_tweetWidgets;

        QTimer *m_refreshTimer;

        uint m_lastTweet;
        bool m_waitingForData;
        KWallet::Wallet *m_wallet;
        enum WalletWait { None=0, Read, Write };
        WalletWait m_walletWait;
};

K_EXPORT_PLASMA_APPLET(twitter, Twitter)

#endif
