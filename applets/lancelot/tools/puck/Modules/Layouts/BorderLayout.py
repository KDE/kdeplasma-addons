from .. import LayoutHandlerManager
from .  import AbstractLayout

import xml

class BorderLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return "Plasma::BorderLayout"

    def include(self):
        return "#include <plasma/layouts/borderlayout.h>"

    def setup(self):
        setup = AbstractLayout.AbstractLayoutHandler.setup(self)

        if self.hasAttribute("leftSize"):
            setup += self.attribute("name") + \
                     "->setSize(" + self.attribute("leftSize") + ", Plasma::LeftPositioned);\n";
        if self.hasAttribute("rightSize"):
            setup += self.attribute("name") + \
                     "->setSize(" + self.attribute("rightSize") + ", Plasma::RightPositioned);\n";
        if self.hasAttribute("topSize"):
            setup += self.attribute("name") + \
                     "->setSize(" + self.attribute("topSize") + ", Plasma::TopPositioned);\n";
        if self.hasAttribute("bottomSize"):
            setup += self.attribute("name") + \
                     "->setSize(" + self.attribute("bottomSize") + ", Plasma::BottomPositioned);\n";

        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == "item":
                continue

            setup += self.attribute("name") + "->addItem(" + child.getAttribute("name") + "," \
                  + " Plasma::" + child.getAttribute(self.namespace() + ":position") + "Positioned);\n"
        return setup

LayoutHandlerManager.addHandler(BorderLayoutHandler())
