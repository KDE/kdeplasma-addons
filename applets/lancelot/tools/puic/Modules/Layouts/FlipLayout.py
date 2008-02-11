from .. import LayoutHandlerManager
from .  import AbstractLayout

class FlipLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return "Plasma::FlipLayout"

    def include(self):
        return "#include <plasma/layouts/fliplayout.h>"

    def declaration(self):
        return "Plasma::FlipLayout < " + self.node().getAttribute("FlipLayout:realLayout") + " > * " + self.node().getAttribute("name") + ";"
    
    def initialization(self):
        return self.node().getAttribute("name") + " = new Plasma::FlipLayout < " + self.node().getAttribute("FlipLayout:realLayout") + " > ();"
    
    def setup(self):
        setup = ""
        if self.node().hasAttribute("FlipLayout:flip"):
            setup += self.node().getAttribute("name") + "->setFlip(" + self.node().getAttribute("FlipLayout:flip") + ");\n";

        h = LayoutHandlerManager.handler(self.node().getAttribute("FlipLayout:realLayout"))
        h.setNode(self.node())
        
        return setup + h.setup()

LayoutHandlerManager.addHandler(FlipLayoutHandler())
