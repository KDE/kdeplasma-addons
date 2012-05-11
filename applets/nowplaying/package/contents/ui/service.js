.pragma library

var control = service("mpris2", "@multiplex");

function associateItem(item, name)
{
    control.associateItem(item, name);
}

function callCommand(name)
{
    control.startOperationCall(control.operationDescription(name));
}

function seek(timeMs)
{
    var desc = control.operationDescription('SetPosition');
    desc.microseconds = Math.floor(timeMs * 1000);
    print("Seeking to " + timeMs + "ms");
    control.startOperationCall(desc);
}

// vi:sts=4:sw=4:et
