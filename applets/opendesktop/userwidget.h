/***************************************************************************
 *   Copyright 2008-2009 by Sebastian Kügler <sebas@kde.org>               *
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

#ifndef USERWIDGET_H
#define USERWIDGET_H

//Qt
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>

// Plasma
#include <Plasma/Frame>

namespace Plasma
{
    class Label;
    class Frame;
    class WebView;
}

class ContactImage;
class StyleSheet;

class UserWidget : public Plasma::Frame
{
    Q_OBJECT

    public:
        UserWidget(QGraphicsWidget *parent = 0);
        virtual ~UserWidget();
        void setName(const QString &name = "");
        void setInfo(const QString &name = "");

    public Q_SLOTS:
        void setAtticaData(Plasma::DataEngine::Data data);
        void updateColors();
        //void loadStyleSheet(const QString &cssFile = "");
        void setStyleSheet(const QString &stylesheet);

    private :
        void buildDialog();

        QString addRow(const QString& title, const QString& text);

        Plasma::DataEngine::Data m_atticaData;

        StyleSheet* m_css;

        QString m_cssFile;
        QString m_email;
        QString m_info;
        // The applet attached to this item
        QGraphicsGridLayout* m_layout;
        ContactImage* m_image;
        Plasma::Label* m_nameLabel;
        Plasma::WebView* m_infoView;
};

#endif

