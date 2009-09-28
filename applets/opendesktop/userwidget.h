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
#include <Plasma/DataEngine>

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
        explicit UserWidget(Plasma::DataEngine* engine, QGraphicsWidget *parent = 0);
        virtual ~UserWidget();
        void setId(const QString& id);

    public Q_SLOTS:
        void updateColors();
        //void loadStyleSheet(const QString &cssFile = "");
        void setStyleSheet(const QString &stylesheet);

    private Q_SLOTS:
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data& data);

    private :
        void buildDialog();
        void setName();
        void setInfo();

        QString addRow(const QString& title, const QString& text);

        // Caches the data
        Plasma::DataEngine::Data m_data;

        StyleSheet* m_css;

        QString m_cssFile;
        QString m_email;
        // Caches the content part of the widget
        QString m_info;
        // Caches the title part of the widget
        QString m_name;
        // The applet attached to this item
        QGraphicsGridLayout* m_layout;
        ContactImage* m_image;
        Plasma::Label* m_nameLabel;
        Plasma::WebView* m_infoView;
        // The user id to display
        QString m_id;
        // The data engine used
        Plasma::DataEngine* m_engine;
};

#endif

