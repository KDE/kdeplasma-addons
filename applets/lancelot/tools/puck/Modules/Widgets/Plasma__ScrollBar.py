from .. import WidgetHandlerManager
from .. import AbstractItem
from . import QGraphicsProxyWidget

class Plasma__ScrollBarHandler(QGraphicsProxyWidget.QGraphicsProxyWidgetHandler):
    def name(self):
        return "Plasma::ScrollBar"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = QGraphicsProxyWidget.QGraphicsProxyWidgetHandler.setup(self)


        if self.hasAttribute('singleStep'):
            setup += self.attribute('name') \
                  + '->setSingleStep(' + self.attribute('singleStep') + ');'

        if self.hasAttribute('pageStep'):
            setup += self.attribute('name') \
                  + '->setPageStep(' + self.attribute('pageStep') + ');'

        if self.hasAttribute('stylesheet'):
            setup += self.attribute('name') \
                  + '->setStyleSheet("' + self.attribute('stylesheet') + '");'


        return setup;

WidgetHandlerManager.addHandler(Plasma__ScrollBarHandler())


