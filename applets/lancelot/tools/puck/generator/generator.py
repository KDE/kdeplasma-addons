#!/usr/bin/env python

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
import os
import re

# global variables
namespace = ""
className = ""
parentClass = ""
properties = []
includes = []

def parseFile(inputFile):
    global namespace
    global className
    global parentClass
    global properties
    global includes

    namespace = ""
    className = ""
    parentClass = ""
    properties = []
    includes = []

    reNamespace = re.compile("namespace\s+([^\s]+)")
    reClass     = re.compile("class.*[\s]+([^\s]+)[\s]*:[\s]*public[\s]*([^\s]+)|class.*[\s]+([^\s]+)[\s]*(:|{|;|$)")
    reProperty  = re.compile("Q_PROPERTY\s*\(\s*([^\s]+)\s+([^\s]+)\s+READ\s+([^\s]+)\s+WRITE\s+([^\s]+)\s*\)")
    reIncludes  = re.compile("L_INCLUDE\s*\(\s*(.+)\s*\)")
    reExtra     = re.compile(".*L_EXTRA\s*\(\s*(.+)\s*\)")

    data = open(inputFile).readlines()

    for line in data:
        line = line.strip()

        match = reNamespace.match(line)
        if match:
            namespace = match.group(1)
            continue

        match = reClass.match(line)
        if match:
            if className != "":
                processClass()

            properties = []
            includes = ""

            print match.groups()
            if match.group(1) != None:
                className = match.group(1)
                parentClass = match.group(2)
            else:
                className = match.group(3)
                parentClass = ""
            print "Found class '" + className + "' inherits '" + parentClass + "'"
            reConstructor = re.compile("^" + className + "\((.*)\);")
            reConstructorBegin = re.compile(className)
            continue

        match = reProperty.match(line)
        if match:
            print "Found property ", match.groups()
            append = list(match.groups())

            match = reExtra.match(line)
            if match:
                append.append(match.group(1).strip())

            properties.append(append)
            # print properties
            continue

        match = reIncludes.match(line)
        if match:
            print "Found includes ", match.group(1)
            includes = match.group(1)
            print includes
    if className != "":
        processClass()

def constructPythonName(fullClass):
    result = fullClass.replace("::", "__")
    return result.strip("_")

def constructCPPName(fullClass):
    return fullClass.strip(":")

def propertyValueExes(type):
    # Qt and Kde types
    if (type == "QString"):
        return "\"${VALUE}\""
    elif (type == "QString i18n"):
        return "i18n(\"${VALUE}\")"
    elif (type == "KIcon"):
        return "KIcon(\"${VALUE}\")"
    elif (type == "QIcon"):
        return "KIcon(\"${VALUE}\")"

    # Lancelot types
    elif (type == "ExtenderPosition"):
        return "Lancelot::${VALUE}Extender"
    elif (type == "ActivationMethod"):
        return "Lancelot::${VALUE}Activate"

    # Heuristics
    elif (type.find("::") != -1):
        position = type.find("::");
        return type[0:position] + "::${VALUE}"
    elif (type.startswith("Q")):
        return type + "(${VALUE})"
    else:
        return "${VALUE}"

def processClass():
    global namespace
    global className
    global parentClass
    global properties
    global includes

    fullClass = namespace + "::" + className
    outputFile = constructPythonName(fullClass) + ".py"
    print "Generating", outputFile

    template = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "template/widget.py"))
    template = template.readlines()
    template = "".join(template)

    if (parentClass != ""):
        parentPythonName = constructPythonName(parentClass)
        pythonImport = "from . import " + parentPythonName;
    else:
        parentPythonName = "AbstractItem"
        pythonImport = ""

    template = template\
            .replace("${NAME}", constructCPPName(fullClass))\
            .replace("${INCLUDES}", includes)\
            .replace("${PYTHON_NAME}", constructPythonName(fullClass))\
            .replace("${PYTHON_IMPORT}", pythonImport)\
            .replace("${PARENT_PYTHON_NAME}", parentPythonName)

    begin = template.find("${PROPERTY_SETUP_BEGIN}") # 23 length
    end   = template.find("${PROPERTY_SETUP_END}")   # 21 length
    propertyTemplate = template[begin + 23:end]
    propertiesCode = ""
    for property in properties:
        exes = ""
        print "Has this no of props: ", len(property);
        if (len(property) == 5):
            exes = propertyValueExes(property[0] + " " + property[4]).split("${VALUE}")
        else:
            exes = propertyValueExes(property[0]).split("${VALUE}")
        print exes
        propertiesCode += propertyTemplate\
                .replace("${PROPERTY_NAME}", property[1])\
                .replace("${PROPERTY_SETTER}", property[3])\
                .replace("${PROPERTY_VALUE_PREFIX}", exes[0])\
                .replace("${PROPERTY_VALUE_SUFIX}", exes[1])

    template = template[0:begin] + propertiesCode + template[end + 21:]
    # print template
    output = open(outputFile, 'w')
    print >> output, template

######## MAIN PROGRAM ###########

inputFile  = sys.argv[1]
# outputFile = sys.argv[2]

print "Parsing: " + inputFile
# print "And creating PUCK module from it: " + outputFile

parseFile(inputFile)

