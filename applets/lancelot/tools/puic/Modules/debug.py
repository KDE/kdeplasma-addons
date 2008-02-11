import sys, traceback

def message(msg, var):
    print >> sys.stdout, "-" * 10 + " : ",  msg, " ", var

def trace(msg):
    print msg, ' - Trace ', '-' * 60
    try:
        raise NameError, 'HiThere'
    except:
        traceback.print_exc(None, sys.stdout)
        traceback.print_stack()
    print '- End Trace ', '-' * 60
