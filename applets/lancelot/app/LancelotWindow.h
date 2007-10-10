/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOTWINDOW_H_
#define LANCELOTWINDOW_H_

#include <QtGui>
#include <QtCore>

#include "ui_LancelotWindow.h"
#include "ActionListView.h"

namespace Plasma {
    class Phase;
}

class DummyListModel : public Lancelot::ActionListViewModel {
public:
    DummyListModel(QString title, int size) : Lancelot::ActionListViewModel(), m_size(size), m_title(title) {}
    virtual ~DummyListModel() {}
    
    virtual QString title(int index) const {
        if (index % 10 == 3) {
            return "\nCategory " + QString::number(index) + QString((index < size())?"":"err"); 
        }
        return m_title + QString::number(index) + QString((index < size())?"":"err"); 
    }
    
    virtual QString description(int index) const {
        if (index % 10 == 3) return "";
        return "Description " + QString::number(index); 
    }
    virtual QIcon * icon(int index) const { Q_UNUSED(index); return NULL; }
    virtual int size() const { return m_size; }
private:
    int m_size;
    QString m_title;
};


class LancelotWindow : public QFrame, public Ui::LancelotWindow
{
    Q_OBJECT
public:
	LancelotWindow( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~LancelotWindow();
	
    bool lancelotShow();
    bool lancelotHide();
    bool lancelotShowItem(QString name);

protected:
    void leaveEvent(QEvent * event);
    void enterEvent(QEvent * event);
    void focusOutEvent(QFocusEvent * event);
    
private Q_SLOTS:
    void itemActivated(const QString & item);
    
private:
    QTimer m_hideTimer;
    bool m_hovered;
    QSignalMapper * m_sectionsSignalMapper;
    Plasma::Phase * m_phase;
};

#endif /*LANCELOTWINDOW_H_*/
