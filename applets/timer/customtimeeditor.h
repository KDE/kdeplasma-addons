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

#ifndef CUSTOMTIMEEDITOR_H
#define CUSTOMTIMEEDITOR_H

#include <keditlistbox.h>
#include <QTime>

class QTimeEdit;
class KLineEdit;
/**
	@author
*/
class CustomTimeEditor : public QObject
{
Q_OBJECT
public:
    CustomTimeEditor();

    ~CustomTimeEditor();
    KEditListBox::CustomEditor *getCustomEditor();
    static QString timerSeparator();
    static QString toLocalizedTimer(const QString &timer);
    static QString fromLocalizedTimer(const QString &timer);
    static const QString TIME_FORMAT;
private:
    QTimeEdit* timeEdit;
    KLineEdit* editor;
    KEditListBox::CustomEditor *customEditor;
protected slots:
    void setEdit(const QTime &time);
};

#endif
