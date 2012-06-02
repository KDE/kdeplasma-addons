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


#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <KRichTextEdit>

#include "ui_config.h"

class QContextMenuEvent;

namespace Plasma
{
    class Applet;
} // namespace Plasma

/**
 * @short Notes Version von KTextEdit
 *
 * This is just a little subclass of KTextEdit which provides a customized context menu and the ability to save its content into a file
 *
 * @see QTextEdit
 * @author Björn Ruberg <bjoern@ruberg-wegener.de>
 */
class NotesTextEdit : public KRichTextEdit
{
    Q_OBJECT

    public:
        NotesTextEdit(Plasma::Applet *applet, QWidget *parent = 0);
        ~NotesTextEdit();

        void setFormatMenu(QMenu *menu);

    public slots:
        virtual void saveToFile();
        void italic();
        void bold();
        void underline();
        void strikeOut();
        void justifyCenter();
        void justifyFill();

    Q_SIGNALS:
        void cursorMoved();
        void scrolledUp();
        void scrolledDown();
        void error(const QString &message);

    protected:
        virtual void contextMenuEvent(QContextMenuEvent *e);
        void mousePressEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void focusOutEvent(QFocusEvent *event);
        void wheelEvent(QWheelEvent *event);

    private:
        Plasma::Applet *m_applet;
        QMenu *m_formatMenu;
};

#endif
