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

#include <QGraphicsLinearLayout>
#include <QTimer>

#include <QToolButton>
#include <KTextEdit>
#include <KRichTextEdit>

#include <Plasma/Applet>
#include <Plasma/Svg>
#include <Plasma/TextEdit>
#include <Plasma/ToolButton>

#include "ui_config.h"

class QGraphicsLinearLayout;
class QSizeF;
class QContextMenuEvent;
class QGraphicsWidget;

namespace Plasma
{
    class TextEdit;
}

/**
 * @short Notes Version von KTextEdit
 *
 * This is just a little subclass of KTextEdit which provides a customized context menu and the ability to save its content into a file
 *
 * @see QTextEdit
 * @author Björn Ruberg <bjoern@ruberg-wegener.de>
 */
class NotesTextEdit : public KRichTextEdit {
    Q_OBJECT

    public:
        NotesTextEdit(QWidget *parent = 0);
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
        void mouseUnhovered();
        void scrolledUp();
        void scrolledDown();

    protected:
        virtual void contextMenuEvent ( QContextMenuEvent *e )  ;
        void mousePressEvent ( QMouseEvent * event );
        void keyPressEvent ( QKeyEvent * event );
        void leaveEvent ( QEvent * event );
        void wheelEvent ( QWheelEvent * event );

    private:
        QMenu *m_formatMenu;
};

/**
 * @short Notes Version von KTextEdit
 *
 * This is a Plasma::TextEdit which uses NotesTextEdit as native widget
 *
 * @see QTextEdit
 * @author Björn Ruberg <bjoern@ruberg-wegener.de>
 */
class PlasmaTextEdit : public Plasma::TextEdit {
    Q_OBJECT

    signals:
    void mouseUnhovered();

    public:
      PlasmaTextEdit(QGraphicsWidget *parent = 0);
      ~PlasmaTextEdit();
      NotesTextEdit* native;
};

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
        void showOptions(bool show);

    protected:
        void constraintsEvent(Plasma::Constraints constraints);
        void createConfigurationInterface(KConfigDialog *parent);

    private Q_SLOTS:
        void saveNote();
        void delayedSaveNote();
        void changeColor(QAction*);
        void lineChanged();
        void mouseUnhovered();
        void increaseFontSize();
        void decreaseFontSize();

    private:
        void createTextFormatingWidgets();
        int fontSize();
        void updateTextGeometry();
        void addColor(const QString &id, const QString &colorName);

        int m_autoFontPercent;
        bool m_autoFont;
        bool m_checkSpelling;

        QTimer m_saveTimer;
        QFont m_font;
        int m_customFontSize;
        bool m_useThemeColor;
        bool m_useNoColor;
        int m_wheelFontAdjustment;
        QColor m_textColor;
        QColor m_textBackgroundColor;
        QString m_color;

        QMenu *m_colorMenu;
        QMenu *m_formatMenu;

        Plasma::Svg m_notesTheme;
        QGraphicsLinearLayout *m_layout;
        PlasmaTextEdit *m_textEdit;
        Ui::config ui;

        QSizeF m_size;

        Plasma::ToolButton *m_buttonOption;
        Plasma::ToolButton *m_buttonBold;
        Plasma::ToolButton *m_buttonItalic;
        Plasma::ToolButton *m_buttonUnderline;
        Plasma::ToolButton *m_buttonStrikeThrough;
        Plasma::ToolButton *m_buttonCenter;
        Plasma::ToolButton *m_buttonFill;
};

K_EXPORT_PLASMA_APPLET(notes, Notes)

#endif
