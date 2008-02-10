from .. import LayoutHandlerManager

class AbstractLayoutHandler:
    
    def __init__(self):
        self.__node = 0
        
    def name(self):
        return "AbstractLayout"
    
    def setNode(self, node):
        self.__node = node
        
    def node(self):
        return self.__node
    
    def include(self):
        return "/* " + self.name() + " declaration - not implemented */"
    
    def declaration(self):
        return self.name() + " * " + self.node().getAttribute("name") + ";"
    
    def initialization(self):
        return self.node().getAttribute("name") + " = new " + self.name() + "();"
    
    def setup(self):
        setup = ""

        if self.node().hasAttribute("margin"):
            setup += self.node().getAttribute("name") + "->setMargin(" + self.node().getAttribute("margin") + ");\n";
        if self.node().hasAttribute("spacing"):
            setup += self.node().getAttribute("name") + "->setSpacing(" + self.node().getAttribute("spacing") + ");\n";
        return setup
