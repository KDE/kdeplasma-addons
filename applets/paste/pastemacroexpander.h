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

#ifndef PASTEMACROEXPANDER_HEADER
#define PASTEMACROEXPANDER_HEADER

#include <KWordMacroExpander>
#include <QStringList>
#include <QVariantList>
#include <QHash>

class MacroParam
{
public:
    enum ParamType {
        String = 0,
        Int,
        Boolean,
        Url
    };

    explicit MacroParam(const QString& n = QString(), ParamType t = String) : name(n), type(t) {};
    QString name;
    ParamType type;
};

Q_DECLARE_METATYPE(MacroParam)

class PasteMacroExpander : public QObject, public KWordMacroExpander
{
    Q_OBJECT
public:
    static PasteMacroExpander& instance();

    QMap<QString, QVariantList> macros();

protected:
    PasteMacroExpander(QWidget* parent = 0);
    friend class PasteMacroExpanderSingleton;

    virtual bool expandMacro(const QString &str, QStringList &ret);

protected slots:
    QString exec(const QString& args);
    QString date(const QString& args);
    QString time(const QString& args);
    QString file(const QString& args);
    QString password(const QString& args);

private:
    QMap<QString, QVariantList> m_macros;
};

#endif // PASTEMACROEXPANDER_HEADER
