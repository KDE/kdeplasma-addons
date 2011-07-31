/***************************************************************************
 *   Copyright 2008 by Marco Gulino <marco.gulino@gmail.com                *
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

#include "customtimeeditor.h"
#include <QTimeEdit>
#include <klineedit.h>
#include <klocale.h>

const QString CustomTimeEditor::TIME_FORMAT("hh:mm:ss");

CustomTimeEditor::CustomTimeEditor()
    : QObject()
{
    timeEdit=new QTimeEdit();
    timeEdit->setDisplayFormat(toLocalizedTimer(TIME_FORMAT));
    editor=new KLineEdit();
    customEditor=new KEditListWidget::CustomEditor(timeEdit, editor);
    connect(timeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(setEdit(QTime)));
}


CustomTimeEditor::~CustomTimeEditor()
{
    delete timeEdit;
    delete editor;
    delete customEditor;
}




/*!
    \fn CustomTimeEditor::setEdit(const QTime &time)
 */
void CustomTimeEditor::setEdit(const QTime &time)
{
    editor->setText(toLocalizedTimer(time.toString()));
}


/*!
    \fn CustomTimeEditor::getCustomEditor()
 */
KEditListWidget::CustomEditor *CustomTimeEditor::getCustomEditor()
{
    return customEditor;
}

/*!
    \fn CustomTimeEditor::timerSeparator()
 */
QString CustomTimeEditor::timerSeparator()
{
    return i18nc("separator of hours:minutes:seconds in timer strings", ":");
}

/*!
    \fn CustomTimeEditor::toLocalizedTimer(const QString &timer)
 */
QString CustomTimeEditor::toLocalizedTimer(const QString &timer)
{
    QString separator = timerSeparator();
    int p1 = timer.indexOf(':');
    if (p1 < 0) {
        return timer;
    }
    int p2 = timer.indexOf(':', p1 + 1);
    if (p2 < 0) {
        return timer;
    }
    return timer.left(p1) + separator + timer.mid(p1 + 1, p2 - p1 - 1) + separator + timer.mid(p2 + 1);
}

/*!
    \fn CustomTimeEditor::fromLocalizedTimer(const QString &timer)
 */
QString CustomTimeEditor::fromLocalizedTimer(const QString &timer)
{
    QString separator = timerSeparator();
    int p1 = timer.indexOf(separator);
    if (p1 < 0) {
        return timer;
    }
    int p2 = timer.indexOf(separator, p1 + 1);
    if (p2 < 0) {
        return timer;
    }
    return timer.left(p1) + ':' + timer.mid(p1 + 1, p2 - p1 - 1) + ':' + timer.mid(p2 + 1);
}
#include "customtimeeditor.moc"

