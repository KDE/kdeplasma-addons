from .. import LayoutHandlerManager
from .. import AbstractItem

class AbstractLayoutHandler(AbstractItem.AbstractItemHandler):

    def __init__(self):
        self.__node = 0

    def setup(self):
        setup = ""

        # if self.hasAttribute("margin"):
        #     setup += self.attribute("name") + "->setMargin(" + self.attribute("margin") + ");\n";

        # if self.hasAttribute("spacing"):
        #     setup += self.attribute("name") + "->setSpacing(" + self.attribute("spacing") + ");\n";
        return setup

LayoutHandlerManager.addHandler(AbstractLayoutHandler())
