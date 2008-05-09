from .. import WidgetHandlerManager
from .. import AbstractItem
from . import QGraphicsLayoutItem

class QGraphicsLayoutHandler(QGraphicsLayoutItem.QGraphicsLayoutItemHandler):
    def name(self):
        return "QGraphicsLayout"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = QGraphicsLayoutItem.QGraphicsLayoutItemHandler.setup(self)

        

        return setup;

WidgetHandlerManager.addHandler(QGraphicsLayoutHandler())


