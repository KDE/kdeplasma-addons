from .. import WidgetHandlerManager
from .. import AbstractItem
from . import QGraphicsWidget

class QGraphicsProxyWidgetHandler(QGraphicsWidget.QGraphicsWidgetHandler):
    def name(self):
        return "QGraphicsProxyWidget"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = QGraphicsWidget.QGraphicsWidgetHandler.setup(self)



        return setup;

WidgetHandlerManager.addHandler(QGraphicsProxyWidgetHandler())


