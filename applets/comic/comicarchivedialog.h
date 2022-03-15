/*
 *   SPDX-FileCopyrightText: 2011 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMIC_ARCHIVE_DIALOG_H
#define COMIC_ARCHIVE_DIALOG_H

#include "comicinfo.h"
#include "engine/types.h"

#include "ui_comicarchivedialog.h"

#include <QDialog>

class ComicArchiveDialog : public QDialog
{
    Q_OBJECT

public:
    ComicArchiveDialog(const QString &pluginName,
                       const QString &comicName,
                       IdentifierType identifierType,
                       const QString &currentIdentifierSuffix,
                       const QString &firstIdentifierSuffix,
                       const QString &savingDir = QString(),
                       QWidget *parent = nullptr);

Q_SIGNALS:
    void archive(int archiveType, const QUrl &dest, const QString &fromIdentifier, const QString &toIdentifier);

private Q_SLOTS:
    void archiveTypeChanged(int newType);
    void fromDateChanged(const QDate &newDate);
    void toDateChanged(const QDate &newDate);
    void slotOkClicked();
    void updateOkButton();

private:
    void setFromVisible(bool visible);
    void setToVisibile(bool visible);

private:
    Ui::ComicArchiveDialog ui;
    IdentifierType mIdentifierType;
    QString mPluginName;
};

#endif
