/*
 *   SPDX-FileCopyrightText: 2011 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicarchivedialog.h"
#include "comicarchivejob.h"


ComicArchiveDialog::ComicArchiveDialog( const QString &pluginName, const QString &comicName, IdentifierType identifierType, const QString &currentIdentifierSuffix, const QString &firstIdentifierSuffix, const QString &savingDir, QWidget *parent )
  : QDialog( parent ),
    mIdentifierType( identifierType ),
    mPluginName( pluginName )
{
    ui.setupUi(this);
    setWindowTitle(i18nc("@title:window", "Create %1 Comic Book Archive", comicName));

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &ComicArchiveDialog::slotOkClicked);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &ComicArchiveDialog::reject);

    switch ( mIdentifierType ) {
        case Date: {
            const QDate current = QDate::fromString(currentIdentifierSuffix, QStringLiteral("yyyy-MM-dd"));
            const QDate first = QDate::fromString(firstIdentifierSuffix, QStringLiteral("yyyy-MM-dd"));
            const QDate today = QDate::currentDate();
            QDate maxDate = today;
            if ( current.isValid() ) {
                ui.fromDate->setDate( current );
                ui.toDate->setDate( current );
                maxDate = ( today > current ? today : current );
            }
            if ( first.isValid() ) {
                ui.fromDate->setMinimumDate( first );
                ui.toDate->setMinimumDate( first );
            }

            connect( ui.fromDate, &QDateTimeEdit::dateChanged, this, &ComicArchiveDialog::fromDateChanged );
            connect( ui.toDate, &QDateTimeEdit::dateChanged, this, &ComicArchiveDialog::toDateChanged );
            break;
        }
        case Number: {
            bool ok;
            const int current = currentIdentifierSuffix.toInt( &ok );
            if ( ok ) {
                ui.fromNumber->setValue( current );
                ui.toNumber->setValue( current );
            }
            const int first = firstIdentifierSuffix.toInt( &ok );
            if ( ok ) {
                ui.fromNumber->setMinimum( first );
                ui.toNumber->setMinimum( first );
            }
            break;
        }
        case String: {
            ui.fromString->setText( currentIdentifierSuffix );
            ui.toString->setText( currentIdentifierSuffix );
            connect( ui.fromString, &QLineEdit::textEdited, this, &ComicArchiveDialog::updateOkButton );
            connect( ui.toString, &QLineEdit::textEdited, this, &ComicArchiveDialog::updateOkButton );
            break;
        }
    }

    ui.types->setCurrentIndex( mIdentifierType );

    archiveTypeChanged( ComicArchiveJob::ArchiveAll );

    //TODO suggest file name!
    ui.dest->setAcceptMode(QFileDialog::AcceptSave);
    if ( !savingDir.isEmpty() ) {
        ui.dest->setStartDir(QUrl::fromLocalFile(savingDir));
    }

    connect( ui.archiveType, SIGNAL(currentIndexChanged(int)), this, SLOT(archiveTypeChanged(int)) );
    connect( ui.dest, &KUrlRequester::textChanged, this, &ComicArchiveDialog::updateOkButton );
    connect( this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()) );
}

void ComicArchiveDialog::archiveTypeChanged( int newType )
{
    switch ( newType ) {
        case ComicArchiveJob::ArchiveAll:
            setFromVisible( false );
            setToVisibile( false );
            break;
            break;
        case ComicArchiveJob::ArchiveEndTo:
        case ComicArchiveJob::ArchiveStartTo:
            setFromVisible( false );
            setToVisibile( true );
            break;
        case ComicArchiveJob::ArchiveFromTo:
            setFromVisible( true );
            setToVisibile( true );
            break;
    }

    updateOkButton();
}

void ComicArchiveDialog::fromDateChanged( const QDate &newDate )
{
    if ( ui.toDate->date() < newDate ) {
        ui.toDate->setDate( newDate );
    }
    updateOkButton();
}

void ComicArchiveDialog::toDateChanged( const QDate &newDate )
{
    if ( ui.fromDate->date() > newDate ) {
        ui.fromDate->setDate( newDate );
    }
    updateOkButton();
}

void ComicArchiveDialog::updateOkButton()
{
    const int archiveType = ui.archiveType->currentIndex();
    bool okEnabled = true;

    //string is handled here, as it is the only identifier which can be invalid (empty)
    if ( mIdentifierType == String ) {
        if ( archiveType == ComicArchiveJob::ArchiveAll ) {
            okEnabled = true ;
        } else if ( ui.archiveType->currentIndex() == ComicArchiveJob::ArchiveFromTo ) {
            okEnabled = !ui.fromString->text().isEmpty() && !ui.toString->text().isEmpty();
        } else {
            okEnabled = !ui.toString->text().isEmpty();
        }
    }

    okEnabled = ( okEnabled && !ui.dest->url().isEmpty() );
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(okEnabled);
}

void ComicArchiveDialog::slotOkClicked()
{
    const int archiveType = ui.archiveType->currentIndex();
    QString fromIdentifier;
    QString toIdentifier;

    switch ( mIdentifierType ) {
        case Date:
            fromIdentifier = ui.fromDate->date().toString(QStringLiteral("yyyy-MM-dd"));
            toIdentifier = ui.toDate->date().toString(QStringLiteral("yyyy-MM-dd"));
            break;
        case Number: {
            fromIdentifier = QString::number( ui.fromNumber->value() );
            toIdentifier = QString::number( ui.toNumber->value() );
            //the user entered from and to wrong, swap them
            if ( ( archiveType == ComicArchiveJob::ArchiveFromTo) && ( ui.toNumber->value() < ui.fromNumber->value() ) ) {
                QString temp = fromIdentifier;
                fromIdentifier = toIdentifier;
                toIdentifier = temp;
            }
            break;
        }
        case String:
            fromIdentifier = ui.fromString->text();
            toIdentifier = ui.toString->text();
            break;
    }

    emit archive( archiveType, ui.dest->url(), fromIdentifier, toIdentifier );
    accept();
}

void ComicArchiveDialog::setFromVisible( bool visible )
{
    ui.fromDateLabel->setVisible( visible );
    ui.fromDate->setVisible( visible );
    ui.fromNumberLabel->setVisible( visible );
    ui.fromNumber->setVisible( visible );
    ui.fromStringLabel->setVisible( visible );
    ui.fromString->setVisible( visible );
}

void ComicArchiveDialog::setToVisibile( bool visible )
{
    ui.toDateLabel->setVisible( visible );
    ui.toDate->setVisible( visible );
    ui.toNumberLabel->setVisible( visible );
    ui.toNumber->setVisible( visible );
    ui.toStringLabel->setVisible( visible );
    ui.toString->setVisible( visible );
}
