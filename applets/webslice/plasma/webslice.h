/***************************************************************************
 *   Copyright 2009-2011 by Sebastian Kügler <sebas@kde.org>               *
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

#ifndef WEBSLICE_HEADER
#define WEBSLICE_HEADER

#include <QString>
#include <plasma/popupapplet.h>
#include "ui_websliceConfig.h"

class KGraphicsWebSlice;
class KConfigDialog;

class WebSlice: public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        WebSlice(QObject *parent, const QVariantList &args);
        ~WebSlice();
        void init();

    protected:
        QGraphicsWidget *graphicsWidget();
        KGraphicsWebSlice *m_slice;

    protected Q_SLOTS:
        void createConfigurationInterface(KConfigDialog *parent);
        void configAccepted();

    public slots:
        void configChanged();

    private slots:
        void loadFinished(bool);
        void loadUrl();
        void updateElements();
        void disconnectLoadFinished();
        void preview(const QString &selector);
        void updateColors();
        void handleGeometryEdit();

    private:
        QString sliceGeometryToString(const QString &selector = QString());
        void loadSlice(const QUrl &url, const QString &selector = QString("body"));
        QUrl m_url;
        QString m_element;
        QRectF m_sliceGeometry;
        Ui::websliceConfig ui;
        QGraphicsWidget *m_widget;
};

K_EXPORT_PLASMA_APPLET(webslice, WebSlice)

#endif
