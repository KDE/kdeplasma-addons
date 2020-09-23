/*
 *   SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
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
