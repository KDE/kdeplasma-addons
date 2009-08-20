/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

#include "currency.h"
#include "conversioni18ncatalog.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDomDocument>
#include <QMutex>
#include <KLocale>
#include <KProcess>
#include <KDebug>
#include <KStandardDirs>
#include <Solid/Networking>

static const char* URL = "http://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml";

Currency::Currency(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
    
    setObjectName("currency");
    setName(i18n("Currency"));
    setDescription("From ECB");
    setUrl(KUrl("http://www.ecb.int/stats/exchange/eurofxref/html/index.en.html"));

    setDefaultUnit(U(i18n("euro"), i18n("euros"), "EUR", 1.0, << QString::fromUtf8("€")));

    // Static rates
    U(i18n("schilling"), i18n("schillings"), "ATS", 1.0 / 13.7603, << i18n("austria"));
    U(i18n("franc"), i18n("francs"), "BEF", 1.0 / 40.3399, << i18n("belgium"));
    U(i18n("guilder"), i18n("guilders"), "NLG", 1.0 / 2.20371, << i18n("netherlands"));
    U(i18n("markka"), i18n("markkas"), "FIM", 1.0 / 5.94573, << i18n("finland") << "mk");
    U(i18n("franc"), i18n("francs"), "FRF", 1.0 / 6.55957, << i18n("france"));
    U(i18n("mark"), i18n("marks"), "DEM", 1.0 / 1.95583, << i18n("germany"));
    U(i18n("irish pound"), i18n("irish pounds"), "IEP", 1.0 / 0.787564, << i18n("ireland"));
    U(i18n("lira"), i18n("liras"), "ITL", 1.0 / 1936.27, << i18n("italy"));
    U(i18n("franc"), i18n("francs"), "LUF", 1.0 / 40.3399, << i18n("luxembourg"));
    U(i18n("escudo"), i18n("escudos"), "PTE", 1.0 / 200.482, << i18n("portugal"));
    U(i18n("peseta"), i18n("pesetas"), "ESP", 1.0 / 166.386, << i18n("spain"));
    U(i18n("drachma"), i18n("drachmas"), "GRD", 1.0 / 340.750, << i18n("greece"));
    U(i18n("tolar"), i18n("tolars"), "SIT", 1.0 / 239.640, << i18n("slovenia"));
    U(i18n("cypriot pound"), i18n("cypriot pounds"), "CYP", 1.0 / 0.585274, << i18n("cyprus"));
    U(i18n("maltese lira"), i18n("maltese liras"), "MTL", 1.0 / 0.429300, << i18n("malta"));
    U(i18n("koruna"), i18n("korunas"), "SKK", 1.0 / 30.1260, << i18n("slovakia"));

    // From ECB
    U(i18n("dollar"), i18n("dollars"), "USD", 1E99, << i18n("usa") << "$");
    U(i18n("yen"), i18n("yens"), "JPY", 1E99, << i18n("japan") << "\xa5");
    U(i18n("lev"), i18n("levs"), "BGN", 1E99, << i18n("bulgaria"));
    U(i18n("koruna"), i18n("korunas"), "CZK", 1E99, << i18n("czech"));
    U(i18n("danish krone"), i18n("danish krones"), "DKK", 1E99, << i18n("denmark"));
    U(i18n("kroon"), i18n("kroons"), "EEK", 1E99, << i18n("estonia"));
    U(i18n("pound"), i18n("pounds"), "GBP", 1E99, << "\xa3" << i18n("UK") << i18n("sterling") << i18n("sterlings"));
    U(i18n("forint"), i18n("forints"), "HUF", 1E99, << i18n("hungary"));
    U(i18n("litas"), i18n("litass"), "LTL", 1E99, << i18n("lithuania"));
    U(i18n("lats"), i18n("latss"), "LVL", 1E99, << i18n("latvia"));
    U(i18n("zloty"), i18n("zlotys"), "PLN", 1E99, << i18n("poland"));
    U(i18n("leu"), i18n("leus"), "RON", 1E99, << i18n("romania"));
    U(i18n("krona"), i18n("kronas"), "SEK", 1E99, << i18n("sweden"));
    U(i18n("franc"), i18n("francs"), "CHF", 1E99, << i18n("switzerland"));
    U(i18n("norwegian krone"), i18n("norwegian krones"), "NOK", 1E99, << i18n("norway"));
    U(i18n("kuna"), i18n("kunas"), "HRK", 1E99, << i18n("croatia"));
    U(i18n("rouble"), i18n("roubles"), "RUB", 1E99, << i18n("russia"));
    U(i18n("lira"), i18n("liras"), "TRY", 1E99, << i18n("turkey"));
    U(i18n("australian dollar"), i18n("australian dollars"), "AUD", 1E99, << i18n("australia"));
    U(i18n("real"), i18n("reals"), "BRL", 1E99, << i18n("brasilia"));
    U(i18n("canadian dollar"), i18n("canadian dollars"), "CAD", 1E99, << i18n("canada"));
    U(i18n("yuan"), i18n("yuans"), "CNY", 1E99, << i18n("china"));
    U(i18n("hong kong dollar"), i18n("hong kong dollars"), "HKD", 1E99, << i18n("hong kong"));
    U(i18n("rupiah"), i18n("rupiahs"), "IDR", 1E99, << i18n("indonesia"));
    U(i18n("rupee"), i18n("rupees"), "INR", 1E99, << i18n("india"));
    U(i18n("won"), i18n("wons"), "KRW", 1E99, << i18n("south korea"));
    U(i18n("mexican peso"), i18n("mexican pesos"), "MXN", 1E99, << i18n("mexico"));
    U(i18n("ringgit"), i18n("ringgits"), "MYR", 1E99, << i18n("malaysia"));
    U(i18n("new zealand dollar"), i18n("new zealand dollars"), "NZD", 1E99, << i18n("new zealand"));
    U(i18n("philippine peso"), i18n("philippine pesos"), "PHP", 1E99, << i18n("philippines"));
    U(i18n("singapore dollar"), i18n("singapore dollars"), "SGD", 1E99, << i18n("singapore"));
    U(i18n("baht"), i18n("bahts"), "THB", 1E99, << i18n("thailand"));
    U(i18n("rand"), i18n("rands"), "ZAR", 1E99, << i18n("south africa"));

    m_cache = KStandardDirs::locateLocal("data", "libconversion/currency.xml");
    m_update = true;
}

Conversion::Value Currency::convert(const Conversion::Value& value, const Conversion::Unit* to)
{
    static QMutex mutex;

    mutex.lock();
    QFileInfo info(m_cache);
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
            kDebug() << "Removed previous cache:" << QFile::remove(m_cache);
            if (KProcess::execute(QStringList() << "kioclient" << "copy" << "--noninteractive" << URL << m_cache) == 0) {
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
                        Conversion::Unit* u = unit(e.attribute("currency"));
                        if (u) {
                            u->setMultiplier(1.0 / e.attribute("rate").toDouble());
                        }
                    }
                }
                m_update = false;
            }
        }
    }
    Conversion::Value v = Conversion::UnitCategory::convert(value, to);
    return v;
}
