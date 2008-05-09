from .. import LayoutHandlerManager
from .  import AbstractLayout

import xml

class FullBorderLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return "Lancelot::FullBorderLayout"

    def include(self):
        return "#include <lancelot/layouts/FullBorderLayout.h>"

    def setup(self):
        setup = AbstractLayout.AbstractLayoutHandler.setup(self)

        if self.hasAttribute("leftSize"):
            setup += self.attribute("name") + \
                     "->setSize(" + self.attribute("leftSize") + ", Lancelot::FullBorderLayout::LeftBorder);\n";
        if self.hasAttribute("rightSize"):
            setup += self.attribute("name") + \
                     "->setSize(" + self.attribute("rightSize") + ", Lancelot::FullBorderLayout::RightBorder);\n";
        if self.hasAttribute("topSize"):
            setup += self.attribute("name") + \
                     "->setSize(" + self.attribute("topSize") + ", Lancelot::FullBorderLayout::TopBorder);\n";
        if self.hasAttribute("bottomSize"):
            setup += self.attribute("name") + \
                    "->setSize(" + self.attribute("bottomSize") + ", Lancelot::FullBorderLayout::BottomBorder);\n";

        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == "item":
                continue

            setup += self.attribute("name") + "->addItem(" + child.getAttribute("name") + "," \
                    + " Lancelot::FullBorderLayout::" + child.getAttribute(self.namespace() + ":position") + ");\n"
        return setup

LayoutHandlerManager.addHandler(FullBorderLayoutHandler())
