/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *   Copyright (C) 2007 by Jeff Cooper <weirdsox11@gmail.com>              *
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

#ifndef DICT_H
#define DICT_H

#include <QGraphicsItem>

#include <KLineEdit>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/flash.h>
#include "ui_config.h"

class QTimer;
class QLineEdit;

class QWebView;
class QGraphicsProxyWidget;
class QGraphicsLinearLayout;

class Dict : public Plasma::Applet
{
    Q_OBJECT
    public:
        Dict(QObject *parent, const QVariantList &args);
        ~Dict();

	void init();
        void setPath(const QString&);
        void constraintsEvent(Plasma::Constraints);

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void autoDefine(const QString &word);
        void linkDefine(const QString &word);

    protected slots:
//         void acceptedTimeStringState(bool);
        void configAccepted();
        void define();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);

    private:
        QString wnToHtml(const QString& text);

        QVariant m_thedef;
        QLineEdit *m_wordChooser;
        QString m_word;
        QTimer* m_timer;
        int m_autoDefineTimeout;
        QGraphicsPixmapItem *m_graphicsIcon; 
	QGraphicsLinearLayout *m_layout;
        KLineEdit *m_wordEdit;
        //Plasma::Flash *m_flash;
        QStringList m_defList;
        QStringList::iterator m_i;
        Ui::config ui;
        QWebView *m_defBrowser;
        QGraphicsProxyWidget *m_defBrowserProxy;
	QGraphicsProxyWidget *m_wordProxyWidget;
};

K_EXPORT_PLASMA_APPLET(dict, Dict)

#endif
