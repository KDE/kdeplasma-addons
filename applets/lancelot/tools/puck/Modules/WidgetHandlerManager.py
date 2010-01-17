#  Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2,
#  or (at your option) any later version, as published by the Free
#  Software Foundation
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, write to the
#  Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.


__handlers = {};
__roots = []

def handlers():
    return __handlers

def handler(widgetName):
    if not __handlers.has_key(widgetName):
        print "WARNING: Widget has no handler ", widgetName, " USING DEFAULT HANDLER"
        return __handlers["QGraphicsWidget"]
    return __handlers[widgetName]

def addHandler(handler):
    __handlers[handler.name()] = handler

def pushRoot(root):
    __roots.append(root)

def popRoot():
    __roots.pop()

def root():
    return __roots[-1]
