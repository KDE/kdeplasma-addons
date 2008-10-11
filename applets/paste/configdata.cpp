/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "configdata.h"
#include <KLocale>
#include <KStandardDirs>
#include <KSaveFile>
#include <KDebug>
#include <KDirWatch>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

ConfigData::ConfigData()
{
    xmlFile = KStandardDirs::locateLocal("data", "plasma_applet_paste/snippets.xml");
    KDirWatch::self()->addFile(xmlFile);
    connect(KDirWatch::self(), SIGNAL(dirty(const QString&)), this, SLOT(readEntries()));
    connect(KDirWatch::self(), SIGNAL(created(const QString&)), this, SLOT(readEntries()));
}

ConfigData::~ConfigData()
{
}

ConfigData &ConfigData::operator=(const KConfigGroup &rhs)
{
    KConfigGroup::operator=(rhs);
    readEntries();
    return *this;
}

#define CHECK(TYPE, VAR, READFUNC) { \
TYPE tmp = READFUNC; \
if (VAR != tmp) { \
    VAR = tmp; \
    change = true; \
} }

void ConfigData::readEntries()
{
    AppMap defApps;
    defApps["Konsole"] = QKeySequence::fromString("Ctrl+Shift+V");
    QString defPaste = QKeySequence(QKeySequence::Paste).toString();
    bool change = false;

    CHECK(SnippetMap,   snippets,    readFromXmlFile());
    CHECK(bool,         autoPaste,   readEntry("auto_paste", true));
    CHECK(QKeySequence, pasteKey,    QKeySequence::fromString(readEntry("paste_key", defPaste)));
    CHECK(AppMap,       specialApps, readKeySequenceMapEntry("special_apps", defApps));

    if (change) {
        emit changed(*this);
    }
}

void ConfigData::writeEntries()
{
    writeToXmlFile(snippets);
    KConfigGroup::writeEntry("auto_paste", autoPaste);
    KConfigGroup::writeEntry("paste_key", pasteKey.toString());
    writeEntry("special_apps", specialApps);
}

void ConfigData::writeToXmlFile(SnippetMap value)
{
    // Make XML
    QDomDocument doc("text_snippets");
    QDomElement root = doc.createElement("snippets");

    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
    foreach (const QString &key, value.keys()) {
        QDomElement snippet = doc.createElement("snippet");
        snippet.setAttribute("name", key);
        snippet.setAttribute("icon", value[key][Icon]);
        QDomText txt = doc.createTextNode(value[key][Text]);
        snippet.appendChild(txt);
        root.appendChild(snippet);
    }
    doc.appendChild(root);

    // Save to file
    KSaveFile file(xmlFile);

    if (file.open()) {
        QTextStream stream(&file);
        doc.save(stream, 2);
        stream.flush();
    }
    file.finalize();
}

SnippetMap ConfigData::readFromXmlFile()
{
    // Read file
    SnippetMap result;
    QDomDocument doc("text_snippets");
    QFile file(xmlFile);

    if (file.open(QIODevice::ReadOnly)) {
        if (doc.setContent(&file)) {
            // Parse xml
            QDomElement node = doc.documentElement().firstChildElement();
            while (!node.isNull()) {
                if (node.tagName() == "snippet") {
                    result[node.attribute("name")] = QStringList() <<
                            node.attribute("icon") << node.firstChild().toText().data();
                }
                node = node.nextSiblingElement();
            }
            return result;
        }
        file.close();
    }
    result[i18n("Random Password")] = QStringList() << "object-unlocked" << "%{password(8)}";
    result[i18n("Current Date")] = QStringList() << "office-calendar" << "%{date()}";
    return result;
}

void ConfigData::writeEntry(const char *pKey, AppMap value)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << value;
    KConfigGroup::writeEntry(pKey, ba);
}

AppMap ConfigData::readKeySequenceMapEntry(const char *pKey, AppMap defaultValue)
{
    AppMap result;
    QByteArray ba = readEntry(pKey, QByteArray());
    if (ba.isEmpty()) {
        return defaultValue;
    }
    QDataStream ds(&ba, QIODevice::ReadOnly);
    ds >> result;
    return result;
}

#include "configdata.moc"
