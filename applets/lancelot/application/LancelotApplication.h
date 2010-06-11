/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOTAPPLICATION_H
#define LANCELOTAPPLICATION_H

#include <QTimer>
#include <QSet>
#include <QStringList>
#include <QSignalMapper>

#include <KUniqueApplication>

#include <Plasma/Plasma>

class LancelotWindow;
class KActionCollection;

class LancelotApplication: public KUniqueApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.lancelot.App")

public:
    static int main(int argc, char **argv);
    bool event(QEvent * e);

public Q_SLOTS:
    static bool show(int x, int y);
    static bool showCentered();
    static bool showItem(int x, int y, const QString & name);

    static bool hide(bool immediate = false);
    static bool search(const QString & string);

    static bool isShowing();
    static bool isShowing(const QString & section);

    void configureShortcuts();
    void configurationChanged();
    void showMenuEditor();

    int addClient();
    bool removeClient(int id);

    QStringList sectionIDs();
    QStringList sectionNames();
    QStringList sectionIcons();

    void setImmutability(int immutable);
    int immutability() const;

protected:
    LancelotWindow * window;

private:
    static LancelotApplication * m_application;

    LancelotApplication(int argc, char **argv);
    LancelotApplication(Display * display,
        Qt::HANDLE visual = 0,
        Qt::HANDLE colormap = 0,
        bool configUnique = false);
    void init();

    virtual ~LancelotApplication();

    int m_clientsNumber;
    int m_lastID;
    QSet<int> m_clients;

};

#endif /*LANCELOTAPPLICATION_H*/
