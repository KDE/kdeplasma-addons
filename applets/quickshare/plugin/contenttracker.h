/*
 *   Copyright 2011 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CONTENTTRACKER_H
#define CONTENTTRACKER_H

#include <QObject>
#include <QPointer>

class OrgKdeActivityManagerSLCInterface;

class ContentTracker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString uri READ uri NOTIFY changed)
    Q_PROPERTY(QString mimeType READ mimeType NOTIFY changed)
    Q_PROPERTY(QString title READ title NOTIFY changed)

public:
    explicit ContentTracker(QObject *parent = nullptr);
    ~ContentTracker() override;

    QString uri() const;
    QString mimeType() const;
    QString title() const;

Q_SIGNALS:
    void changed();

protected:
    void connectToActivityManager();

private Q_SLOTS:
    void focusChanged(const QString &uri, const QString &mimetype, const QString &title);
    void serviceChange(const QString& name, const QString& oldOwner, const QString& newOwner);

private:
    QPointer<OrgKdeActivityManagerSLCInterface> m_activityManagerIface;
    QString m_uri;
    QString m_mimetype;
    QString m_title;
    int m_window;
};

#endif
