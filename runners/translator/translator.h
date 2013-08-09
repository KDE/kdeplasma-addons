/******************************************************************************
 *  Copyright (C) 2013 by David Baum <david.baum@naraesk.eu>                  *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <Plasma/AbstractRunner>

/**
 * Translate a word or sentence using google translate service http://translate.google.com/
 */
class Translator : public Plasma::AbstractRunner
{

    Q_OBJECT

public:
    Translator(QObject *parent, const QVariantList &args);
    ~Translator();

    void match(Plasma::RunnerContext &);
    void run(const Plasma::RunnerContext &, const Plasma::QueryMatch &);

Q_SIGNALS:
    void matchMade(Plasma::RunnerContext *);

private:
    bool parseTerm(const QString &, QString &, QPair<QString, QString> &);
    void parseResult(const QString &, Plasma::RunnerContext &, const QString &);
	static const QSet<QString> supportedLanguages;
};

K_EXPORT_PLASMA_RUNNER(translator, Translator)

#endif
