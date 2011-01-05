/*
*   Copyright 2009 Matteo Agostinelli <agostinelli@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef QALCULATEENGINE_H
#define QALCULATEENGINE_H

#include <QObject>

class QalculateSettings;

class KJob;

class QalculateEngine : public QObject
{
    Q_OBJECT
public:
    explicit QalculateEngine(QalculateSettings* settings, QObject* parent = 0);

    QString lastResult() const {
        return m_lastResult;
    }

public slots:
    void evaluate(const QString& expression);
    void updateExchangeRates();

    void copyToClipboard(bool flag = true);
    
protected slots:
    void updateResult(KJob*);

signals:
    void resultReady(const QString&);
    void formattedResultReady(const QString&);

private:
    QalculateSettings* m_settings;

    QString m_lastResult;
    bool m_currenciesLoaded;
};

#endif // QALCULATEENGINE_H
