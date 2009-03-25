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
#include <KTextEdit>

#include <Plasma/Applet>
#include <Plasma/Svg>
#include <Plasma/TextEdit>

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
class NotesTextEdit : public KTextEdit {
    Q_OBJECT

    public:
	NotesTextEdit(QWidget *parent = 0);
	~NotesTextEdit();
	virtual void contextMenuEvent ( QContextMenuEvent *e )  ;
	void mousePressEvent ( QMouseEvent * event );
	void keyPressEvent ( QKeyEvent * event );
	void leaveEvent ( QEvent * event );

    signals:
	void cursorMoved();
	void mouseUnhovered();

    public slots:
	virtual void saveToFile();
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

    protected:
        void constraintsEvent(Plasma::Constraints constraints);
        void createConfigurationInterface(KConfigDialog *parent);

    private Q_SLOTS:
        void saveNote();
        void delayedSaveNote();
        void changeColor();
	void lineChanged();
	void mouseUnhovered();

    private:
        int fontSize();
        int m_autoFontPercent;
        bool m_autoFont;
        bool m_checkSpelling;
        void updateTextGeometry();
        void addColor(const QString &id, const QString &colorName);

        QTimer m_saveTimer;
        QFont m_font;
        int m_customFontSize;
        bool m_useThemeColor;
	bool m_useNoColor;
        QColor m_textColor;
	QColor m_textBackgroundColor;
        QString m_color;

        QList<QAction*> m_colorActions;

        Plasma::Svg m_notes_theme;
        QGraphicsLinearLayout *m_layout;
        PlasmaTextEdit *m_textEdit;
        Ui::config ui;

        QSizeF m_size;
};

K_EXPORT_PLASMA_APPLET(notes, Notes)

#endif
