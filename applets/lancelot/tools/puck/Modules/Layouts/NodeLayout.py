from .. import LayoutHandlerManager
from .  import AbstractLayout

import xml

class NodeLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return "Lancelot::NodeLayout"

    def include(self):
        return "#include <lancelot/layouts/NodeLayout.h>"

    def setup(self):
        setup = AbstractLayout.AbstractLayoutHandler.setup(self)

        for child in self.node().childNodes:
            if not child.nodeType == xml.dom.Node.ELEMENT_NODE or not child.prefix == "item":
                continue

            setup += self.attribute("name") + "->addItem(" + child.getAttribute("name") + ",\n" \
                  + "  Lancelot::NodeLayout::NodeCoordinate(" + child.getAttribute(self.namespace() + ":topLeft") + "),\n" \
                  + "  Lancelot::NodeLayout::NodeCoordinate(" + child.getAttribute(self.namespace() + ":bottomRight") + ")\n" \
                  + ");\n"
        return setup

LayoutHandlerManager.addHandler(NodeLayoutHandler())
