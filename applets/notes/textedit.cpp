/***************************************************************************
 *   Copyright (C) 2007 Lukas Kropatschek <lukas.krop@kdemail.net>         *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
 *   Copyright (C) 2008 Davide Bettio <davide.bettio@kdemail.net>          *
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

#include "textedit.h"

#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QScrollBar>
#include <QtGui/QMenu>

#include <KAction>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KGlobalSettings>
#include <KFileDialog>
#include <KIcon>
#include <KStandardAction>

#include <Plasma/Applet>

NotesTextEdit::NotesTextEdit(Plasma::Applet *applet, QWidget *parent)
    : KRichTextEdit(parent),
      m_applet(applet),
      m_formatMenu(0)
{
}

NotesTextEdit::~NotesTextEdit()
{
}

/**
 * Save content to file
 */
void NotesTextEdit::saveToFile()
{
    QString fileName = KFileDialog::getSaveFileName();

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        emit error(file.errorString());
        return;
    }

    QTextStream out(&file);
    out << toPlainText();
    file.close();
}

void NotesTextEdit::setFormatMenu(QMenu *menu)
{
    m_formatMenu = menu;
}

/**
 * Customize the context menu
 */
void NotesTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *popup = mousePopupMenu();

    popup->addSeparator();
    popup->addAction(KStandardAction::saveAs(this, SLOT(saveToFile()), this));

    if (m_formatMenu) {
        popup->addMenu(m_formatMenu);
    }

    popup->exec(event->pos());
    delete popup;
}

void NotesTextEdit::italic()
{
    setTextItalic(!fontItalic());
}

void NotesTextEdit::bold()
{
    setTextBold(fontWeight() != 75);
}

void NotesTextEdit::underline()
{
    setTextUnderline(!fontUnderline());
}

void NotesTextEdit::strikeOut()
{
    QFont a = currentFont();
    setTextStrikeOut(!a.strikeOut());
}

void NotesTextEdit::justifyCenter()
{
    if (alignment() == Qt::AlignHCenter) {
        setAlignment(Qt::AlignLeft);
    } else {
        alignCenter();
    }
}

void NotesTextEdit::justifyFill()
{
    if (alignment() == Qt::AlignJustify) {
        setAlignment(Qt::AlignLeft);
    } else {
        alignJustify();
    }
}

/**
 * Add to mousePressEvent a signal to change the edited line's background color
 */
void NotesTextEdit::mousePressEvent ( QMouseEvent * event )
{
    KTextEdit::mousePressEvent(event);
    if(event->button()== Qt::LeftButton)
      emit cursorMoved();
}

/**
 * Same as mousePressEvent
 */
void NotesTextEdit::keyPressEvent ( QKeyEvent * event )
{
  KTextEdit::keyPressEvent(event);
  switch(event->key())
  {
    case Qt::Key_Left : emit cursorMoved(); break;
    case Qt::Key_Right : emit cursorMoved(); break;
    case Qt::Key_Down : emit cursorMoved(); break;
    case Qt::Key_Up : emit cursorMoved(); break;
    default:break;
  }
}

/**
* Scale text on wheel scrolling with control pressed
*/
void NotesTextEdit::wheelEvent ( QWheelEvent * event )
{
    if (event->modifiers() & Qt::ControlModifier ) {
        if (event->orientation() == Qt::Horizontal) {
            return;
        } else {
            if (event->delta() > 0) {
                emit scrolledUp();
            } else {
                emit scrolledDown();
            }
        }
        event->accept();
    } else {
        KTextEdit::wheelEvent(event);
    }
}

/**
 * Add to the Note a signal to prevent from leaving the note and remove line background color
 * when there is no focus on the plasmoid
 */
void NotesTextEdit::focusOutEvent ( QFocusEvent * event )
{
    KTextEdit::focusOutEvent(event);

    QTextEdit::ExtraSelection textxtra;
    textxtra.cursor = textCursor();
    textxtra.cursor.movePosition(QTextCursor::StartOfLine);
    textxtra.cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    textxtra.format.setBackground(Qt::transparent);

    QList<QTextEdit::ExtraSelection> extras;
    extras << textxtra;
    setExtraSelections(extras);
}

#include "textedit.moc"

