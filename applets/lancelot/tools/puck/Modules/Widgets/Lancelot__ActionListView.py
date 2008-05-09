from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__Widget

class Lancelot__ActionListViewHandler(Lancelot__Widget.Lancelot__WidgetHandler):
    def name(self):
        return "Lancelot::ActionListView"

    def include(self):
        includes = "lancelot/widgets/ActionListView.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__Widget.Lancelot__WidgetHandler.setup(self)

        

        return setup;

WidgetHandlerManager.addHandler(Lancelot__ActionListViewHandler())


