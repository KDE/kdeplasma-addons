#  Copyright (C) 2007 Ivan Cukic <ivan.cukic+nymph@gmail.com>
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

def handlers():
    return __handlers

def handler(layoutName):
    if not __handlers.has_key(layoutName):
        print "WARNING: Layout has no handler ", layoutName, " USING DEFAULT HANDLER"
        return __handlers["Abstract"]
    return __handlers[layoutName]

def addHandler(handler):
    __handlers[handler.name()] = handler
