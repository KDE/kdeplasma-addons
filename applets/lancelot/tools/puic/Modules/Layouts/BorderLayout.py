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
        
        if self.node().hasAttribute("BorderLayout:leftSize"):
            setup += self.node().getAttribute("name") + \
                     "->setSize(" + self.node().getAttribute("BorderLayout:leftSize") + ", Plasma::LeftPositioned);\n";
        if self.node().hasAttribute("BorderLayout:rightSize"):
            setup += self.node().getAttribute("name") + \
                     "->setSize(" + self.node().getAttribute("BorderLayout:rightSize") + ", Plasma::RightPositioned);\n";
        if self.node().hasAttribute("BorderLayout:topSize"):
            setup += self.node().getAttribute("name") + \
                     "->setSize(" + self.node().getAttribute("BorderLayout:topSize") + ", Plasma::TopPositioned);\n";
        if self.node().hasAttribute("BorderLayout:bottomSize"):
            setup += self.node().getAttribute("name") + \
                     "->setSize(" + self.node().getAttribute("BorderLayout:bottomSize") + ", Plasma::BottomPositioned);\n";

        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == "item":
                continue

            setup += self.node().getAttribute("name") + "->addItem(" + child.getAttribute("name") + "," \
                  + " Plasma::" + child.getAttribute("BorderLayout:position") + "Positioned);\n"
        return setup

LayoutHandlerManager.addHandler(BorderLayoutHandler())
