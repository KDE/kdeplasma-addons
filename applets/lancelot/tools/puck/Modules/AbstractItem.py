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

from . import debug

class AbstractItemHandler:
    __node = 0

    def __init__(self):
        self.__node = 0

    """ @returns the full C++ name for this class """
    def name(self):
        return "Abstract"

    """ @returns the XML namespace for this class (for use in the PUCK XML) """
    def namespace(self):
        return self.name().replace(":", "_")

    def setNode(self, node):
        self.__node = node

    def node(self):
        return self.__node

    """ @returns the C++ inlcudes for needed by the class """
    def include(self):
        return "/* " + self.attribute("type") + " declaration - not implemented */"

    """ @returns the C++ variable declaration """
    def declaration(self):
        return self.attribute("type") + " * " + self.attribute("name") + ";"

    """ @returns the C++ variable initialization """
    def initialization(self):
        return self._construction() + ";";

    def _construction(self):
        return self.attribute("name") + " = new " + self.attribute("type") + "()"

    """ @returns the C++ object setup (properties etc.) """
    def setup(self):
        return ""

    """ @returns whether the active node has the specified attribute """
    def hasAttribute(self, attrName):
        debug.message(self.name(), " wants attribute " + self.namespace() + ":" + attrName)
        return \
            self.node().hasAttribute(self.name() + ":" + attrName) or \
            self.node().hasAttribute(attrName)

    """ @returns the value of the active node's attribute """
    def attribute(self, attrName):
        debug.message(self.name(), " retrieves attribute " + self.namespace() + ":" + attrName)

        result = None

        if self.node().hasAttribute(self.name() + ":" + attrName):
            result = self.node().getAttribute(self.name() + ":" + attrName)

        elif self.node().hasAttribute(attrName):
            result = self.node().getAttribute(attrName)

        if not result == None:
            debug.message(" return ", result)
        else:
            debug.message(" return ", " NONE ")

        return result
