from .. import WidgetHandlerManager
from .. import AbstractItem
from . import PopupList

class Lancelot__PopupMenuHandler(PopupList.PopupListHandler):
    def name(self):
        return "Lancelot::PopupMenu"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = PopupList.PopupListHandler.setup(self)



        return setup;

WidgetHandlerManager.addHandler(Lancelot__PopupMenuHandler())


