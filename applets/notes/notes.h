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

#include <Plasma/PopupApplet>
#include <Plasma/Svg>
#include <Plasma/TextEdit>
#include <Plasma/ToolButton>

#include "ui_config.h"

class QGraphicsLinearLayout;
class QSizeF;
class QContextMenuEvent;
class QParallelAnimationGroup;

class TopWidget;
class NotesTextEdit;

class Notes : public Plasma::PopupApplet
{
    Q_OBJECT

    public:
        Notes(QObject *parent, const QVariantList &args);
        ~Notes();
        void init();
        QList<QAction*> contextualActions();

    public Q_SLOTS:
        void configChanged();
        void configAccepted();
        void showOptions(bool show);

    protected:
        void constraintsEvent(Plasma::Constraints constraints);
        void createConfigurationInterface(KConfigDialog *parent);
        void saveState(KConfigGroup &cg) const;

    private Q_SLOTS:
        void saveNote();
        void delayedSaveNote();
        void changeColor(QAction*);
        void lineChanged();
        void increaseFontSize();
        void decreaseFontSize();
        void themeChanged();
        void updateOptions();
        void showError(const QString &message);

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

        QMenu *m_colorMenu;
        QMenu *m_formatMenu;

        QGraphicsLinearLayout *m_layout;
        Plasma::TextEdit *m_textEdit;
        NotesTextEdit *m_noteEditor;
        Ui::config ui;

        QSizeF m_size;

        Plasma::ToolButton *m_buttonOption;
        Plasma::ToolButton *m_buttonBold;
        Plasma::ToolButton *m_buttonItalic;
        Plasma::ToolButton *m_buttonUnderline;
        Plasma::ToolButton *m_buttonStrikeThrough;
        Plasma::ToolButton *m_buttonCenter;
        Plasma::ToolButton *m_buttonFill;
        TopWidget *m_topWidget;

        QParallelAnimationGroup *m_buttonAnimGroup;
        Plasma::Animation *m_buttonAnim[6];
};

#endif
