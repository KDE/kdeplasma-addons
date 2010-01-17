#!/usr/bin/env python

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

import sys, os

import xml.dom.minidom
from xml.dom.minidom import Node

from Modules.Layouts import *
from Modules.Widgets import *

from Modules import LayoutHandlerManager
from Modules import WidgetHandlerManager

from Modules import debug

stmtDefine         = ""
stmtDeclaration    = ""
stmtInitialization = ""
stmtInclude       = set()
stmtSetup          = ""
WidgetHandlerManager.pushRoot("root")

def processDefines(node):
    global stmtDefine

    for defineNode in node.childNodes:
        if not defineNode.nodeType == xml.dom.Node.ELEMENT_NODE:
            continue
        if defineNode.localName == "define":
            stmtDefine += "#define " + defineNode.getAttribute("name") + " " + defineNode.getAttribute("value") + "\n"

def processElement(node):
    global stmtDefine
    global stmtInclude
    global stmtDeclaration
    global stmtInitialization
    global stmtSetup

    if node.nodeType == xml.dom.Node.COMMENT_NODE:
        stmtSetup += "/*" + node.nodeValue + "*/\n"
        return 0

    if not node.nodeType == xml.dom.Node.ELEMENT_NODE:
        return 0

    if node.prefix == "code" or node.localName == "code":
        for child in node.childNodes:
            if node.localName == "define":
                stmtDefine += child.nodeValue + "\n"
            elif node.localName == "include":
                stmtInclude.add(child.nodeValue)
            elif node.localName == "declaration":
                stmtDeclaration += child.nodeValue + "\n"
            elif node.localName == "initialization":
                stmtInitialization += child.nodeValue + "\n"
            else:
                stmtSetup += child.nodeValue + "\n"
    elif node.prefix == "item":
        if node.localName == "layout":
            processLayout(node)
        elif node.localName == "widget":
            processWidget(node)
        elif node.localName == "qwidget":
            processQWidget(node)


def processLayout(node):
    global stmtDeclaration
    global stmtInitialization
    global stmtSetup

    handler = LayoutHandlerManager.handler(node.getAttribute("type")) # TODO: make this check if layout is supported
    handler.setNode(node)

    includes = handler.include()
    for include in includes.split("\n"):
        stmtInclude.add(include)

    stmtDeclaration    += handler.declaration() + "\n"
    stmtInitialization += handler.initialization() + "\n"
    stmtSetup          += handler.setup() + "\n"

    for child in node.childNodes:
        processElement(child)

def processWidget(node):
    global stmtDeclaration
    global stmtInitialization
    global stmtSetup

    handler = WidgetHandlerManager.handler(node.getAttribute("type")) # TODO: make this check if widget is supported
    handler.setNode(node)

    includes = handler.include()
    for include in includes.split("\n"):
        stmtInclude.add(include)

    stmtDeclaration    += handler.declaration() + "\n"
    stmtInitialization += handler.initialization() + "\n"
    stmtSetup          += handler.setup() + "\n"

    WidgetHandlerManager.pushRoot(node.getAttribute("name"))
    for child in node.childNodes:
        processElement(child)
    WidgetHandlerManager.popRoot()

# Main program: ##################################################################################

print "Plasma UI Compiler 0.1"
print "Generating ", sys.argv[2]
print "From ", sys.argv[1]

doc = xml.dom.minidom.parse(sys.argv[1])

if not doc.documentElement.localName == "pui":
    exit

rootObjectType = ""
className      = ""

for node in doc.documentElement.childNodes:
    if not node.nodeType == xml.dom.Node.ELEMENT_NODE:
        continue
    elif node.localName == "defines":
        processDefines(node)
    elif node.localName == "ui":
        rootObjectType = node.getAttribute("rootObjectType")
        className      = node.getAttribute("className")

        for child in node.childNodes:
            processElement(child)

template = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "template/cpp.h"))
template = template.readlines()
template = "".join(template)

output = open(sys.argv[2], 'w')

print >> output, template\
    .replace("${HEADER_ID}",          "PUI_" + className.upper() + "_H") \
    .replace("${CLASS_NAME}",         className) \
    .replace("${PARENT_OBJECT_TYPE}", rootObjectType) \
    .replace("${INCLUDES}",           "\n".join(stmtInclude)) \
    .replace("${DEFINES}",            stmtDefine) \
    .replace("${DECLARATION}",        stmtDeclaration) \
    .replace("${INITIALIZATION}",     stmtInitialization) \
    .replace("${SETUP}",              stmtSetup)

output.close()
