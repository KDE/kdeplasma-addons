
__handlers = {};

def handlers():
    return __handlers

def handler(layoutName):
    if not __handlers.has_key(layoutName):
        print "Error: Layout has no handler " + layoutName
    return __handlers[layoutName]

def addHandler(handler):
    __handlers[handler.name()] = handler