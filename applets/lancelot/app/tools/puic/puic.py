#!/usr/bin/python2.5

#
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
#

import sys

import xml.dom.minidom
from xml.dom.minidom import Node

from Modules.Layouts import *
from Modules import LayoutHandlerManager

stmtDefine         = ""
stmtDeclaration    = ""
stmtInitialization = ""
stmtSetup          = ""

def debug(msg, var):
    print >> sys.stdout, "------------------------ : ",  msg, " ", var

def processDefines(node):
    global stmtDefine
    
    for defineNode in node.childNodes:
        if not defineNode.nodeType == xml.dom.Node.ELEMENT_NODE:
            continue
        if defineNode.localName == "define":
            stmtDefine += "\n#define " + defineNode.getAttribute("name") + " " + defineNode.getAttribute("value")

def processElement(node):
    global stmtSetup
    debug(">> processElement M1", node.localName)

    if node.nodeType == xml.dom.Node.COMMENT_NODE:
        stmtSetup += "\n/*" + node.nodeValue + "*/"
        return 0
    
    if not node.nodeType == xml.dom.Node.ELEMENT_NODE:
        return 0
    
    debug(">> processElement M2", node.localName)
    if node.prefix == "code" or node.localName == "code":
        for child in node.childNodes:
            if node.localName == "declaration":
                stmtDeclaration += "\n" + child.nodeValue
            elif node.localName == "initialization":
                stmtInitialization += "\n" + child.nodeValue
            else:
                stmtSetup += "\n" + child.nodeValue
    elif node.prefix == "item":
        if node.localName == "layout":
            processLayout(node)
        elif node.localName == "widget":
            processWidget(node)
        

def processLayout(node):
    global stmtDeclaration
    global stmtInitialization
    global stmtSetup
    debug(">> processLayout", node.getAttribute("type"))

    lmgr = LayoutHandlerManager.handler(node.getAttribute("type")) # make this check if layout is supported
    
    lmgr.setNode(node)
    
    stmtDeclaration    += "\n" + lmgr.declaration()
    stmtInitialization += "\n" + lmgr.initialization()
    stmtSetup          += "\n" + lmgr.setup()
    
    debug("", stmtSetup)

    for child in node.childNodes:
        processElement(child)
        
# Main program: ##################################################################################

debug("Program ", sys.argv[0])
debug("is parsing ", sys.argv[1])

doc = xml.dom.minidom.parse(sys.argv[1])

if not doc.documentElement.localName == "pui":
    exit

for node in doc.documentElement.childNodes:
    if not node.nodeType == xml.dom.Node.ELEMENT_NODE:
        continue
    elif node.localName == "defines":
        processDefines(node)
    elif node.localName == "ui":
        for child in node.childNodes:
            processElement(child)

print "/* Defines */", stmtDefine
print "/* Declarations */", stmtDeclaration
print "/* Initialization */", stmtInitialization
print "/* Setup */", stmtSetup