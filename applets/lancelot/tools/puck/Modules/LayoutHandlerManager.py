
__handlers = {};

def handlers():
    return __handlers

def handler(layoutName):
    if not __handlers.has_key(layoutName):
        print "WARNING: Layout has no handler ", layoutName, " USING DEFAULT HANDLER"
        return __handlers["Abstract"]
    return __handlers[layoutName]

def addHandler(handler):
    __handlers[handler.name()] = handler