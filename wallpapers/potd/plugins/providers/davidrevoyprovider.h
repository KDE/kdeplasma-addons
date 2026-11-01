/*
 *   SPDX-FileCopyrightText: 2026 Guillermo Villafuerte <me@strongville.dev>
 *
 *   SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 */

#ifndef DAVIDREVOYPROVIDER_H
#define DAVIDREVOYPROVIDER_H

#include "potdprovider.h"

#include <KJob>

/**
 * Picture-of-the-Day provider for David Revoy's wallpapers, available royalty-free at:
 * https://www.peppercarrot.com/en/wallpapers/index.html
 */
class DavidRevoyProvider : public PotdProvider
{
    Q_OBJECT

public:
    explicit DavidRevoyProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

private:
    void metaDataRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);
    void checksumCalculationFinished(KJob *job);

    QByteArray m_imageData;
    QString m_checksum;
};

/**
 * Calculates the CRC32 checksum of a buffer, reading it in fixed-size chunks.
 */
class ChecksumJob : public KJob
{
    Q_OBJECT

public:
    explicit ChecksumJob(const QByteArray &data, QObject *parent = nullptr);
    quint32 checksum() const;
    void start() override;

private:
    void calculateChecksum();

    static constexpr qsizetype ChecksumChunkSize = 64 * 1024;

    QByteArray m_data;
    quint32 m_checksum = 0;
};

#endif
