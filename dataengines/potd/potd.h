/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef POTD_DATAENGINE_H
#define POTD_DATAENGINE_H

#include <Plasma/DataEngine>
#include <KPluginMetaData>

class PotdProvider;

class QTimer;

/**
 * This class provides the Pictures of The Day from various online websites.
 *
 * The query keys have the following structure:
 *   \<potd_identifier\>:\<date\>[:other_args]
 * e.g.
 *   apod:2007-07-19
 *   unsplash:12435322
 *
 */
class PotdEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        PotdEngine( QObject* parent, const QVariantList& args );
        ~PotdEngine() override;

    protected:
        bool sourceRequestEvent( const QString &identifier ) override;

    protected Q_SLOTS:
        bool updateSourceEvent( const QString &identifier ) override;

    private Q_SLOTS:
        void finished( PotdProvider* );
        void error( PotdProvider* );
        void checkDayChanged();
        void cachingFinished( const QString &source, const QString &path, const QImage &img );

    private:
        bool updateSource( const QString &identifier, bool loadCachedAlways );

        QMap<QString, KPluginMetaData> mFactories;
        QTimer *m_checkDatesTimer;
        bool m_canDiscardCache;
};

#endif
