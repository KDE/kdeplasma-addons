/***************************************************************************
 *   Copyright (C) 2009 by Sebastian Kügler <sebas@kde.org>                *
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
        void constraintsEvent(Plasma::Constraints   constraints);
        KGraphicsWebSlice *m_slice;

    protected Q_SLOTS:
        void createConfigurationInterface(KConfigDialog *parent);
        void configAccepted();

    public slots:
        void configChanged();
        
    private slots:
        void sizeChanged (QSizeF newsize);
        void loadFinished(bool);

    private:
        QString sliceGeometryToString();
        QUrl m_url;
        QString m_element;
        QSizeF m_size;
        QRectF m_sliceGeometry;
        Ui::websliceConfig ui;
        QGraphicsWidget *m_widget;
};

K_EXPORT_PLASMA_APPLET(webslice, WebSlice)

#endif
