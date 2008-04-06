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

#include "LancelotPart.h"
#include "KDebug"
#include "KIcon"

#include "../src/models/Places.h"

LancelotPart::LancelotPart(QObject *parent, const QVariantList &args) :
    /*Lancelot::WidgetCore(),*/ Plasma::Applet(parent, args)
{
    instance = new Lancelot::Instance();
    //instance()->addWidget(this);

    setDrawStandardBackground(true);

    list = new Lancelot::ActionListView("Noname", new Lancelot::Models::Places(), this);

    setGroup(instance->defaultGroup());
    instance->activateAll();

    setAcceptsHoverEvents(true);
}

LancelotPart::~LancelotPart() {
    delete instance;
}

QSizeF LancelotPart::contentSizeHint () const {
    return QSizeF(200, 300);
}

void LancelotPart::setGeometry(const QRectF & geometry)
{
    Applet::setGeometry(geometry);
    if (group() && list) {
        list->setGeometry(QRectF(QPoint(), contentSize()));
    }
}

//#include "LancelotPart.maoc"
