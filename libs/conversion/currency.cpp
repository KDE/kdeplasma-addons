/*
 * Copyright (C) 2009 Petri Damstén <damu@iki.fi>
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

#include "currency.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDomDocument>
#include <QMutex>
#include <KLocale>
#include <KProcess>
#include <KStandardDirs>
#include <KIO/Job>
#include <KIO/NetAccess>
#include <Solid/Networking>

static const char* URL = "http://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml";

Currency::Currency(QObject* parent)
: SimpleUnit(parent)
{
    setObjectName("currency");
    m_cache = KStandardDirs::locateLocal("data", "libconversion/currency.xml");

    m_default                     = QString::fromUtf8("€");
    m_units[i18n("EUR")]          = m_default;
    m_units[i18n("euro")]         = m_default;
    m_units[m_default]            = 1.0;

    m_units[i18n("usa")]          = "$";
    m_units[i18n("dollar")]       = "$";
    m_units["USD"]                = "$";
    m_units["$"]                  = 1E99;
    m_units[i18n("japan")]        = "\xa5";
    m_units[i18n("yen")]          = "\xa5";
    m_units["JPY"]                = "\xa5";
    m_units["\xa5"]               = 1E99;
    m_units[i18n("bulgaria")]     = "BGN";
    m_units[i18n("lev")]          = "BGN";
    m_units["BGN"]                = 1E99;
    m_units[i18n("czech")]        = "CZK";
    m_units[i18n("koruna")]       = "CZK";
    m_units["CZK"]                = 1E99;
    m_units[i18n("denmark")]      = "DKK";
    m_units[i18n("danish krone")] = "DKK";
    m_units["DKK"]                = 1E99;
    m_units[i18n("estonia")]      = "EEK";
    m_units[i18n("kroon")]        = "EEK";
    m_units["EEK"]                = 1E99;
    m_units[i18n("UK")]           = "\xa3";
    m_units[i18n("pound")]        = "\xa3";
    m_units[i18n("sterling")]     = "\xa3";
    m_units["GBP"]                = "\xa3";
    m_units["\xa3"]               = 1E99;
    m_units[i18n("hungary")]      = "HUF";
    m_units[i18n("forint")]       = "HUF";
    m_units["HUF"]                = 1E99;
    m_units[i18n("lithuania")]    = "LTL";
    m_units[i18n("litas")]        = "LTL";
    m_units["LTL"]                = 1E99;
    m_units[i18n("latvia")]       = "LVL";
    m_units[i18n("lats")]         = "LVL";
    m_units["LVL"]                = 1E99;
    m_units[i18n("poland")]       = "PLN";
    m_units[i18n("zloty")]        = "PLN";
    m_units["PLN"]                = 1E99;
    m_units[i18n("romania")]      = "RON";
    m_units[i18n("leu")]          = "RON";
    m_units["RON"]                = 1E99;
    m_units[i18n("sweden")]       = "SEK";
    m_units[i18n("krona")]        = "SEK";
    m_units["SEK"]                = 1E99;
    m_units[i18n("switzerland")]  = "CHF";
    m_units[i18n("franc")]        = "CHF";
    m_units["CHF"]                = 1E99;
    m_units[i18n("Norway")]       = "NOK";
    m_units[i18n("norwegian krone")] = "NOK";
    m_units["NOK"]                = 1E99;
    m_units[i18n("croatia")]      = "HRK";
    m_units[i18n("kuna")]         = "HRK";
    m_units["HRK"]                = 1E99;
    m_units[i18n("russia")]       = "RUB";
    m_units[i18n("rouble")]       = "RUB";
    m_units["RUB"]                = 1E99;
    m_units[i18n("turkey")]       = "TRY";
    m_units[i18n("lira")]         = "TRY";
    m_units["TRY"]                = 1E99;
    m_units[i18n("australia")]    = "AUD";
    m_units[i18n("australian dollar")] = "AUD";
    m_units["AUD"]                = 1E99;
    m_units[i18n("brasilia")]     = "BRL";
    m_units[i18n("real")]         = "BRL";
    m_units["BRL"]                = 1E99;
    m_units[i18n("canada")]       = "CAD";
    m_units[i18n("canadian dollar")] = "CAD";
    m_units["CAD"]                = 1E99;
    m_units[i18n("china")]        = "CNY";
    m_units[i18n("yuan")]         = "CNT";
    m_units["CNY"]                = 1E99;
    m_units[i18n("hong kong")]    = "HKD";
    m_units[i18n("hong kong dollar")] = "HKD";
    m_units["HKD"]                = 1E99;
    m_units[i18n("indonesia")]    = "IDR";
    m_units[i18n("rupiah")]       = "IDR";
    m_units["IDR"]                = 1E99;
    m_units[i18n("india")]        = "INR";
    m_units[i18n("rupee")]        = "INR";
    m_units["INR"]                = 1E99;
    m_units[i18n("south korea")]  = "KRW";
    m_units[i18n("won")]          = "KRW";
    m_units["KRW"]                = 1E99;
    m_units[i18n("mexico")]       = "MXN";
    m_units[i18n("mexican peso")] = "MXN";
    m_units["MXN"]                = 1E99;
    m_units[i18n("malaysia")]     = "MYR";
    m_units[i18n("ringgit")]      = "MYR";
    m_units["MYR"]                = 1E99;
    m_units[i18n("new zealand")]  = "NZD";
    m_units[i18n("new zealand dollar")] = "NZD";
    m_units["NZD"]                = 1E99;
    m_units[i18n("philippines")]  = "PHP";
    m_units[i18n("philippine peso")] = "PHP";
    m_units["PHP"]                = 1E99;
    m_units[i18n("singapore")]    = "SGD";
    m_units[i18n("singapore dollar")] = "SGD";
    m_units["SGD"]                = 1E99;
    m_units[i18n("thailand")]     = "THB";
    m_units[i18n("baht")]         = "THB";
    m_units["THB"]                = 1E99;
    m_units[i18n("south africa")] = "ZAR";
    m_units[i18n("rand")]         = "ZAR";
    m_units["ZAR"]                = 1E99;

    m_update = true;
}

QString Currency::name() const
{
    return i18n("Currency");
}

Conversion::Value Currency::convert(const Conversion::Value& value, const QString& to)
{
    QFileInfo info(m_cache);

    static QMutex mutex;

    mutex.lock();
    if (!info.exists() || info.lastModified().secsTo(QDateTime::currentDateTime()) > 86400) {
        Solid::Networking::Status status = Solid::Networking::status();
        if (status == Solid::Networking::Connected || status == Solid::Networking::Unknown ) {
            kDebug() << "Getting currency info from net:" << URL;
            // TODO: This crashes in runner. Threading issues??
            /*
            KIO::Job* job = KIO::file_copy(KUrl(URL), KUrl(m_cache), -1,
                                           KIO::Overwrite | KIO::HideProgressInfo);
            job->setUiDelegate(0);
            if (KIO::NetAccess::synchronousRun(job, 0)) {
                m_update = true;
            }
            */
            if (KProcess::execute(QStringList() << "kioclient" << "copy" << URL << m_cache) == 0) {
                m_update = true;
            }
        }
    }
    mutex.unlock();

    if (m_update) {
        QFile file(m_cache);
        if (file.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            if (doc.setContent(&file, false) ) {
                QDomNodeList list = doc.elementsByTagName("Cube");
                for (int i = 0; i < list.count(); ++i) {
                    const QDomElement e = list.item(i).toElement();
                    if (e.hasAttribute("currency")) {
                        QString cur = e.attribute("currency");
                        if (m_units[cur].userType() == QVariant::String) {
                            cur = m_units[cur].toString();
                        }
                        m_units[cur] = 1.0 / e.attribute("rate").toDouble();
                    }
                }
                m_update = false;
            }
        }
    }
    Conversion::Value v = SimpleUnit::convert(value, to);
    v.setDescription("From ECB|http://www.ecb.int/stats/exchange/eurofxref/html/index.en.html");
    return v;
}
