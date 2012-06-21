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

#ifndef MICROBLOG_H
#define MICROBLOG_H

#include <QMap>

#include <KTimeZone>

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

#include "ui_configuration.h"

class QGraphicsLinearLayout;
class QGraphicsAnchorLayout;
class KColorScheme;
class PostWidget;

namespace KWallet
{
    class Wallet;
}

namespace Plasma
{
    class Svg;
    class FlashingLabel;
    class IconWidget;
    class TextEdit;
    class WebContent;
    class Frame;
    class ServiceJob;
    class TabBar;
    class ScrollWidget;
}

class MicroBlog : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        MicroBlog(QObject *parent, const QVariantList &args);
        ~MicroBlog();
        void init();
        void constraintsEvent(Plasma::Constraints constraints);
        QGraphicsWidget *graphicsWidget();


    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void createConfigurationInterface( KConfigDialog *parent );
        void configChanged();

        /**
         * read from the opened KWallet
         * @param success whether we got to open it
         */
        void readWallet(bool success);
        /**
         * write to the opened KWallet
         * @param success whether we got to open it
         */
        void writeWallet(bool success);

    protected Q_SLOTS:
        void configAccepted();
        void updateStatus();
        void updateCompleted(Plasma::ServiceJob *job);
        void retweetCompleted(Plasma::ServiceJob *job);
        void favoriteCompleted(Plasma::ServiceJob *job);
        void downloadHistory();
        void editTextChanged();
        void serviceFinished(Plasma::ServiceJob *job);

        void themeChanged();
        void scheduleShowTweets();
        void showTweets();
        void modeChanged(int index);
        void openProfile(const QString &user = QString());
        void reply(const QString &replyToId, const QString &to);
        void forward(const QString &rt);
        void favorite(const QString &rt, const bool isFavourite);

        /**
         * attempt to get the wallet from KWallet
         * */
        void getWallet();

    protected:
        bool eventFilter(QObject *obj, QEvent *event);
        void popupEvent(bool show);
        void focusInEvent(QFocusEvent *event);

    private:
        /**
         * convenience function for setting the folder
         */
        bool enterWalletFolder(const QString &folder);
        /**
         * disable the UI until user is authenticated
         */
        void setAuthRequired(bool auth);
        /**
         * write the password to config instead of wallet
         */
        void writeConfigPassword();

        /**
         * generates and sets the popup icon
         */
        void paintIcon();

        /**
         * Sets up the timeline service object (m_service)
         */
        void createTimelineService();

        Plasma::Svg *m_theme;
        Plasma::TextEdit *m_statusEdit;
        Plasma::WebContent *m_historyEdit;
        Plasma::FlashingLabel *m_flash;
        Plasma::IconWidget *m_icon;
        Plasma::ScrollWidget *m_scrollWidget;
        Plasma::TabBar *m_tabBar;
        QGraphicsWidget *m_tweetsWidget;
        QGraphicsLinearLayout *m_tweetsLayout;
        QGraphicsLinearLayout *m_layout;
        QGraphicsAnchorLayout *m_headerLayout;
        QGraphicsWidget *m_graphicsWidget;
        QPixmap m_popupIcon;

        QString m_username;
        QString m_password;
        QString m_serviceUrl;
        QString m_imageQuery;
        //QHash<Plasma::ServiceJob *, QString> m_statusUpdates;
        int m_historySize;
        int m_historyRefresh;
        int m_newTweets;
        bool m_includeFriends;
        int m_lastMode;

        Plasma::DataEngine *m_engine;
        QWeakPointer<Plasma::Service> m_service;
        Plasma::Service *m_profileService;
        QSet<Plasma::ServiceJob *> m_updateJobs;
        QSet<Plasma::ServiceJob *> m_retweetJobs;
        QSet<Plasma::ServiceJob *> m_favoriteJobs;
        QString m_curTimeline;
        QString m_replyToId;
        Plasma::Frame *m_headerFrame;

        QMap<QString, QPixmap> m_pictureMap;
        QStringList m_avatarHistory;
        QMap<qulonglong, Plasma::DataEngine::Data> m_tweetMap;
        QList<PostWidget *> m_tweetWidgets;

        qulonglong m_lastTweet;
        KWallet::Wallet *m_wallet;
        enum WalletWait { None=0, Read, Write };
        WalletWait m_walletWait;

        KColorScheme *m_colorScheme;
        Ui::TwitterConfig configUi;

        QTimer *m_showTweetsTimer;
        QTimer *m_getWalletDelayTimer;
};

K_EXPORT_PLASMA_APPLET(microblog, MicroBlog)

#endif
