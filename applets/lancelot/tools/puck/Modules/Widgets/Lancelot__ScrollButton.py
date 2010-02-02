from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__BasicWidget

class Lancelot__ScrollButtonHandler(Lancelot__BasicWidget.Lancelot__BasicWidgetHandler):
    def name(self):
        return "Lancelot::ScrollButton"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__BasicWidget.Lancelot__BasicWidgetHandler.setup(self)



        return setup;

WidgetHandlerManager.addHandler(Lancelot__ScrollButtonHandler())


