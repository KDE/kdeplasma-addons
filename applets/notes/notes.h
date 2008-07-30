/***************************************************************************
 *   Copyright (C) 2007 Lukas Kropatschek <lukas.krop@kdemail.net>         *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
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


#ifndef NOTES_HEADER
#define NOTES_HEADER

#include <QTimer>

#include <Plasma/Applet>
#include <Plasma/Svg>

#include "ui_config.h"

class QGraphicsLinearLayout;
class QSizeF;

namespace Plasma
{
    class TextEdit;
}

class Notes : public Plasma::Applet
{
    Q_OBJECT

    public:
        Notes(QObject *parent, const QVariantList &args);
        ~Notes();
        void init();
        QList<QAction*> contextualActions();
        void paintInterface(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            const QRect& contentsRect);

    public Q_SLOTS:
        void configAccepted();

    protected:
        void constraintsEvent(Plasma::Constraints constraints);
        void createConfigurationInterface(KConfigDialog *parent);

    private Q_SLOTS:
        void saveNote();
        void delayedSaveNote();
        void changeColor();

    private:
        int fontSize();
        int m_autoFontPercent;
        bool m_autoFont;
        bool m_checkSpelling;
        void updateTextGeometry();
        void addColor(const QString &id, const QString &colorName);

        QTimer m_saveTimer;
        QFont m_font;
        QColor m_textColor;
        QString m_color;

        QList<QAction*> m_colorActions;

        Plasma::Svg m_notes_theme;
        QGraphicsLinearLayout *m_layout;
        Plasma::TextEdit *m_textEdit;
        Ui::config ui;

        QSizeF m_size;
};

K_EXPORT_PLASMA_APPLET(notes, Notes)

#endif
