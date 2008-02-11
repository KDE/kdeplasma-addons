from .. import WidgetHandlerManager

class AbstractWidgetHandler:
    
    def __init__(self):
        self.__node = 0
        self.__root = "root";
        
    def name(self):
        return "AbstractWidget"
    
    def setNode(self, node):
        self.__node = node
        
    def node(self):
        return self.__node
    
    def include(self):
        return "/* " + self.name() + " declaration - not implemented */"
    
    def declaration(self):
        return self.name() + " * " + self.node().getAttribute("name") + ";"
    
    def initialization(self):
        return WidgetHandlerManager.root() + "->addItem(" + self._construction() + ");"
    
    def _construction(self):
        return self.node().getAttribute("name") + " = new " + self.name() + "()"
    
    def setup(self):
        setup = ""

        if self.node().hasAttribute("minimumSize"):
            setup += self.node().getAttribute("name") + "->setMinimumSize(" + self.node().getAttribute("minimumSize") + ");\n";
        if self.node().hasAttribute("maximumSize"):
            setup += self.node().getAttribute("name") + "->setMaximumSize(" + self.node().getAttribute("maximumSize") + ");\n";
        if self.node().hasAttribute("opacity"):
            setup += self.node().getAttribute("name") + "->setOpacity(" + self.node().getAttribute("opacity") + ");\n";
        return setup
