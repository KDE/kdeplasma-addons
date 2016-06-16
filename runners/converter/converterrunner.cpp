/*
 * Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
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
#include <QGuiApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QSet>
#include <QDebug>
#include <KLocalizedString>
#include <KUnitConversion/Converter>
#include <KUnitConversion/UnitCategory>

#define CONVERSION_CHAR QLatin1Char( '>' )

K_EXPORT_PLASMA_RUNNER(converterrunner, ConverterRunner)

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
            if(current == QLatin1Char( CONVERSION_CHAR )) {
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
        if (QString(QLatin1String( ".,-+" )).contains( ch )) {
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
        const QString temp = m_s.mid(m_index);

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
    setObjectName(QLatin1String( "Converter" ));

    m_separators << QString( CONVERSION_CHAR );
    m_separators << i18nc("list of words that can used as amount of 'unit1' [in|to|as] 'unit2'",
                          "in;to;as").split(QLatin1Char( ';' ));

    //can not ignore commands: we have things like m4
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);

    QString description = i18n("Converts the value of :q: when :q: is made up of "
                               "\"value unit [>, to, as, in] unit\". You can use the "
                               "Unit converter applet to find all available units.");
    addSyntax(Plasma::RunnerSyntax(QLatin1String(":q:"), description));
}

ConverterRunner::~ConverterRunner()
{
}

void ConverterRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.size() < 2) {
        return;
    }

    StringParser cmd(term);
    QString unit1;
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

    const QString s = cmd.get(StringParser::GetString);

    if (!s.isEmpty() && !m_separators.contains(s)) {
        unit1 += QLatin1Char( ' ' ) + s;
    }
    if (s.isEmpty() || !m_separators.contains(s)) {
        cmd.pass(m_separators);
    }
    unit2 = cmd.rest();

    KUnitConversion::Converter converter;
    KUnitConversion::UnitCategory category = converter.categoryForUnit(unit1);
    bool found = false;
    if (category.id() == KUnitConversion::InvalidCategory) {
        foreach (category, converter.categories()) {
            foreach (const QString& s, category.allUnits()) {
                if (s.compare(unit1, Qt::CaseInsensitive) == 0) {
                    unit1 = s;
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
        if (!found) {
            return;
        }
    }

    QList<KUnitConversion::Unit> units;

    if (!unit2.isEmpty()) {
        KUnitConversion::Unit u = category.unit(unit2);
        if (!u.isNull() && u.isValid()) {
            units.append(u);
            config().writeEntry(category.name(), u.symbol());
        } else {
            const QStringList unitStrings = category.allUnits();
            QList<KUnitConversion::Unit> matchingUnits;
            foreach (const QString& s, unitStrings) {
                if (s.startsWith(unit2, Qt::CaseInsensitive)) {
                    matchingUnits << category.unit(s);
                }
            }
            units = matchingUnits;
            if (units.count() == 1) {
                config().writeEntry(category.name(), units[0].symbol());
            }
        }
    } else {
        units = category.mostCommonUnits();
        KUnitConversion::Unit u = category.unit(config().readEntry(category.name()));
        if (!u.isNull() && units.indexOf(u) < 0) {
            units << u;
        }
    }

    QList<Plasma::QueryMatch> matches;

    QLocale locale;
    KUnitConversion::Unit u1 = category.unit(unit1);
    foreach (const KUnitConversion::Unit& u, units) {
        if (u1 == u) {
            continue;
        }

        bool ok;
        double numberValue = locale.toDouble(value, &ok);
        if (!ok) {
            numberValue = value.toDouble(&ok);
            if (!ok) {
                continue;
            }
        }
        KUnitConversion::Value v = category.convert(KUnitConversion::Value(numberValue, u1), u);

        if (!v.isValid()) {
            continue;
        }

        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIcon(QIcon::fromTheme(QLatin1String( "edit-copy" )));
        match.setText(QString(QLatin1String( "%1 (%2)" )).arg(v.toString()).arg(u.symbol()));
        match.setData(v.number());
        match.setRelevance(1.0 - std::abs(std::log10(v.number())) / 50.0);
        matches.append(match);
    }

    context.addMatches(matches);
}

void ConverterRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString data = match.data().toString();
    if (data.startsWith(QLatin1String("http://"))) {
        QDesktopServices::openUrl(data);
    } else {
        QGuiApplication::clipboard()->setText(data);
    }
}

#include "converterrunner.moc"
