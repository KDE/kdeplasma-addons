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

#ifndef CONFIGDATA_HEADER
#define CONFIGDATA_HEADER

#include <KConfigGroup>
#include <QKeySequence>

typedef QMap<QString, QStringList> SnippetMap;
typedef QMap<QString, QKeySequence> AppMap;

class ConfigData : public QObject, public KConfigGroup
{
    Q_OBJECT
public:
    enum DataIndex {Icon = 0, Text};

    ConfigData();
    virtual ~ConfigData();

    ConfigData &operator=(const KConfigGroup &rhs);
    void writeEntries();
    void writeEntry(const char *pKey, AppMap value);
    QMap<QString, QKeySequence> readKeySequenceMapEntry(const char *pKey,
            AppMap defaultValue = AppMap());

    void writeToXmlFile(SnippetMap value);
    SnippetMap readFromXmlFile();

public slots:
    void readEntries();

signals:
    void changed(const ConfigData&);

public:
    SnippetMap snippets;
    bool autoPaste;
    QKeySequence pasteKey;
    AppMap specialApps;
    QString xmlFile;
};

#endif // CONFIGDATA_HEADER
