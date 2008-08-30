/*
 *   Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
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

#include "converterrunner.h"
#include <QApplication>
#include <QClipboard>
#include <KIcon>
#include "length.h"
#include "area.h"
#include "volume.h"
#include "temperature.h"
#include "speed.h"

#define CONVERSION_CHAR '>'

class StringParser
{
public:
    enum GetType
    {
        GetString = 1,
        GetDigit  = 2
    };

    StringParser(const QString &s) : m_index(0), m_s(s) {};
    ~StringParser() {};

    QString get(int type)
    {
        QChar current;
        QString result;

        passWhiteSpace();
        while (true) {
            current = next();
            if (current.isNull()) {
                break;
            }
            if (current.isSpace()) {
                break;
            }
            bool number = isNumber(current);
            if (type == GetDigit && !number) {
                break;
            }
            if (type == GetString && number) {
                break;
            }
            if(current == CONVERSION_CHAR) {
                break;
            }
            ++m_index;
            result += current;
        }
        return result;
    }

    bool isNumber(const QChar &ch)
    {
        if (ch.isNumber()) {
            return true;
        }
        if (QString(".,-+").contains(ch)) {
            return true;
        }
        return false;
    }

    QString rest()
    {
        return m_s.mid(m_index).simplified();
    }

    void pass(const QStringList &strings)
    {
        passWhiteSpace();
        QString temp = m_s.mid(m_index);

        foreach (const QString& s, strings) {
            if (temp.startsWith(s)) {
                m_index += s.length();
                return;
            }
        }
    }

private:
    void passWhiteSpace()
    {
        while (next().isSpace()) {
            ++m_index;
        }
    }

    QChar next()
    {
        if (m_index >= m_s.size()) {
            return QChar::Null;
        }
        return m_s.at(m_index);
    }

    int m_index;
    QString m_s;
};

ConverterRunner::ConverterRunner(QObject* parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName(i18n("Converter"));

    m_units.append(new Length);
    m_units.append(new Area);
    m_units.append(new Volume);
    m_units.append(new Temperature);
    m_units.append(new ::Speed); // There is Plasma::AbstractRunner::Speed
    //can not ignore commands: we have things like m4
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);

}

ConverterRunner::~ConverterRunner()
{
    foreach (Unit *unit, m_units) {
        delete unit;
    }
}

Unit* ConverterRunner::unitType(const QString& type)
{
    foreach (Unit *unit, m_units) {
        if (unit->hasUnit(type)) {
            return unit;
        }
    }
    return 0;
}

void ConverterRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.size() < 2) {
        return;
    }

    StringParser cmd(term);
    QString unit1;
    Unit *converter;
    QString value;
    QString unit2;

    unit1 = cmd.get(StringParser::GetString);
    value = cmd.get(StringParser::GetDigit);
    if (value.isEmpty()) {
        return;
    }
    if (unit1.isEmpty()) {
        unit1 = cmd.get(StringParser::GetString | StringParser::GetDigit);
        if (unit1.isEmpty()) {
            return;
        }
    }
    QStringList separators;
    separators << QString(CONVERSION_CHAR) << i18nc("amount of <unit1> in <unit2>", "in");
    QString s = cmd.get(StringParser::GetString);
    if (!s.isEmpty() && !separators.contains(s)) {
        unit1 += ' ' + s;
    }
    converter = unitType(unit1);
    if (!converter) {
        return;
    }
    cmd.pass(separators);
    unit2 = cmd.rest();
    if (!unit2.isEmpty() && !converter->hasUnit(unit2)) {
        return;
    }
    QVariant data;
    QString result = converter->convert(value, unit1, unit2, &data);
    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::InformationalMatch);
    match.setIcon(KIcon("kruler"));
    match.setText(result);
    match.setData(data);
    context.addMatch(term, match);
}

void ConverterRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    QApplication::clipboard()->setText(match.data().toString());
}

#include "converterrunner.moc"
