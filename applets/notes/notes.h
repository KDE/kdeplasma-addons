/*
    Copyright (C) 2007 Lukas Kropatschek <lukas.krop@kdemail.net> 
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
    This program is distributed in the hope that it will be useful,       
    but WITHOUT ANY WARRANTY; without even the implied warranty of        
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
    GNU General Public License for more details.                          
                                                                          
    You should have received a copy of the GNU General Public License     
    along with this program; if not, write to the                         
    Free Software Foundation, Inc.,                                       
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        
 */

#ifndef NOTES_HEADER
#define NOTES_HEADER

#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QTextEdit>


#include <Plasma/Applet>
#include <Plasma/Svg>

#include "ui_config.h"

class KDialog;
class QFont;
class QSizeF;

class Notes : public Plasma::Applet
{
    Q_OBJECT

    public:
        Notes(QObject *parent, const QVariantList &args);
        ~Notes();
        void init();
        void paintInterface(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            const QRect& contentsRect);
        void constraintsUpdated(Plasma::Constraints constraints);

    public slots:
        void createConfigurationInterface(KConfigDialog *parent);
        void configAccepted();
        void saveNote();

    private:
        void updateTextGeometry();
        Plasma::Svg m_notes_theme;
        QGraphicsLinearLayout *m_layout;
        QGraphicsProxyWidget *m_proxy;
        QTextEdit *m_textEdit;
        Ui::config ui;
        //KDialog *m_dialog;

        QSizeF m_size;
};

K_EXPORT_PLASMA_APPLET(notes, Notes)

#endif
