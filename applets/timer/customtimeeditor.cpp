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

const QString CustomTimeEditor::TIME_FORMAT("hh:mm:ss");

CustomTimeEditor::CustomTimeEditor()
    : QObject()
{
    timeEdit=new QTimeEdit();
    timeEdit->setDisplayFormat(TIME_FORMAT);
    editor=new KLineEdit();
    customEditor=new KEditListBox::CustomEditor(timeEdit, editor);
    connect(timeEdit, SIGNAL(timeChanged( const QTime& )), this, SLOT(setEdit(const QTime&) ));
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
    editor->setText(time.toString() );
}

#include "customtimeeditor.moc"

/*!
    \fn CustomTimeEditor::getCustomEditor()
 */
KEditListBox::CustomEditor *CustomTimeEditor::getCustomEditor()
{
    return customEditor;
}
