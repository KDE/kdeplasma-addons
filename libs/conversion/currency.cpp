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
    setObjectName("currency");
    setName(i18n("Currency"));
    setDescription("From ECB");
    setUrl(KUrl("http://www.ecb.int/stats/exchange/eurofxref/html/index.en.html"));

    // Static rates
    setDefaultUnit(U(Currency::Eur, 1,
      i18nc("currency unit symbol", "EUR"),
      i18nc("unit description in lists", "euros"),
      i18nc("unit synonyms for matching user input", "euro;euros;EUR;eur;€"),
      ki18nc("amount in units (real)", "%1 euros"),
      ki18ncp("amount in units (integer)", "%1 euro", "%1 euros")
    ));
    U(Currency::Ats, 0.0726728,
      i18nc("currency unit symbol", "ATS"),
      i18nc("unit description in lists", "schillings"),
      i18nc("unit synonyms for matching user input", "schilling;schillings;ATS;ats;austria"),
      ki18nc("amount in units (real)", "%1 schillings"),
      ki18ncp("amount in units (integer)", "%1 schilling", "%1 schillings")
    );
    U(Currency::Bef, 0.0247894,
      i18nc("currency unit symbol", "BEF"),
      i18nc("unit description in lists", "francs"),
      i18nc("unit synonyms for matching user input", "franc;francs;BEF;bef;belgium"),
      ki18nc("amount in units (real)", "%1 francs"),
      ki18ncp("amount in units (integer)", "%1 franc", "%1 francs")
    );
    U(Currency::Nlg, 0.45378,
      i18nc("currency unit symbol", "NLG"),
      i18nc("unit description in lists", "guilders"),
      i18nc("unit synonyms for matching user input", "guilder;guilders;NLG;nlg;netherlands"),
      ki18nc("amount in units (real)", "%1 guilders"),
      ki18ncp("amount in units (integer)", "%1 guilder", "%1 guilders")
    );
    U(Currency::Fim, 0.168188,
      i18nc("currency unit symbol", "FIM"),
      i18nc("unit description in lists", "markkas"),
      i18nc("unit synonyms for matching user input", "markka;markkas;FIM;fim;finland;mk"),
      ki18nc("amount in units (real)", "%1 markkas"),
      ki18ncp("amount in units (integer)", "%1 markka", "%1 markkas")
    );
    U(Currency::Frf, 0.152449,
      i18nc("currency unit symbol", "FRF"),
      i18nc("unit description in lists", "francs"),
      i18nc("unit synonyms for matching user input", "franc;francs;FRF;frf;france"),
      ki18nc("amount in units (real)", "%1 francs"),
      ki18ncp("amount in units (integer)", "%1 franc", "%1 francs")
    );
    U(Currency::Dem, 0.511292,
      i18nc("currency unit symbol", "DEM"),
      i18nc("unit description in lists", "marks"),
      i18nc("unit synonyms for matching user input", "mark;marks;DEM;dem;germany"),
      ki18nc("amount in units (real)", "%1 marks"),
      ki18ncp("amount in units (integer)", "%1 mark", "%1 marks")
    );
    U(Currency::Iep, 1.26974,
      i18nc("currency unit symbol", "IEP"),
      i18nc("unit description in lists", "irish pounds"),
      i18nc("unit synonyms for matching user input", "irish pound;irish pounds;IEP;iep;ireland"),
      ki18nc("amount in units (real)", "%1 irish pounds"),
      ki18ncp("amount in units (integer)", "%1 irish pound", "%1 irish pounds")
    );
    U(Currency::Itl, 0.000516457,
      i18nc("currency unit symbol", "ITL"),
      i18nc("unit description in lists", "liras"),
      i18nc("unit synonyms for matching user input", "lira;liras;ITL;itl;italy"),
      ki18nc("amount in units (real)", "%1 liras"),
      ki18ncp("amount in units (integer)", "%1 lira", "%1 liras")
    );
    U(Currency::Luf, 0.0247894,
      i18nc("currency unit symbol", "LUF"),
      i18nc("unit description in lists", "francs"),
      i18nc("unit synonyms for matching user input", "franc;francs;LUF;luf;luxembourg"),
      ki18nc("amount in units (real)", "%1 francs"),
      ki18ncp("amount in units (integer)", "%1 franc", "%1 francs")
    );
    U(Currency::Pte, 0.00498798,
      i18nc("currency unit symbol", "PTE"),
      i18nc("unit description in lists", "escudos"),
      i18nc("unit synonyms for matching user input", "escudo;escudos;PTE;pte;portugal"),
      ki18nc("amount in units (real)", "%1 escudos"),
      ki18ncp("amount in units (integer)", "%1 escudo", "%1 escudos")
    );
    U(Currency::Esp, 0.00601012,
      i18nc("currency unit symbol", "ESP"),
      i18nc("unit description in lists", "pesetas"),
      i18nc("unit synonyms for matching user input", "peseta;pesetas;ESP;esp;spain"),
      ki18nc("amount in units (real)", "%1 pesetas"),
      ki18ncp("amount in units (integer)", "%1 peseta", "%1 pesetas")
    );
    U(Currency::Grd, 0.0029347,
      i18nc("currency unit symbol", "GRD"),
      i18nc("unit description in lists", "drachmas"),
      i18nc("unit synonyms for matching user input", "drachma;drachmas;GRD;grd;greece"),
      ki18nc("amount in units (real)", "%1 drachmas"),
      ki18ncp("amount in units (integer)", "%1 drachma", "%1 drachmas")
    );
    U(Currency::Sit, 0.00417293,
      i18nc("currency unit symbol", "SIT"),
      i18nc("unit description in lists", "tolars"),
      i18nc("unit synonyms for matching user input", "tolar;tolars;SIT;sit;slovenia"),
      ki18nc("amount in units (real)", "%1 tolars"),
      ki18ncp("amount in units (integer)", "%1 tolar", "%1 tolars")
    );
    U(Currency::Cyp, 1.7086,
      i18nc("currency unit symbol", "CYP"),
      i18nc("unit description in lists", "cypriot pounds"),
      i18nc("unit synonyms for matching user input", "cypriot pound;cypriot pounds;CYP;cyp;cyprus"),
      ki18nc("amount in units (real)", "%1 cypriot pounds"),
      ki18ncp("amount in units (integer)", "%1 cypriot pound", "%1 cypriot pounds")
    );
    U(Currency::Mtl, 2.32937,
      i18nc("currency unit symbol", "MTL"),
      i18nc("unit description in lists", "maltese liras"),
      i18nc("unit synonyms for matching user input", "maltese lira;maltese liras;MTL;mtl;malta"),
      ki18nc("amount in units (real)", "%1 maltese liras"),
      ki18ncp("amount in units (integer)", "%1 maltese lira", "%1 maltese liras")
    );
    U(Currency::Skk, 0.0331939,
      i18nc("currency unit symbol", "SKK"),
      i18nc("unit description in lists", "korunas"),
      i18nc("unit synonyms for matching user input", "koruna;korunas;SKK;skk;slovakia"),
      ki18nc("amount in units (real)", "%1 korunas"),
      ki18ncp("amount in units (integer)", "%1 koruna", "%1 korunas")
    );

    // From ECB
    U(Currency::Usd, 1e+99,
      i18nc("currency unit symbol", "USD"),
      i18nc("unit description in lists", "dollars"),
      i18nc("unit synonyms for matching user input", "dollar;dollars;USD;usd;usa;$"),
      ki18nc("amount in units (real)", "%1 dollars"),
      ki18ncp("amount in units (integer)", "%1 dollar", "%1 dollars")
    );
    U(Currency::Jpy, 1e+99,
      i18nc("currency unit symbol", "JPY"),
      i18nc("unit description in lists", "yens"),
      i18nc("unit synonyms for matching user input", "yen;yens;JPY;jpy;japan;¥"),
      ki18nc("amount in units (real)", "%1 yens"),
      ki18ncp("amount in units (integer)", "%1 yen", "%1 yens")
    );
    U(Currency::Bgn, 1e+99,
      i18nc("currency unit symbol", "BGN"),
      i18nc("unit description in lists", "levs"),
      i18nc("unit synonyms for matching user input", "lev;levs;BGN;bgn;bulgaria"),
      ki18nc("amount in units (real)", "%1 levs"),
      ki18ncp("amount in units (integer)", "%1 lev", "%1 levs")
    );
    U(Currency::Czk, 1e+99,
      i18nc("currency unit symbol", "CZK"),
      i18nc("unit description in lists", "korunas"),
      i18nc("unit synonyms for matching user input", "koruna;korunas;CZK;czk;czech"),
      ki18nc("amount in units (real)", "%1 korunas"),
      ki18ncp("amount in units (integer)", "%1 koruna", "%1 korunas")
    );
    U(Currency::Dkk, 1e+99,
      i18nc("currency unit symbol", "DKK"),
      i18nc("unit description in lists", "danish krones"),
      i18nc("unit synonyms for matching user input", "danish krone;danish krones;DKK;dkk;denmark"),
      ki18nc("amount in units (real)", "%1 danish krones"),
      ki18ncp("amount in units (integer)", "%1 danish krone", "%1 danish krones")
    );
    U(Currency::Eek, 1e+99,
      i18nc("currency unit symbol", "EEK"),
      i18nc("unit description in lists", "kroons"),
      i18nc("unit synonyms for matching user input", "kroon;kroons;EEK;eek;estonia"),
      ki18nc("amount in units (real)", "%1 kroons"),
      ki18ncp("amount in units (integer)", "%1 kroon", "%1 kroons")
    );
    U(Currency::Gbp, 1e+99,
      i18nc("currency unit symbol", "GBP"),
      i18nc("unit description in lists", "pounds"),
      i18nc("unit synonyms for matching user input",
            "pound;pounds;GBP;gbp;£;UK;sterling;sterlings"),
      ki18nc("amount in units (real)", "%1 pounds"),
      ki18ncp("amount in units (integer)", "%1 pound", "%1 pounds")
    );
    U(Currency::Huf, 1e+99,
      i18nc("currency unit symbol", "HUF"),
      i18nc("unit description in lists", "forints"),
      i18nc("unit synonyms for matching user input", "forint;forints;HUF;huf;hungary"),
      ki18nc("amount in units (real)", "%1 forints"),
      ki18ncp("amount in units (integer)", "%1 forint", "%1 forints")
    );
    U(Currency::Ltl, 1e+99,
      i18nc("currency unit symbol", "LTL"),
      i18nc("unit description in lists", "litass"),
      i18nc("unit synonyms for matching user input", "litas;litass;LTL;ltl;lithuania"),
      ki18nc("amount in units (real)", "%1 litass"),
      ki18ncp("amount in units (integer)", "%1 litas", "%1 litass")
    );
    U(Currency::Lvl, 1e+99,
      i18nc("currency unit symbol", "LVL"),
      i18nc("unit description in lists", "latss"),
      i18nc("unit synonyms for matching user input", "lats;latss;LVL;lvl;latvia"),
      ki18nc("amount in units (real)", "%1 latss"),
      ki18ncp("amount in units (integer)", "%1 lats", "%1 latss")
    );
    U(Currency::Pln, 1e+99,
      i18nc("currency unit symbol", "PLN"),
      i18nc("unit description in lists", "zlotys"),
      i18nc("unit synonyms for matching user input", "zloty;zlotys;PLN;pln;poland"),
      ki18nc("amount in units (real)", "%1 zlotys"),
      ki18ncp("amount in units (integer)", "%1 zloty", "%1 zlotys")
    );
    U(Currency::Ron, 1e+99,
      i18nc("currency unit symbol", "RON"),
      i18nc("unit description in lists", "leus"),
      i18nc("unit synonyms for matching user input", "leu;leus;RON;ron;romania"),
      ki18nc("amount in units (real)", "%1 leus"),
      ki18ncp("amount in units (integer)", "%1 leu", "%1 leus")
    );
    U(Currency::Sek, 1e+99,
      i18nc("currency unit symbol", "SEK"),
      i18nc("unit description in lists", "kronas"),
      i18nc("unit synonyms for matching user input", "krona;kronas;SEK;sek;sweden"),
      ki18nc("amount in units (real)", "%1 kronas"),
      ki18ncp("amount in units (integer)", "%1 krona", "%1 kronas")
    );
    U(Currency::Chf, 1e+99,
      i18nc("currency unit symbol", "CHF"),
      i18nc("unit description in lists", "francs"),
      i18nc("unit synonyms for matching user input", "franc;francs;CHF;chf;switzerland"),
      ki18nc("amount in units (real)", "%1 francs"),
      ki18ncp("amount in units (integer)", "%1 franc", "%1 francs")
    );
    U(Currency::Nok, 1e+99,
      i18nc("currency unit symbol", "NOK"),
      i18nc("unit description in lists", "norwegian krones"),
      i18nc("unit synonyms for matching user input",
            "norwegian krone;norwegian krones;NOK;nok;norway"),
      ki18nc("amount in units (real)", "%1 norwegian krones"),
      ki18ncp("amount in units (integer)", "%1 norwegian krone", "%1 norwegian krones")
    );
    U(Currency::Hrk, 1e+99,
      i18nc("currency unit symbol", "HRK"),
      i18nc("unit description in lists", "kunas"),
      i18nc("unit synonyms for matching user input", "kuna;kunas;HRK;hrk;croatia"),
      ki18nc("amount in units (real)", "%1 kunas"),
      ki18ncp("amount in units (integer)", "%1 kuna", "%1 kunas")
    );
    U(Currency::Rub, 1e+99,
      i18nc("currency unit symbol", "RUB"),
      i18nc("unit description in lists", "roubles"),
      i18nc("unit synonyms for matching user input", "rouble;roubles;RUB;rub;russia"),
      ki18nc("amount in units (real)", "%1 roubles"),
      ki18ncp("amount in units (integer)", "%1 rouble", "%1 roubles")
    );
    U(Currency::Try, 1e+99,
      i18nc("currency unit symbol", "TRY"),
      i18nc("unit description in lists", "liras"),
      i18nc("unit synonyms for matching user input", "lira;liras;TRY;try;turkey"),
      ki18nc("amount in units (real)", "%1 liras"),
      ki18ncp("amount in units (integer)", "%1 lira", "%1 liras")
    );
    U(Currency::Aud, 1e+99,
      i18nc("currency unit symbol", "AUD"),
      i18nc("unit description in lists", "australian dollars"),
      i18nc("unit synonyms for matching user input",
            "australian dollar;australian dollars;AUD;aud;australia"),
      ki18nc("amount in units (real)", "%1 australian dollars"),
      ki18ncp("amount in units (integer)", "%1 australian dollar", "%1 australian dollars")
    );
    U(Currency::Brl, 1e+99,
      i18nc("currency unit symbol", "BRL"),
      i18nc("unit description in lists", "reals"),
      i18nc("unit synonyms for matching user input", "real;reals;BRL;brasilia"),
      ki18nc("amount in units (real)", "%1 reals"),
      ki18ncp("amount in units (integer)", "%1 real", "%1 reals")
    );
    U(Currency::Cad, 1e+99,
      i18nc("currency unit symbol", "CAD"),
      i18nc("unit description in lists", "canadian dollars"),
      i18nc("unit synonyms for matching user input",
            "canadian dollar;canadian dollars;CAD;cad;canada"),
      ki18nc("amount in units (real)", "%1 canadian dollars"),
      ki18ncp("amount in units (integer)", "%1 canadian dollar", "%1 canadian dollars")
    );
    U(Currency::Cny, 1e+99,
      i18nc("currency unit symbol", "CNY"),
      i18nc("unit description in lists", "yuans"),
      i18nc("unit synonyms for matching user input", "yuan;yuans;CNY;china"),
      ki18nc("amount in units (real)", "%1 yuans"),
      ki18ncp("amount in units (integer)", "%1 yuan", "%1 yuans")
    );
    U(Currency::Hkd, 1e+99,
      i18nc("currency unit symbol", "HKD"),
      i18nc("unit description in lists", "hong kong dollars"),
      i18nc("unit synonyms for matching user input",
            "hong kong dollar;hong kong dollars;HKD;hkd;hong kong"),
      ki18nc("amount in units (real)", "%1 hong kong dollars"),
      ki18ncp("amount in units (integer)", "%1 hong kong dollar", "%1 hong kong dollars")
    );
    U(Currency::Idr, 1e+99,
      i18nc("currency unit symbol", "IDR"),
      i18nc("unit description in lists", "rupiahs"),
      i18nc("unit synonyms for matching user input",
            "rupiah;rupiahs;IDR;idr;indonesia"),
      ki18nc("amount in units (real)", "%1 rupiahs"),
      ki18ncp("amount in units (integer)", "%1 rupiah", "%1 rupiahs")
    );
    U(Currency::Inr, 1e+99,
      i18nc("currency unit symbol", "INR"),
      i18nc("unit description in lists", "rupees"),
      i18nc("unit synonyms for matching user input", "rupee;rupees;INR;inr;india"),
      ki18nc("amount in units (real)", "%1 rupees"),
      ki18ncp("amount in units (integer)", "%1 rupee", "%1 rupees")
    );
    U(Currency::Krw, 1e+99,
      i18nc("currency unit symbol", "KRW"),
      i18nc("unit description in lists", "wons"),
      i18nc("unit synonyms for matching user input", "won;wons;KRW;krw;south korea"),
      ki18nc("amount in units (real)", "%1 wons"),
      ki18ncp("amount in units (integer)", "%1 won", "%1 wons")
    );
    U(Currency::Mxn, 1e+99,
      i18nc("currency unit symbol", "MXN"),
      i18nc("unit description in lists", "mexican pesos"),
      i18nc("unit synonyms for matching user input", "mexican peso;mexican pesos;MXN;mxn;mexico"),
      ki18nc("amount in units (real)", "%1 mexican pesos"),
      ki18ncp("amount in units (integer)", "%1 mexican peso", "%1 mexican pesos")
    );
    U(Currency::Myr, 1e+99,
      i18nc("currency unit symbol", "MYR"),
      i18nc("unit description in lists", "ringgits"),
      i18nc("unit synonyms for matching user input", "ringgit;ringgits;MYR;myr;malaysia"),
      ki18nc("amount in units (real)", "%1 ringgits"),
      ki18ncp("amount in units (integer)", "%1 ringgit", "%1 ringgits")
    );
    U(Currency::Nzd, 1e+99,
      i18nc("currency unit symbol", "NZD"),
      i18nc("unit description in lists", "new zealand dollars"),
      i18nc("unit synonyms for matching user input",
            "new zealand dollar;new zealand dollars;NZD;nzd;new zealand"),
      ki18nc("amount in units (real)", "%1 new zealand dollars"),
      ki18ncp("amount in units (integer)", "%1 new zealand dollar", "%1 new zealand dollars")
    );
    U(Currency::Php, 1e+99,
      i18nc("currency unit symbol", "PHP"),
      i18nc("unit description in lists", "philippine pesos"),
      i18nc("unit synonyms for matching user input",
            "philippine peso;philippine pesos;PHP;php;philippines"),
      ki18nc("amount in units (real)", "%1 philippine pesos"),
      ki18ncp("amount in units (integer)", "%1 philippine peso", "%1 philippine pesos")
    );
    U(Currency::Sgd, 1e+99,
      i18nc("currency unit symbol", "SGD"),
      i18nc("unit description in lists", "singapore dollars"),
      i18nc("unit synonyms for matching user input",
            "singapore dollar;singapore dollars;SGD;sgd;singapore"),
      ki18nc("amount in units (real)", "%1 singapore dollars"),
      ki18ncp("amount in units (integer)", "%1 singapore dollar", "%1 singapore dollars")
    );
    U(Currency::Thb, 1e+99,
      i18nc("currency unit symbol", "THB"),
      i18nc("unit description in lists", "bahts"),
      i18nc("unit synonyms for matching user input", "baht;bahts;THB;thb;thailand"),
      ki18nc("amount in units (real)", "%1 bahts"),
      ki18ncp("amount in units (integer)", "%1 baht", "%1 bahts")
    );
    U(Currency::Zar, 1e+99,
      i18nc("currency unit symbol", "ZAR"),
      i18nc("unit description in lists", "rands"),
      i18nc("unit synonyms for matching user input", "rand;rands;ZAR;zar;south africa"),
      ki18nc("amount in units (real)", "%1 rands"),
      ki18ncp("amount in units (integer)", "%1 rand", "%1 rands")
    );

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
