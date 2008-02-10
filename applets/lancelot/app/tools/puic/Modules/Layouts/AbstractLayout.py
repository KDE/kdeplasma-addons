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
    
    def declaration(self):
        return "/* " + self.name() + " declaration - not implemented */"
    
    def initialization(self):
        return "/* " + self.name() + " initialization - not implemented */"
    
    def setup(self):
        return "/* " + self.name() + " setup - not implemented */"

