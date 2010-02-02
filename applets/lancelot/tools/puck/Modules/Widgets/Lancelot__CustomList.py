from .. import WidgetHandlerManager
from .. import AbstractItem
from . import QGraphicsWidget

class Lancelot__CustomListHandler(QGraphicsWidget.QGraphicsWidgetHandler):
    def name(self):
        return "Lancelot::CustomList"

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

WidgetHandlerManager.addHandler(Lancelot__CustomListHandler())


