from .. import WidgetHandlerManager
from .. import AbstractItem
from . import Plasma__Dialog

class Lancelot__PopupListHandler(Plasma__Dialog.Plasma__DialogHandler):
    def name(self):
        return "Lancelot::PopupList"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode += "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = Plasma__Dialog.Plasma__DialogHandler.setup(self)


        if self.hasAttribute('closeTimeout'):
            setup += self.attribute('name') \
                  + '->setCloseTimeout(' + self.attribute('closeTimeout') + ');'

        if self.hasAttribute('sublevelOpenAction'):
            setup += self.attribute('name') \
                  + '->setSublevelOpenAction(' + self.attribute('sublevelOpenAction') + ');'


        return setup;

WidgetHandlerManager.addHandler(Lancelot__PopupListHandler())


