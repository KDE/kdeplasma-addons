
__handlers = {};
__roots = []

def handlers():
    return __handlers

def handler(widgetName):
    if not __handlers.has_key(widgetName):
        print "WARNING: Widget has no handler ", widgetName, " USING DEFAULT HANDLER"
        return __handlers["Abstract"]
    return __handlers[widgetName]

def addHandler(handler):
    __handlers[handler.name()] = handler

def pushRoot(root):
    __roots.append(root)

def popRoot():
    __roots.pop()

def root():
    return __roots[-1]
