from .. import WidgetHandlerManager
from .. import AbstractItem


class Lancelot__Private;Handler(AbstractItem.AbstractItemHandler):
    def name(self):
        return "Lancelot::Private;"

    def include(self):
        includes = "".split(" ")
        includesCode = ""
        for include in includes:
            if (include != ""):
                includesCode = "#include<" + include + ">\n"
        return includesCode


    def setup(self):
        setup = AbstractItem.AbstractItemHandler.setup(self)

        

        return setup;


