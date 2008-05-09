from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__Widget

class Lancelot__WidgetPositionerHandler(Lancelot__Widget.Lancelot__WidgetHandler):
    def name(self):
        return "Lancelot::WidgetPositioner"

    def include(self):
        includes = "lancelot/widgets/WidgetPositioner.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__Widget.Lancelot__WidgetHandler.setup(self)

        

        return setup;

WidgetHandlerManager.addHandler(Lancelot__WidgetPositionerHandler())


