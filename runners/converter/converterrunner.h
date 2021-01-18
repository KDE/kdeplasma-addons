/*
 * SPDX-FileCopyrightText: 2007, 2008 Petri Damstén <damu@iki.fi>
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CONVERTERRUNNER_H
#define CONVERTERRUNNER_H

#include <KRunner/AbstractRunner>
#include <KUnitConversion/Converter>
#include <KUnitConversion/UnitCategory>
#include <QAction>
#include <QLocale>
#include <QRegularExpression>

/**
 * This class converts values to different units.
 */
class ConverterRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    ConverterRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    void init() override;
    void insertCompatibleUnits();
    ~ConverterRunner() override;

    void match(Plasma::RunnerContext &context) override;
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
    QList<KUnitConversion::Unit> createResultUnits(QString &outputUnitString, const KUnitConversion::UnitCategory &category);
};

#endif
