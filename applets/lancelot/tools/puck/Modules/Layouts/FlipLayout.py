from .. import LayoutHandlerManager
from .  import AbstractLayout

class FlipLayoutHandler(AbstractLayout.AbstractLayoutHandler):
    def name(self):
        return "Lancelot::FlipLayout"

    def include(self):
        h = LayoutHandlerManager.handler(self.attribute("realLayout"))
        h.setNode(self.node())

        return h.include() + "\n#include <lancelot/layouts/FlipLayout.h>"

    def declaration(self):
        return "Lancelot::FlipLayout < " + self.attribute("realLayout") + " > * " + self.attribute("name") + ";"

    def initialization(self):
        return self.attribute("name") + " = new Lancelot::FlipLayout < " + self.attribute("realLayout") + " > ();"

    def setup(self):
        setup = ""
        if self.attribute("flip"):
            setup += self.attribute("name") + "->setFlip(" + self.attribute("flip") + ");\n";

        h = LayoutHandlerManager.handler(self.attribute("realLayout"))
        h.setNode(self.node())

        return setup + h.setup()

LayoutHandlerManager.addHandler(FlipLayoutHandler())
