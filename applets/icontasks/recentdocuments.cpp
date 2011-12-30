/*
 * Icon Task Manager
 *
 * Copyright 2011 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "recentdocuments.h"
#include <KDE/KRecentDocument>
#include <KDE/KDirWatch>
#include <KDE/KGlobal>
#include <KDE/KDesktopFile>
#include <KDE/KConfigGroup>
#include <KDE/KIcon>
#include <KDE/KRun>
#include <KDE/KStandardDirs>
#include <KDE/KSycoca>
#include <KDE/KDebug>
#include <KDE/KServiceTypeTrader>
#include <KDE/KService>
#include <KDE/KMimeType>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QtXml/QDomText>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QDateTime>

K_GLOBAL_STATIC(RecentDocuments, recentDocs)

static QLatin1String constXbel("recently-used.xbel");

static QList<QAction *>::ConstIterator findUrl(const QList<QAction *> &list, const QString &url)
{
    QList<QAction *>::ConstIterator it(list.constBegin()),
          end(list.constEnd());
    for (; it != end; ++it) {
        if ((*it)->property("url") == url) {
            break;
        }
    }
    return it;
}

static bool hasUrl(const QList<QAction *> &list, const QString &url)
{
    return list.end() != findUrl(list, url);
}

static QString dirSyntax(const QString &d)
{
    if (!d.isEmpty()) {
        QString ds(d);

        ds.replace("//", "/");

        int slashPos(ds.lastIndexOf('/'));

        if (slashPos != (((int)ds.length()) - 1))
            ds.append('/');

        return ds;
    }

    return d;
}

RecentDocuments * RecentDocuments::self()
{
    return recentDocs;
}

RecentDocuments::RecentDocuments()
    : m_enabled(false)
    , m_watcher(0)
    , m_menu(0)
{
}

RecentDocuments::~RecentDocuments()
{
    if (m_menu) {
        m_menu->deleteLater();
    }
}

void RecentDocuments::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        if (enabled) {
            if (m_files.isEmpty()) {
                m_files << File(File::Xbel, dirSyntax(KGlobal::dirs()->localxdgdatadir()) + constXbel)
                        << File(File::Xbel, dirSyntax(QDir::homePath()) + "." + constXbel)
                        << File(File::Office, dirSyntax(QDir::homePath()) + ".recently-used");
            }

            m_watcher = new KDirWatch(this);
            m_watcher->addDir(KRecentDocument::recentDocumentDirectory(), KDirWatch::WatchFiles);
            foreach (File f, m_files) {
                m_watcher->addFile(f.path);
            }
            connect(m_watcher, SIGNAL(created(QString)), this, SLOT(added(QString)));
            connect(m_watcher, SIGNAL(deleted(QString)), this, SLOT(removed(QString)));
            connect(m_watcher, SIGNAL(dirty(QString)), this, SLOT(modified(QString)));
            connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(sycocaChanged(const QStringList &)));
            readCurrentDocs();
        } else if (m_enabled) {
            disconnect(m_watcher, SIGNAL(created(QString)), this, SLOT(added(QString)));
            disconnect(m_watcher, SIGNAL(deleted(QString)), this, SLOT(removed(QString)));
            disconnect(m_watcher, SIGNAL(dirty(QString)), this, SLOT(modified(QString)));
            disconnect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(sycocaChanged(const QStringList &)));
            delete m_watcher;
            m_watcher = 0;

            QMap<QString, QList<QAction *> >::Iterator it(m_docs.begin()),
                 end(m_docs.end());

            for (; it != end; ++it) {
                foreach (const QAction * act, *it) {
                    delete act;
                }
            }
            m_docs.clear();
            m_apps.clear();
        }
        m_enabled = enabled;
    }
}

QList<QAction *> RecentDocuments::get(const QString &app)
{
    if (m_enabled) {
        load();
        if (m_docs.contains(app)) {
            if (m_docs[app].count() > 1) {
                if (!m_menu) {
                    m_menu = new TaskManager::ToolTipMenu(0, i18n("Recent Documents"));
                }

                QList<QAction *> old=m_menu->actions();
                foreach (QAction * act, old) {
                    m_menu->removeAction(act);
                }

                foreach (QAction * act, m_docs[app]) {
                    m_menu->addAction(act);
                }

                QList<QAction *> acts;
                acts.append(m_menu->menuAction());
                return acts;
            }
            return m_docs[app];
        }
    }
    return QList<QAction *>();
}

void RecentDocuments::added(const QString &path)
{
    if (KDesktopFile::isDesktopFile(path)) {
        removed(path); // Remove first!
        KDesktopFile df(path);
        KConfigGroup de(&df, "Desktop Entry");
        QString url = de.readEntry("URL", QString());
        QString name = KUrl(url).fileName();
        QString app = de.readEntry("X-KDE-LastOpenedWith", QString());

        if (!name.isEmpty() && !app.isEmpty() && !url.isEmpty() && !hasUrl(m_docs[app], url)) {
            QString icon = de.readEntry("Icon", QString());
            QAction *act = icon.isEmpty() ? new QAction(name, this) :  new QAction(KIcon(icon), name, this);
            act->setToolTip(KUrl(url).prettyUrl());
            act->setProperty("timestamp", (qulonglong)0);
            act->setProperty("path", path);
            act->setProperty("url", url);
            connect(act, SIGNAL(triggered()), SLOT(loadDoc()));
            m_docs[app].append(act);
        }
    } else {
        QList<File>::Iterator it(m_files.begin()),
              end(m_files.end());
        for (; it != end; ++it) {
            if ((*it).path == path) {
                (*it).dirty = true;
                break;
            }
        }
    }
}

void RecentDocuments::removed(const QString &path)
{
    if (path.endsWith(".desktop")) {
        QMap<QString, QList<QAction *> >::Iterator it(m_docs.begin()),
             end(m_docs.end());

        for (; it != end; ++it) {
            foreach (QAction * act, *it) {
                if (act->property("path").toString() == path) {
                    disconnect(act, SIGNAL(triggered()), this, SLOT(loadDoc()));
                    delete act;
                    (*it).removeAll(act);
                    if ((*it).isEmpty()) {
                        m_docs.erase(it);
                    }
                    return;
                }
            }
        }
    } else {
        QList<File>::Iterator it(m_files.begin()),
              end(m_files.end());
        for (; it != end; ++it) {
            if ((*it).path == path) {
                (*it).dirty = true;
                break;
            }
        }
    }
}

void RecentDocuments::modified(const QString &path)
{
    QList<File>::Iterator it(m_files.begin()),
          end(m_files.end());
    for (; it != end; ++it) {
        if ((*it).path == path) {
            (*it).dirty = true;
            break;
        }
    }
}

void RecentDocuments::sycocaChanged(const QStringList &types)
{
    if (types.contains("apps")) {
        m_apps.clear();
        QList<File>::Iterator it(m_files.begin()),
              end(m_files.end());
        for (; it != end; ++it) {
            if (File::Xbel == (*it).type) {
                (*it).dirty = true;
            }
        }
    }
}

void RecentDocuments::loadDoc()
{
    QObject *s = sender();
    if (s && qobject_cast<QAction *>(s)) {
        QAction *item = static_cast<QAction *>(s);
        QString path = item->property("path").toString();

        if (path.isEmpty()) {
            QString exec = item->property("exec").toString();
            KUrl url = KUrl(item->property("url").toString());

            if (url.isValid() && !exec.isEmpty()) {
                KRun::run(exec, KUrl::List() << url, 0, QString(), QString(), "0");
            }
        } else {
            new KRun(KUrl(path), 0);
        }
    }
}

void RecentDocuments::readCurrentDocs()
{
    const QStringList documents = KRecentDocument::recentDocuments();
    foreach (const QString & document, documents) {
        added(document);
    }
}

void RecentDocuments::load()
{
    qulonglong now = (qulonglong)QDateTime::currentMSecsSinceEpoch();
    QList<File>::Iterator it(m_files.begin()),
          end(m_files.end());
    for (; it != end; ++it) {
        if ((*it).dirty) {
            if (File::Xbel == (*it).type) {
                loadXbel((*it).path, now);
            } else if (File::Office == (*it).type) {
                loadOffice((*it).path, now);
            }
            (*it).dirty = false;
        }
    }
}

static QString convertMimeType(const QString &mimeType, const KUrl &url)
{
    return mimeType == "text/plain" && url.fileName().endsWith(".csv")
        ? QLatin1String("text/csv") : mimeType;
}

RecentDocuments::App RecentDocuments::officeAppForMimeType(const QString &mimeType)
{
    if (m_apps.contains(mimeType)) {
        return m_apps[mimeType];
    } else {
        KService::List services = KServiceTypeTrader::self()->query("Application",
                                    QString("exist Exec and (exist ServiceTypes) and ('libreoffice' ~ Exec) and ('%1' in ServiceTypes)").arg(mimeType));

        if (!services.empty()) {
            QString desktopFile = services[0]->entryPath();
            KDesktopFile df(desktopFile);
            KConfigGroup grp(&df, "Desktop Entry");
            QString exec = grp.readEntry("Exec", QString());

            if (!exec.isEmpty()) {
                App app(KUrl::fromPath(desktopFile).fileName().remove(".desktop"), exec);
                m_apps.insert(mimeType, app);
                return app;
            }
        }
    }

    return App();
}

RecentDocuments::App RecentDocuments::appForExec(const QString &execString)
{
    if (m_apps.contains(execString)) {
        return m_apps[execString];
    } else {
        KService::List services = KServiceTypeTrader::self()->query("Application",
                                    QString("exist Exec and ('%1' =~ Exec)").arg(execString));
        if (services.empty()) {
            QString execApp = execString;
            int space = execApp.indexOf(' ');
            if (-1 != space) {
                execApp = execApp.left(space);
            }
            services = KServiceTypeTrader::self()->query("Application",
                        QString("exist TryExec and ('%1' =~ TryExec)").arg(execApp));
        }
        if (!services.empty()) {
            QString desktopFile = services[0]->entryPath();
            KDesktopFile df(desktopFile);
            KConfigGroup grp(&df, "Desktop Entry");
            QString exec = grp.readEntry("Exec", QString());

            if (!exec.isEmpty()) {
                App app(KUrl::fromPath(desktopFile).fileName().remove(".desktop"), exec);
                m_apps.insert(execString, app);
                return app;
            }
        }
    }

    return App();
}

void RecentDocuments::loadXbel(const QString &path, qulonglong now)
{
    QDomDocument doc("xbel");
    QFile f(path);

    if (f.open(QIODevice::ReadOnly) && doc.setContent(&f)) {
        QDomElement root = doc.documentElement();
        if ("xbel" == root.tagName() && root.hasAttribute("version") && "1.0" == root.attribute("version")) {
            QDomElement bookmark = root.firstChildElement("bookmark");
            while (!bookmark.isNull()) {
                if (bookmark.hasAttribute("href")) {
                    QDomElement info = bookmark.firstChildElement("info");
                    if (!info.isNull()) {
                        QDomElement metadata = info.firstChildElement("metadata");
                        if (!metadata.isNull() && metadata.hasAttribute("owner") && "http://freedesktop.org" == metadata.attribute("owner")) {
                            QDomElement applications = metadata.firstChildElement("bookmark:applications");
                            if (!applications.isNull()) {
                                QDomElement application = applications.firstChildElement("bookmark:application");
                                if (!application.isNull() && application.hasAttribute("exec")) {
                                    KUrl url = bookmark.attribute("href");
                                    if (url.isValid() && (!url.isLocalFile() || QFile::exists(url.toLocalFile()))) {
                                        QString exec = application.attribute("exec");
                                        QDomElement mimeType = metadata.firstChildElement("mime:mime-type");
                                        QString mType;
                                        KMimeType::Ptr mime;
                                        if (!mimeType.isNull() && mimeType.hasAttribute("type")) {
                                            mType = convertMimeType(mimeType.attribute("type"), url);
                                            mime = KMimeType::mimeType(mType);
                                        }

                                        exec.remove('\'');

                                        App app = mime && QLatin1String("soffice %u")==exec
                                                    ? officeAppForMimeType(mType)
                                                    : appForExec(exec);

                                        if (!app.name.isEmpty()) {
                                            QString name = KUrl(url).fileName();

                                            if (!name.isEmpty()) {
                                                bool found = false;
                                                if (!m_docs[app.name].isEmpty()) {
                                                    QList<QAction *>::ConstIterator it = findUrl(m_docs[app.name], url.url());
                                                    if (it != m_docs[app.name].constEnd()) {
                                                        found = true;
                                                        if ((*it)->property("timestamp").toULongLong() > 0) {
                                                            (*it)->setProperty("timestamp", now);
                                                        }
                                                    }
                                                }
                                                if (!found) {
                                                    QAction *act = mime
                                                                    ? new QAction(KIcon(mime->iconName()), name, this)
                                                                    : new QAction(name, this);

                                                    act->setToolTip(KUrl(url).prettyUrl());
                                                    act->setProperty("timestamp", now);
                                                    act->setProperty("url", url.url());
                                                    act->setProperty("exec", app.exec);
                                                    act->setProperty("type", (int)File::Xbel);
                                                    connect(act, SIGNAL(triggered()), SLOT(loadDoc()));
                                                    m_docs[app.name].append(act);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                bookmark = bookmark.nextSiblingElement("bookmark");
            }
        }
    }

    removeOld(now, File::Xbel);
}

void RecentDocuments::loadOffice(const QString &path, qulonglong now)
{
    QDomDocument doc("RecentFiles");
    QFile f(path);

    if (f.open(QIODevice::ReadOnly) && doc.setContent(&f)) {
        QDomElement root = doc.documentElement();
        if ("RecentFiles" == root.tagName()) {
            QDomElement recentItem = root.firstChildElement("RecentItem");
            while (!recentItem.isNull()) {
                QDomElement groups = recentItem.firstChildElement("Groups");
                if (!groups.isNull()) {
                    QDomElement group = groups.firstChildElement("Group");
                    bool ok = false;
                    while (!group.isNull()) {
                        if (group.text() == "openoffice.org") {
                            ok = true;
                            break;
                        }
                        group = group.nextSiblingElement("Group");
                    }

                    if (ok) {
                        QDomElement uri = recentItem.firstChildElement("URI");
                        QDomElement mimeType = recentItem.firstChildElement("Mime-Type");

                        if (!uri.isNull() && !mimeType.isNull()) {
                            KUrl url(uri.text());

                            if (url.isValid() && (!url.isLocalFile() || QFile::exists(url.toLocalFile()))) {
                                QString mType = convertMimeType(mimeType.text(), url);
                                App app = officeAppForMimeType(mType);

                                if (!app.name.isEmpty() && !app.exec.isEmpty()) {
                                    QString name = KUrl(url).fileName();

                                    if (!name.isEmpty()) {
                                        bool found = false;
                                        if (!m_docs[app.name].isEmpty()) {
                                            QList<QAction *>::ConstIterator it = findUrl(m_docs[app.name], url.url());
                                            if (it != m_docs[app.name].constEnd()) {
                                                found = true;
                                                if ((*it)->property("timestamp").toULongLong() > 0) {
                                                    (*it)->setProperty("timestamp", now);
                                                }
                                            }
                                        }
                                        if (!found) {
                                            KMimeType::Ptr mime = KMimeType::mimeType(mType);
                                            QAction *act = mime
                                                           ? new QAction(KIcon(mime->iconName()), name, this)
                                                           : new QAction(name, this);

                                            act->setToolTip(KUrl(url).prettyUrl());
                                            act->setProperty("local", false);
                                            act->setProperty("timestamp", now);
                                            act->setProperty("url", url.url());
                                            act->setProperty("exec", app.exec);
                                            act->setProperty("type", (int)File::Office);
                                            connect(act, SIGNAL(triggered()), SLOT(loadDoc()));
                                            m_docs[app.name].append(act);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                recentItem = recentItem.nextSiblingElement("RecentItem");
            }
        }
    }

    removeOld(now, File::Office);
}

void RecentDocuments::removeOld(qulonglong now, File::Type type)
{
    QMap<QString, QList<QAction *> >::Iterator it(m_docs.begin()),
         end(m_docs.end());
    while (it != end) {
        QList<QAction *> old;

        foreach (QAction * act, (*it)) {
            qulonglong t = act->property("timestamp").toULongLong();
            if (type==act->property("type").toInt() && t > 0 && t < now) {
                old.append(act);
            }
        }

        foreach (QAction * act, old) {
            act->deleteLater();
            (*it).removeAll(act);
        }

        if ((*it).isEmpty()) {
            QMap<QString, QList<QAction *> >::Iterator cur = it;
            it++;
            m_docs.erase(cur);
        } else {
            it++;
        }
    }
}

#include "recentdocuments.moc"
