from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__ScrollPane

class Lancelot__CustomListViewHandler(Lancelot__ScrollPane.Lancelot__ScrollPaneHandler):
    def name(self):
        return "Lancelot::CustomListView"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__ScrollPane.Lancelot__ScrollPaneHandler.setup(self)



        return setup;

WidgetHandlerManager.addHandler(Lancelot__CustomListViewHandler())


