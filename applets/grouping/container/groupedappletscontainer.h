/***************************************************************************
 *   Copyright (C) 2015 Marco Martin <mart@kde.org>                        *
 *   Copyright (C) 2016 David Edmundson <davidedmundson@kde.org>           *
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

#ifndef GROUPEDAPPLETSCONTAINER_H
#define GROUPEDAPPLETSCONTAINER_H


#include <Plasma/Applet>

class QQuickItem;

class GroupedAppletsContainer : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *internalContainmentItem READ internalContainmentItem NOTIFY internalContainmentItemChanged)

public:
    explicit GroupedAppletsContainer(QObject *parent, const QVariantList &args);
    ~GroupedAppletsContainer() override;

    void init() override;

    QQuickItem *internalContainmentItem();

protected:
    void constraintsEvent(Plasma::Types::Constraints constraints) override;
    void ensureSystrayExists();

Q_SIGNALS:
    void internalContainmentItemChanged();

private:
    QPointer<Plasma::Containment> m_innerContainment;
    QPointer<QQuickItem> m_internalContainmentItem;
};

#endif
