#include "Layout.h"
#include <QFile>
#include <QXmlStreamReader>

Layout::Layout(QString path)
{
    m_path = path;

    QFile* file = new QFile(path);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader* xmlReader = new QXmlStreamReader(file);

    // reading in header information
    if (xmlReader->readNextStartElement()) {
        if (xmlReader->name() == "keyboard"){
            m_name = xmlReader->attributes().value("title").toString();
        }
    }

    delete file;
    delete xmlReader;
}

QString Layout::name() {
    return m_name;
}


QString Layout::path() {
    return m_path;
}
