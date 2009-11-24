/*************************************************************************
 * Copyright 2009 Sandro Andrade sandroandrade@kde.org                   *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#include "kdeobservatorypresets.h"

QList<QStringList> KdeObservatoryPresets::presets;

void KdeObservatoryPresets::init()
{
    // Columns:
    // Project Name; commit subject; krazy report; krazy identifier; icon; automatically add in views ?

    presets
    // Plasma
    << (QStringList() << "Plasma"        << "plasma"        << "reports/kde-4.x/kdebase-workspace/plasmaaa/index.html" << "" << "plasma"                    << "true")
    // KDevelop + KDevPlatform + KDEWebDev
    << (QStringList() << "KDevelop"      << "kdevelop"      << "component=kde-4.x&module=kdevelop"                     << "" << "kde"                       << "false")
    << (QStringList() << "KDevPlatform"  << "kdevplatform"  << "component=kde-4.x&module=kdevplatform"                 << "" << "kde"                       << "false")
    << (QStringList() << "KDEWebDev"     << "kdewebdev"     << "component=kde-4.x&module=kdewebdev"                    << "" << "kde"                       << "false")
    // KDE-Edu
    << (QStringList() << "KAlgebra"      << "kalgebra"      << "reports/kde-4.x/kdeedu/kalgebra/index.html"            << "" << "kalgebra"                  << "false")
    << (QStringList() << "Kalzium"       << "kalzium"       << "reports/kde-4.x/kdeedu/kalzium/index.html"             << "" << "kalzium"                   << "false")
    << (QStringList() << "KAnagram"      << "kanagram"      << "reports/kde-4.x/kdeedu/kanagram/index.html"            << "" << "kanagram"                  << "false")
    << (QStringList() << "KBruch"        << "kbruch"        << "reports/kde-4.x/kdeedu/kbruch/index.html"              << "" << "kbruch"                    << "false")
    << (QStringList() << "KGeography"    << "kgeography"    << "reports/kde-4.x/kdeedu/kgeography/index.html"          << "" << "kgeography"                << "false")
    << (QStringList() << "Khangman"      << "khangman"      << "reports/kde-4.x/kdeedu/khangman/index.html"            << "" << "khangman"                  << "false")
    << (QStringList() << "Kig"           << "kig"           << "reports/kde-4.x/kdeedu/kig/index.html"                 << "" << "kig"                       << "false")
    << (QStringList() << "Kiten"         << "kiten"         << "reports/kde-4.x/kdeedu/kiten/index.html"               << "" << "kiten"                     << "false")
    << (QStringList() << "KLettres"      << "klettres"      << "reports/kde-4.x/kdeedu/klettres/index.html"            << "" << "klettres"                  << "false")
    << (QStringList() << "Kmplot"        << "kmplot"        << "reports/kde-4.x/kdeedu/kmplot/index.html"              << "" << "kmplot"                    << "false")
    << (QStringList() << "KStars"        << "kstars"        << "reports/kde-4.x/kdeedu/kstars/index.html"              << "" << "kstars"                    << "false")
    << (QStringList() << "KTouch"        << "ktouch"        << "reports/kde-4.x/kdeedu/ktouch/index.html"              << "" << "ktouch"                    << "false")
    << (QStringList() << "KTurtle"       << "kturtle"       << "reports/kde-4.x/kdeedu/kturtle/index.html"             << "" << "kturtle"                   << "false")
    << (QStringList() << "KWordQuiz"     << "kwordquiz"     << "reports/kde-4.x/kdeedu/kwordquiz/index.html"           << "" << "kwordquiz"                 << "false")
    << (QStringList() << "Marble"        << "marble"        << "reports/kde-4.x/kdeedu/marble/index.html"              << "" << "marble"                    << "false")
    << (QStringList() << "Parley"        << "parley"        << "reports/kde-4.x/kdeedu/parley/index.html"              << "" << "parley"                    << "false")
    << (QStringList() << "Rocs"          << "rocs"          << "reports/kde-4.x/kdeedu/rocs/index.html"                << "" << "rocs"                      << "false")
    << (QStringList() << "Step"          << "step"          << "reports/kde-4.x/kdeedu/step/index.html"                << "" << "step"                      << "false")
    // KDE-Games
    << (QStringList() << "Bomber"        << "bomber"        << "reports/kde-4.x/kdegames/bomber/index.html"            << "" << "bomber"                    << "false")
    << (QStringList() << "Bovo"          << "bovo"          << "reports/kde-4.x/kdegames/bovo/index.html"              << "" << "bovo"                      << "false")
    << (QStringList() << "Kapman"        << "kapman"        << "reports/kde-4.x/kdegames/kapman/index.html"            << "" << "kapman"                    << "false")
    << (QStringList() << "KAtomic"       << "katomic"       << "reports/kde-4.x/kdegames/katomic/index.html"           << "" << "katomic"                   << "false")
    << (QStringList() << "KBattleShip"   << "kbattleship"   << "reports/kde-4.x/kdegames/kbattleship/index.html"       << "" << "kbattleship"               << "false")
    << (QStringList() << "KBlackBox"     << "kblackbox"     << "reports/kde-4.x/kdegames/kblackbox/index.html"         << "" << "kblackbox"                 << "false")
    << (QStringList() << "KBlocks"       << "kblocks"       << "reports/kde-4.x/kdegames/kblocks/index.html"           << "" << "kblocks"                   << "false")
    << (QStringList() << "KBounce"       << "kbounce"       << "reports/kde-4.x/kdegames/kbounce/index.html"           << "" << "kbounce"                   << "false")
    << (QStringList() << "KBreakout"     << "kbreakout"     << "reports/kde-4.x/kdegames/kbreakout/index.html"         << "" << "kbreakout"                 << "false")
    << (QStringList() << "KDiamond"      << "kdiamond"      << "reports/kde-4.x/kdegames/kdiamond/index.html"          << "" << "kdiamond"                  << "false")
    << (QStringList() << "KFourInLine"   << "kfourinline"   << "reports/kde-4.x/kdegames/kfourinline/index.html"       << "" << "kfourinline"               << "false")
    << (QStringList() << "KGoldRunner"   << "kgoldrunner"   << "reports/kde-4.x/kdegames/kgoldrunner/index.html"       << "" << "kgoldrunner"               << "false")
    << (QStringList() << "KillBots"      << "killbots"      << "reports/kde-4.x/kdegames/killbots/index.html"          << "" << "killbots"                  << "false")
    << (QStringList() << "Kiriki"        << "kiriki"        << "reports/kde-4.x/kdegames/kiriki/index.html"            << "" << "kiriki"                    << "false")
    << (QStringList() << "KJumpingCube"  << "kjumpingcube"  << "reports/kde-4.x/kdegames/kjumpingcube/index.html"      << "" << "kjumpingcube"              << "false")
    << (QStringList() << "KLines"        << "klines"        << "reports/kde-4.x/kdegames/klines/index.html"            << "" << "klines"                    << "false")
    << (QStringList() << "Kmahjongg"     << "kmahjongg"     << "reports/kde-4.x/kdegames/kmahjongg/index.html"         << "" << "kmahjongg"                 << "false")
    << (QStringList() << "KMines"        << "kmines"        << "reports/kde-4.x/kdegames/kmines/index.html"            << "" << "kmines"                    << "false")
    << (QStringList() << "KNetwalk"      << "knetwalk"      << "reports/kde-4.x/kdegames/knetwalk/index.html"          << "" << "knetwalk"                  << "false")
    << (QStringList() << "Kolf"          << "kolf"          << "reports/kde-4.x/kdegames/kolf/index.html"              << "" << "kolf"                      << "false")
    << (QStringList() << "Kollision"     << "kollision"     << "reports/kde-4.x/kdegames/kollision/index.html"         << "" << "kollision"                 << "false")
    << (QStringList() << "Konquest"      << "konquest"      << "reports/kde-4.x/kdegames/konquest/index.html"          << "" << "konquest"                  << "false")
    << (QStringList() << "Kpat"          << "kpat"          << "reports/kde-4.x/kdegames/kpat/index.html"              << "" << "kpat"                      << "false")
    << (QStringList() << "KReversi"      << "kreversi"      << "reports/kde-4.x/kdegames/kreversi/index.html"          << "" << "kreversi"                  << "false")
    << (QStringList() << "KSame"         << "ksame"         << "reports/kde-4.x/kdegames/ksame/index.html"             << "" << "ksame"                     << "false")
    << (QStringList() << "KShisen"       << "kshisen"       << "reports/kde-4.x/kdegames/kshisen/index.html"           << "" << "kshisen"                   << "false")
    << (QStringList() << "KSirk"         << "ksirk"         << "reports/kde-4.x/kdegames/ksirk/index.html"             << "" << "ksirk"                     << "false")
    << (QStringList() << "KSpaceDuel"    << "kspaceduel"    << "reports/kde-4.x/kdegames/kspaceduel/index.html"        << "" << "kspaceduel"                << "false")
    << (QStringList() << "KSquares"      << "ksquared"      << "reports/kde-4.x/kdegames/ksquares/index.html"          << "" << "ksquares"                  << "false")
    << (QStringList() << "KSudoku"       << "ksudoku"       << "reports/kde-4.x/kdegames/ksudoku/index.html"           << "" << "ksudoku"                   << "false")
    << (QStringList() << "KTron"         << "ktron"         << "reports/kde-4.x/kdegames/ktron/index.html"             << "" << "ktron"                     << "false")
    << (QStringList() << "KTuberling"    << "ktuberling"    << "reports/kde-4.x/kdegames/ktuberling/index.html"        << "" << "ktuberling"                << "false")
    << (QStringList() << "Kubrick"       << "kubrick"       << "reports/kde-4.x/kdegames/kubrick/index.html"           << "" << "kubrick"                   << "false")
    // KDE-Graphics
    << (QStringList() << "Gwenview"      << "gwenview"      << "reports/kde-4.x/kdegraphics/gwenview/index.html"       << "" << "gwenview"                  << "false")
    << (QStringList() << "KColorChooser" << "kcolorchooser" << "reports/kde-4.x/kdegraphics/kcolorchooser/index.html"  << "" << "kcolorchooser"             << "false")
    << (QStringList() << "KolourPaint"   << "kolourpaint"   << "reports/kde-4.x/kdegraphics/kolourpaint/index.html"    << "" << "kolourpaint"               << "false")
    << (QStringList() << "KRuler"        << "kruler"        << "reports/kde-4.x/kdegraphics/kruler/index.html"         << "" << "kruler"                   << "false")
    << (QStringList() << "KSnapshot"     << "ksnapshot"     << "reports/kde-4.x/kdegraphics/ksnapshot/index.html"      << "" << "ksnapshot"                 << "false")
    << (QStringList() << "Okular"        << "okular"        << "reports/kde-4.x/kdegraphics/okular/index.html"         << "" << "okular"                    << "false")
    // KDE-Multimidia
    << (QStringList() << "Dragon Player" << "dragonplayer"  << "reports/kde-4.x/kdemultimedia/dragonplayer/index.html" << "" << "dragonplayer"              << "false")
    << (QStringList() << "Juk"           << "juk"           << "reports/kde-4.x/kdemultimedia/juk/index.html"          << "" << "juk"                       << "false")
    << (QStringList() << "KMix"          << "kmix"          << "reports/kde-4.x/kdemultimedia/kmix/index.html"         << "" << "kmix"                      << "false")
    << (QStringList() << "Kscd"          << "kscd"          << "reports/kde-4.x/kdemultimedia/kscd/index.html"         << "" << "kscd"                      << "false")
    // KDE-Network
    << (QStringList() << "KGet"          << "kget"          << "reports/kde-4.x/kdenetwork/kget/index.html"            << "" << "kget"                      << "false")
    << (QStringList() << "Kopete"        << "kopete"        << "reports/kde-4.x/kdenetwork/kopete/index.html"          << "" << "kopete"                    << "false")
    << (QStringList() << "Kppp"          << "kppp"          << "reports/kde-4.x/kdenetwork/kppp/index.html"            << "" << "kppp"                      << "false")
    << (QStringList() << "Krdc"          << "krdc"          << "reports/kde-4.x/kdenetwork/krdc/index.html"            << "" << "krdc"                      << "false")
    << (QStringList() << "Krfb"          << "krfb"          << "reports/kde-4.x/kdenetwork/krfb/index.html"            << "" << "krfb"                      << "false")
    // KDE-PIM
    << (QStringList() << "Akonadi"       << "akonadi"       << "reports/kde-4.x/kdepim/akonadi/index.html"             << "" << "akonadi"                   << "false")
    << (QStringList() << "Akregator"     << "akregator"     << "reports/kde-4.x/kdepim/akregator/index.html"           << "" << "akregator"                 << "false")
    << (QStringList() << "KAddressBook"  << "kaddressbook"  << "reports/kde-4.x/kdepim/kaddressbook/index.html"        << "" << "kaddressbook"              << "false")
    << (QStringList() << "KAlarm"        << "kalarm"        << "reports/kde-4.x/kdepim/kalarm/index.html"              << "" << "kalarm"                    << "false")
    << (QStringList() << "KJots"         << "kjots"         << "reports/kde-4.x/kdepim/kjots/index.html"               << "" << "kjots"                     << "false")
    << (QStringList() << "Kleopatra"     << "kleopatra"     << "reports/kde-4.x/kdepim/kleopatra/index.html"           << "" << "kleopatra"                 << "false")
    << (QStringList() << "KMail"         << "kmail"         << "reports/kde-4.x/kdepim/kmail/index.html"               << "" << "kmail"                     << "false")
    << (QStringList() << "KNode"         << "knode"         << "reports/kde-4.x/kdepim/knode/index.html"               << "" << "knode"                     << "false")
    << (QStringList() << "KNotes"        << "knotes"        << "reports/kde-4.x/kdepim/knotes/index.html"              << "" << "knotes"                    << "false")
    << (QStringList() << "Kontact"       << "kontact"       << "reports/kde-4.x/kdepim/kontact/index.html"             << "" << "kontact"                   << "false")
    << (QStringList() << "Korganizer"    << "korganizer"    << "reports/kde-4.x/kdepim/korganizer/index.html"          << "" << "korganizer"                << "false")
    << (QStringList() << "KPilot"        << "kpilot"        << "reports/kde-4.x/kdepim/kpilot/index.html"              << "" << "kpilot"                    << "false")
    << (QStringList() << "KSendEmail"    << "ksendemail"    << "reports/kde-4.x/kdepim/ksendemail/index.html"          << "" << "kontact"                   << "false")
    << (QStringList() << "KTimeTracker"  << "ktimetracker"  << "reports/kde-4.x/kdepim/ktimetracker/index.html"        << "" << "ktimetracker"              << "false")
    // KDE-Toys
    << (QStringList() << "Amor"          << "amor"          << "reports/kde-4.x/kdetoys/amor/index.html"               << "" << "amor"                      << "false")
    << (QStringList() << "KTeaTime"      << "kteatime"      << "reports/kde-4.x/kdetoys/kteatime/index.html"           << "" << "kteatime"                  << "false")
    << (QStringList() << "KTux"          << "ktux"          << "reports/kde-4.x/kdetoys/ktux/index.html"               << "" << "ktux"                      << "false")
    << (QStringList() << "KWeather"      << "kweather"      << "reports/kde-4.x/kdetoys/kweather/index.html"           << "" << "kweather"                  << "false")
    // KDE-Utils
    << (QStringList() << "Ark"           << "ark"           << "reports/kde-4.x/kdeutils/ark/index.html"               << "" << "utilities-file-archiver"  << "false")
    << (QStringList() << "KCalc"         << "kcalc"         << "reports/kde-4.x/kdeutils/kcalc/index.html"             << "" << "accessories-calculator"    << "false")
    << (QStringList() << "KCharSelect"   << "kcharselect"   << "reports/kde-4.x/kdeutils/kcharselect/index.html"       << "" << "accessories-character-map" << "false")
    << (QStringList() << "KDE SSH"       << "kdessh"        << "reports/kde-4.x/kdeutils/kdessh/index.html"            << "" << "kde"                       << "false")
    << (QStringList() << "Kdf"           << "kdf"           << "reports/kde-4.x/kdeutils/kdf/index.html"               << "" << "kdf"                       << "false")
    << (QStringList() << "KFloppy"       << "kfloppy"       << "reports/kde-4.x/kdeutils/kfloppy/index.html"           << "" << "kfloppy"                   << "false")
    << (QStringList() << "KGPG"          << "kgpg"          << "reports/kde-4.x/kdeutils/kgpg/index.html"              << "" << "kgpg"                      << "false")
    << (QStringList() << "KTimer"        << "ktimer"        << "reports/kde-4.x/kdeutils/ktimer/index.html"            << "" << "ktimer"                    << "false")
    << (QStringList() << "KWallet"       << "kwallet"       << "reports/kde-4.x/kdeutils/kwallet/index.html"           << "" << "kwalletmanager"            << "false")
    << (QStringList() << "Okteta"        << "okteta"        << "reports/kde-4.x/kdeutils/okteta/index.html"            << "" << "okteta"                    << "false")
    << (QStringList() << "Superkaramba"  << "superkaramba"  << "reports/kde-4.x/kdeutils/superkaramba/index.html"      << "" << "superkaramba"              << "false")
    << (QStringList() << "Sweeper"       << "sweeper"       << "reports/kde-4.x/kdeutils/sweeper/index.html"           << "" << "trash-empty"               << "false")
    // KDE-SDK
    << (QStringList() << "KDE SDK"       << "kdesdk"        << "component=kde-4.x&module=kdesdk"                       << "" << "kde"                       << "false")
    // Libs
    << (QStringList() << "KDE Libs"      << "kdelibs"       << "component=kde-4.x&module=kdelibs"                      << "" << "kde"                       << "true")
    << (QStringList() << "KDE PIM Libs"  << "kdepimlibs"    << "component=kde-4.x&module=kdepimlibs"                   << "" << "kde"                       << "true")
    ;
}

QStringList KdeObservatoryPresets::preset(PresetInfo info)
{
    QStringList list;
    int count = presets.count();
    for (int i = 0; i < count; ++i)
        list << presets.at(i).at(info);
    return list;
}

QList<bool> KdeObservatoryPresets::automaticallyInViews()
{
    QList<bool> list;
    int count = presets.count();
    for (int i = 0; i < count; ++i)
        list << ((presets.at(i).last() == "false") ? false:true);
    return list;
}

QStringList KdeObservatoryPresets::viewsPreset()
{
    return QStringList() << "Top Active Projects" << "Commit History" << "Top Developers" << "Krazy Report";
}

QList<bool> KdeObservatoryPresets::viewsActivePreset()
{
    return QList<bool>() << true << true << true << true;
}
