from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Widget

class Lancelot__CustomListViewHandler(Widget.WidgetHandler):
    def name(self):
        return "Lancelot::CustomListView"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Widget.WidgetHandler.setup(self)

        

        return setup;

WidgetHandlerManager.addHandler(Lancelot__CustomListViewHandler())


