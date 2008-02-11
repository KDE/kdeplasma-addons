
__handlers = {};
__root = "root"

def handlers():
    return __handlers

def handler(widgetName):
    if not __handlers.has_key(widgetName):
        print "Error: Widget has no handler " + widgetName
    return __handlers[widgetName]

def addHandler(handler):
    __handlers[handler.name()] = handler

def setRoot(root):
    __root = root
    
def root():
    return __root
