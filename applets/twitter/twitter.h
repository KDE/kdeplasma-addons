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

#ifndef TWITTER_H
#define TWITTER_H

#include <QLabel>
#include <QDomDocument>
#include <QHttpResponseHeader>
#include <QMap>

#include <plasma/applet.h>
#include <plasma/dataengine.h>


class KDialog;
class KLineEdit;
class QSpinBox;
class KJob;
class QCheckBox;

namespace Plasma
{
    class Svg;
    class LineEdit;
    class Flash;
}
namespace KIO
{
    class Job;
}

class Twitter : public Plasma::Applet
{
    Q_OBJECT
    public:
        enum Action { Idle, Upload, HistoryDownload };

        Twitter(QObject *parent, const QVariantList &args);
        ~Twitter();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            const QRect &contentsRect);
        QSizeF contentSizeHint() const;

    public slots:
        void updated(const QString &name, const Plasma::DataEngine::Data &data);
        void showConfigurationInterface();

    protected slots:
        void newSource( const QString & );

        void configAccepted();
        void updateStatus();
        void downloadHistory();

        void geometryChanged();

        void slotConnected();
        void slotRead();

    protected:
        QString timeDescription( const QDateTime &dt );

    Q_SIGNALS:
        void pictureDownloaded( const QString &nick );

    private:
        QSizeF m_size;
        Plasma::Svg *m_theme;
        KDialog *m_dialog;
        Plasma::LineEdit *m_title;
        Plasma::LineEdit *m_statusEdit;
        Plasma::LineEdit *m_historyEdit;
        Plasma::Flash *m_flash;

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

        Action m_action;

        Plasma::DataEngine *m_engine;
        QTcpSocket *m_socket;
        QHttpResponseHeader m_header;
        QDomDocument m_historyDoc;
        QString m_data;
        QPixmap m_picture;
        QString m_curTimeline;

        QMap< QString, QPixmap > m_pictureMap;
        QMap< KJob *, QString > m_pictureDownloadMap;
        QMap< KJob *, QByteArray > m_bufferMap;

        uint m_lastTweet;
};

K_EXPORT_PLASMA_APPLET(twitter, Twitter)

#endif
