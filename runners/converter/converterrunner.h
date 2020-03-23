/*
 * Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
 * Copyright (C) 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
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

#ifndef CONVERTERRUNNER_H
#define CONVERTERRUNNER_H

#include <KRunner/AbstractRunner>
#include <QRegularExpression>
#include <QLocale>
#include <QAction>
#include <KUnitConversion/Converter>
#include <KUnitConversion/UnitCategory>


/**
 * This class converts values to different units.
 */
class ConverterRunner: public Plasma::AbstractRunner
{
Q_OBJECT

public:
    ConverterRunner(QObject *parent, const QVariantList &args);
    void init() override;
    void insertCompatibleUnits();
    ~ConverterRunner() override;

    void match(Plasma::RunnerContext &context) override;
    QList<QAction *> actionsForMatch(const Plasma::QueryMatch &match) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

private:
    KUnitConversion::Converter converter;
    const QLocale locale;
    QRegularExpression valueRegex;
    QRegularExpression unitSeperatorRegex;
    /** To convert currency symbols back to ISO string and handle case sensitive units */
    QMap<QString, QString> compatibleUnits;

    QList<QAction *> actionList;
    QLatin1String copyActionId = QLatin1String("copy");
    QLatin1String copyUnitActionId = QLatin1String("copy-unit");

    QPair<bool, double> stringToDouble(const QStringRef &value);
    QPair<bool, double> getValidatedNumberValue(const QString &value);
    QList<KUnitConversion::Unit> createResultUnits(QString &outputUnitString,
                                                   const KUnitConversion::UnitCategory &category);
};

#endif
