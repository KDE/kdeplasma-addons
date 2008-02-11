from .. import LayoutHandlerManager
from .  import AbstractLayout

import xml

class NodeLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return "Plasma::NodeLayout"
    
    def include(self):
        return "#include <plasma/layouts/nodelayout.h>"

    def setup(self):
        setup = AbstractLayout.AbstractLayoutHandler.setup(self)
        
        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == "item":
                continue
            
            setup += self.node().getAttribute("name") + "->addItem(" + child.getAttribute("name") + ",\n" \
                  + "  Plasma::NodeLayout::NodeCoordinate(" + child.getAttribute("NodeLayout:topLeft") + "),\n" \
                  + "  Plasma::NodeLayout::NodeCoordinate(" + child.getAttribute("NodeLayout:bottomRight") + ")\n" \
                  + ");\n"
        return setup

LayoutHandlerManager.addHandler(NodeLayoutHandler())
