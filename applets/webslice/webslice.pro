######################################################################
# This will only build the slicewidget and the graphicswidget
#
# You most likely want to use the cmake-based build.
######################################################################

TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += /home/rich/src/project-silk/webslice/. .
QT += webkit

# Input
HEADERS += kwebslice.h kgraphicswebslice.h
SOURCES += main.cpp kwebslice.cpp kgraphicswebslice.cpp
