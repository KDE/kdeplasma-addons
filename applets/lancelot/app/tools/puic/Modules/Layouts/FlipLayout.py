from .. import LayoutHandlerManager
from .  import AbstractLayout

class FlipLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return "Plasma::FlipLayout"

    def declaration(self):
        return "Plasma::FlipLayout < " + self.node().getAttribute("FlipLayout:realLayout") + " > * " + self.node().getAttribute("name") + ";"
    
    def initialization(self):
        return self.node().getAttribute("name") + " = new Plasma::FlipLayout < " + self.node().getAttribute("FlipLayout:realLayout") + " > ();"
    
    def setup(self):
        h = LayoutHandlerManager.handler(self.node().getAttribute("FlipLayout:realLayout"))
        h.setNode(self.node())
        return h.setup()

LayoutHandlerManager.addHandler(FlipLayoutHandler())
