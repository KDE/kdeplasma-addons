from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Lancelot__Panel

class Lancelot__ResizeBordersPanelHandler(Lancelot__Panel.Lancelot__PanelHandler):
    def name(self):
        return "Lancelot::ResizeBordersPanel"

    def include(self):
        includes = "lancelot/widgets/ResizeBordersPanel.h".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Lancelot__Panel.Lancelot__PanelHandler.setup(self)



        return setup;

WidgetHandlerManager.addHandler(Lancelot__ResizeBordersPanelHandler())


