/***************************************************************************
 *   Copyright 2008 by Laurent Montel <montel@kde.org>                     *
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

#ifndef CHARSELECT_H
#define CHARSELECT_H

#include <Plasma/PopupApplet>

class KCharSelect;
class QGraphicsGridLayout;
class KLineEdit;
class KPushButton;

class CharSelectApplet : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    CharSelectApplet(QObject *parent, const QVariantList &args);
    virtual ~CharSelectApplet();

    virtual QWidget *widget();

protected:
    void constraintsEvent(Plasma::Constraints);

public slots:
    void slotAddToClipboard();
    void slotCharSelect( const QChar &c );

private:
    QWidget *m_mainWidget;
    KCharSelect *m_charselect;
    QGraphicsGridLayout *m_layout;
    KLineEdit *m_lineEdit;
    KPushButton *m_addToClipboard;
};

K_EXPORT_PLASMA_APPLET(CharSelectApplet, CharSelectApplet)

#endif
