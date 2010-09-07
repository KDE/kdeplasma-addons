/***********************************************************************************
* Spell Check: Plasmoid for fast spell checking.
* Copyright (C) 2008 - 2010 Michal Dutkiewicz aka Emdek <emdeck@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
***********************************************************************************/

#ifndef SPELLCHECK_HEADER
#define SPELLCHECK_HEADER

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneDragDropEvent>

#include <KTextEdit>

#include <Plasma/Applet>
#include <Plasma/Dialog>

#include <Sonnet/Highlighter>
#include <Sonnet/DictionaryComboBox>

class SpellCheck : public Plasma::Applet
{
    Q_OBJECT

public:
    SpellCheck(QObject *parent, const QVariantList &args);
    ~SpellCheck();

    void init();

public slots:
    void configChanged();

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void timerEvent(QTimerEvent *event);

protected slots:
    void toggleDialog(bool pasteText = true, bool preferSelection = true);
    void dialogResized();
    void copyToClipboard();
    void setLanguage(const QString &language);

private:
    KTextEdit *m_textEdit;
    Plasma::Dialog *m_spellingDialog;
    Sonnet::DictionaryComboBox *m_dictionaryComboBox;
    int m_dragTimer;
};

#endif
