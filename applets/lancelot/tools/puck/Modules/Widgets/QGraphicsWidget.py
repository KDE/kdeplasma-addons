## PUCK Generator: disable
## This file is modified after running generator!

from .. import WidgetHandlerManager
from .. import AbstractItem
from . import QGraphicsLayoutItem

class QGraphicsWidgetHandler(QGraphicsLayoutItem.QGraphicsLayoutItemHandler):
    def name(self):
        return "QGraphicsWidget"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode

    ## Changed area begin: ##
    def initialization(self):
        init = AbstractItem.AbstractItemHandler.initialization(self)
        init += self.attribute("name") + "->setParentItem(" + WidgetHandlerManager.root() + ");"
        return init
    ## Changed area end. ##

    def setup(self):
        setup = QGraphicsLayoutItem.QGraphicsLayoutItemHandler.setup(self)


        if self.hasAttribute('palette'):
            setup += self.attribute('name') \
                  + '->setPalette(QPalette(' + self.attribute('palette') + '));'

        if self.hasAttribute('font'):
            setup += self.attribute('name') \
                  + '->setFont(QFont(' + self.attribute('font') + '));'

        if self.hasAttribute('size'):
            setup += self.attribute('name') \
                  + '->resize(QSizeF(' + self.attribute('size') + '));'

        if self.hasAttribute('focusPolicy'):
            setup += self.attribute('name') \
                  + '->setFocusPolicy(Qt::' + self.attribute('focusPolicy') + ');'

        if self.hasAttribute('enabled'):
            setup += self.attribute('name') \
                  + '->setEnabled(' + self.attribute('enabled') + ');'

        if self.hasAttribute('visible'):
            setup += self.attribute('name') \
                  + '->setVisible(' + self.attribute('visible') + ');'

        if self.hasAttribute('windowFlags'):
            setup += self.attribute('name') \
                  + '->setWindowFlags(Qt::' + self.attribute('windowFlags') + ');'

        if self.hasAttribute('windowTitle'):
            setup += self.attribute('name') \
                  + '->setWindowTitle("' + self.attribute('windowTitle') + '");'


        return setup;

WidgetHandlerManager.addHandler(QGraphicsWidgetHandler())


