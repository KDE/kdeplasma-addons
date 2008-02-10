from .. import LayoutHandlerManager
from .  import AbstractLayout

import xml

class NodeLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return "Plasma::NodeLayout"
    
    def declaration(self):
        return "Plasma::NodeLayout * " + self.node().getAttribute("name") + ";"
    
    def initialization(self):
        return self.node().getAttribute("name") + " = new Plasma::NodeLayout();"
    
    def setup(self):
        setup = ""
        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == "item":
                continue
            
            setup += self.node().getAttribute("name") + "->addItem(" + child.getAttribute("name") + ",\n" \
                  + "  Plasma::NodeLayout::NodeCoordinate(" + child.getAttribute("NodeLayout:topLeft") + "),\n" \
                  + "  Plasma::NodeLayout::NodeCoordinate(" + child.getAttribute("NodeLayout:bottomRight") + ")\n" \
                  + ");\n"
        return setup

lh = NodeLayoutHandler()
LayoutHandlerManager.addHandler(lh)
