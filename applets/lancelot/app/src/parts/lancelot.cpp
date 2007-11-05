/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "lancelot.h"
#include "KDebug"
#include "KIcon"

#include "../src/Global.h"
#include "../src/ActionListView.h"
#include "../src/models/Places.h"

LancelotPart::LancelotPart(QObject *parent, const QVariantList &args) :
    Plasma::Applet(parent, args)
{
    setDrawStandardBackground(true);
    Lancelot::Instance * instance = new Lancelot::Instance();
    Lancelot::ActionListView * list = new Lancelot::ActionListView("Noname", new Lancelot::Models::Places(), this);
    instance->activateAll();
    list->setGeometry(QRectF(0,0, 200,300));
    
    setAcceptsHoverEvents(true);
}

LancelotPart::~LancelotPart() {
}

QSizeF LancelotPart::contentSizeHint () const {
    return QSizeF(200, 300);
}


#include "lancelot.moc"
