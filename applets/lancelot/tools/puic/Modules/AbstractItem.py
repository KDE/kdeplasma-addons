from . import debug

class AbstractItemHandler:
    
    def __init__(self):
        self.__node = 0
        
    def name(self):
        return "AbstractItem"
    
    def namespace(self):
        return self.name().replace(":", "_")
    
    def setNode(self, node):
        self.__node = node
        
    def node(self):
        return self.__node
    
    def include(self):
        return "/* " + self.name() + " declaration - not implemented */"
    
    def declaration(self):
        return self.name() + " * " + self.attribute("name") + ";"
    
    def initialization(self):
        return self.attribute("name") + " = new " + self.name() + "();"
    
    def _construction(self):
        return self.attribute("name") + " = new " + self.name() + "()"
    
    def setup(self):
        setup = ""

        if self.hasAttribute("minimumSize"):
            setup += attribute("name") + "->setMinimumSize(" + self.attribute("minimumSize") + ");\n";
            
        if self.hasAttribute("maximumSize"):
            setup += attribute("name") + "->setMaximumSize(" + self.attribute("maximumSize") + ");\n";
            
        if self.hasAttribute("opacity"):
            setup += self.attribute("name") + "->setOpacity(" + self.attribute("opacity") + ");\n";
            
        return setup

    def hasAttribute(self, attrName):
        debug.message(self.name(), " wants attribute " + self.namespace() + ":" + attrName)
        return \
            self.node().hasAttribute(self.name() + ":" + attrName) or \
            self.node().hasAttribute(attrName)

    def attribute(self, attrName):
        debug.message(self.name(), " retrieves attribute " + self.namespace() + ":" + attrName)
        
        result = None
        
        if self.node().hasAttribute(self.name() + ":" + attrName):
            result = self.node().getAttribute(self.name() + ":" + attrName)
            
        elif self.node().hasAttribute(attrName):
            result = self.node().getAttribute(attrName)
            
        if not result == None:
            debug.message(" return ", result)
        else:
            debug.message(" return ", " NONE ")

        return result
