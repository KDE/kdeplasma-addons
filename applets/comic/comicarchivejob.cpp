/***************************************************************************
 *   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "comicarchivejob.h"

#include <KDebug>
#include <KIO/NetAccess>
#include <KTemporaryFile>
#include <KZip>

#ifdef HAVE_NEPOMUK
#include <Nepomuk/Resource>
#include <Nepomuk/Tag>
#include <Nepomuk/Variant>
#include <Nepomuk/Vocabulary/NCO>
#include <Nepomuk/Vocabulary/NFO>
#include <Nepomuk/Vocabulary/PIMO>

using namespace Nepomuk::Vocabulary;
#endif

ComicArchiveJob::ComicArchiveJob( const KUrl &dest, Plasma::DataEngine *engine, ComicArchiveJob::ArchiveType archiveType, IdentifierType identifierType, const QString &pluginName, QObject *parent )
  : KJob( parent ),
    mType( archiveType ),
    mDirection( Undefined ),
    mIdentifierType( identifierType ),
    mSuspend( false ),
    mFindAmount( true ),
    mHasVariants( false ),
    mDone( false ),
    mComicNumber( 0 ),
    mProcessedFiles( 0 ),
    mTotalFiles( -1 ),
    mEngine( engine ),
    mZipFile( new KTemporaryFile ),
    mZip( 0 ),
    mPluginName( pluginName ),
    mDest( dest )
{
    if ( mZipFile->open() ) {
        mZip = new KZip( mZipFile->fileName() );
        mZip->open( QIODevice::ReadWrite );
        mZip->setCompression( KZip::NoCompression );
        setCapabilities( Killable | Suspendable );
    } else {
        kError() << "Could not create a temporary file for the zip file.";
    }
}

ComicArchiveJob::~ComicArchiveJob()
{
    emitResultIfNeeded();
    delete mZip;
    delete mZipFile;
    qDeleteAll( mBackwardFiles );
}

bool ComicArchiveJob::isValid() const
{
    if ( mPluginName.isEmpty() ) {
        kWarning() << "No plugin name specified.";
        return false;
    }

    switch ( mType ) {
        case ArchiveFromTo:
            if ( mToIdentifier.isEmpty() || mFromIdentifier.isEmpty() ) {
                kWarning() << "Not enought data provided to archive a range.";
                return false;
            }
            break;
        case ArchiveStartTo:
        case ArchiveEndTo:
            if ( mToIdentifier.isEmpty() ) {
                kWarning() << "Not enough data provied to archive StartTo/EndTo.";
                return false;
            }
            break;
        default:
            break;
    }

    return mEngine->isValid() && mZip && mZip->isOpen();
}

void ComicArchiveJob::setToIdentifier( const QString &toIdentifier )
{
    mToIdentifier = toIdentifier;
    mToIdentifierSuffix = mToIdentifier;
    mToIdentifierSuffix.remove( mPluginName + ':' );
}

void ComicArchiveJob::setFromIdentifier( const QString &fromIdentifier )
{
    mFromIdentifier = fromIdentifier;
    mFromIdentifierSuffix = mFromIdentifier;
    mFromIdentifierSuffix.remove( mPluginName + ':' );
}

void ComicArchiveJob::start()
{
    switch ( mType ) {
        case ArchiveAll:
            requestComic( suffixToIdentifier( QString() ) );
            break;
        case ArchiveStartTo:
            requestComic( mToIdentifier );
            break;
        case ArchiveEndTo: {
            setFromIdentifier( mToIdentifier );
            mToIdentifier.clear();
            mToIdentifierSuffix.clear();
            requestComic( suffixToIdentifier( QString() ) );
            break;
        }
        case ArchiveFromTo:
            mDirection = Foward;
            defineTotalNumber();
            requestComic( mFromIdentifier );
            break;
    }
}

void ComicArchiveJob::dataUpdated( const QString &source, const Plasma::DataEngine::Data &data )
{
    if ( !mZip ) {
        kWarning() << "No zip file, aborting.";
        setErrorText( i18n( "No zip file is existing, aborting." ) );
        setError( KilledJobError );
        emitResultIfNeeded();
        return;
    }

    const QString currentIdentifier = data[ "Identifier" ].toString();
    QString currentIdentifierSuffix = currentIdentifier;
    currentIdentifierSuffix.remove( mPluginName + ':' );

    const QImage image = data[ "Image" ].value<QImage>();
    const bool hasError = data[ "Error" ].toBool() || image.isNull();
    const QString previousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();
    const QString nextIdentifierSuffix = data[ "Next identifier suffix" ].toString();
    const QString firstIdentifierSuffix = data[ "First strip identifier suffix" ].toString();

    mAuthors << data[ "Comic Author" ].toString().split( ',', QString::SkipEmptyParts );
    mAuthors.removeDuplicates();

    if ( mComicTitle.isEmpty() ) {
        mComicTitle = data[ "Title" ].toString();
    }

    mEngine->disconnectSource( source, this );

    if ( hasError ) {
        kWarning() << "An error occured at" << source << "stopping.";
        setErrorText( i18n( "An error happened for identifier %1.", source ) );
        setError( KilledJobError );
        copyZipFileToDestination();
        return;
    }

    if ( mDirection == Undefined ) {
        if ( ( mType == ArchiveAll ) || ( mType == ArchiveStartTo ) ) {
            if ( !firstIdentifierSuffix.isEmpty() ) {
                setFromIdentifier( suffixToIdentifier( firstIdentifierSuffix ) );
            }
            if ( mType == ArchiveAll ) {
                setToIdentifier( currentIdentifier );
            }
            mDirection = ( firstIdentifierSuffix.isEmpty() ? Backward : Foward );
            if ( mDirection == Foward ) {
                requestComic( suffixToIdentifier( firstIdentifierSuffix ) );
                return;
            } else {
                //backward, i.e. the to identifier is unknown
                mToIdentifier.clear();
                mToIdentifierSuffix.clear();
            }
        } else if ( mType == ArchiveEndTo ) {
            mDirection = Foward;
            setToIdentifier( currentIdentifier );
            requestComic( mFromIdentifier );
            return;
        }
    }

    bool worked = false;
    ++mProcessedFiles;
    if ( mDirection == Foward ) {
        KTemporaryFile tempFile;
        worked = tempFile.open();
        worked = worked && tempFile.flush();
        worked = ( worked ? image.save( tempFile.fileName(), "PNG" ) : worked );
        worked = ( worked ? addFileToZip( tempFile.fileName() ) : worked );

        if ( worked ) {
            if ( ( currentIdentifier == mToIdentifier ) || ( currentIdentifierSuffix == nextIdentifierSuffix) || nextIdentifierSuffix.isEmpty() ) {
                kDebug() << "Done downloading at:" << source;
                copyZipFileToDestination();
            } else {
                requestComic( suffixToIdentifier( nextIdentifierSuffix ) );
            }
        }
    } else if ( mDirection == Backward ) {
        KTemporaryFile *tempFile = new KTemporaryFile;
        mBackwardFiles << tempFile;
        worked = tempFile->open();
        worked = worked && tempFile->flush();
        worked = ( worked ? image.save( tempFile->fileName(), "PNG" ) : worked );

        if ( worked ) {
            if ( ( currentIdentifier == mToIdentifier ) || ( currentIdentifierSuffix == previousIdentifierSuffix ) || previousIdentifierSuffix.isEmpty() ) {
                kDebug() << "Done downloading at:" << source;
                createBackwardZip();
            } else {
                requestComic( suffixToIdentifier( previousIdentifierSuffix) );
            }
        }
    }

    defineTotalNumber( currentIdentifierSuffix );
    setProcessedAmount( Files, mProcessedFiles );
    if ( mTotalFiles != -1 ) {
        setPercent( ( 100 * mProcessedFiles ) / mTotalFiles );
    }

    if ( !worked ) {
        kError() << "Could not write the file, identifier:" << source;
        setErrorText( i18n( "Failed creating the file with identifier %1.", source ) );
        setError( KilledJobError );
        emitResultIfNeeded();
    }
}

bool ComicArchiveJob::doKill()
{
    mSuspend = true;
    return KJob::doKill();
}

bool ComicArchiveJob::doSuspend()
{
    mSuspend = true;
    return true;
}

bool ComicArchiveJob::doResume()
{
    mSuspend = false;
    if ( !mRequest.isEmpty() ) {
        requestComic( mRequest );
    }
    return true;
}

void ComicArchiveJob::defineTotalNumber( const QString &currentSuffix )
{
    findTotalNumberFromTo();
    if ( mTotalFiles == -1 ) {
        kDebug() << "Unable to find the total number for" << mPluginName;
        return;
    }

    //calculate a new value for total files, can be different from the previous one,
    //if there are no strips for certain days/numbers
    if ( !currentSuffix.isEmpty() ) {
        if ( mIdentifierType == Date ) {
            const QDate current = QDate::fromString( currentSuffix, "yyyy-MM-dd" );
            const QDate to = QDate::fromString( mToIdentifierSuffix, "yyyy-MM-dd" );
            if ( current.isValid() && to.isValid() ) {
                //processed files + files still to download
                mTotalFiles = mProcessedFiles + qAbs( current.daysTo( to ) );
            }
        } else if ( mIdentifierType == Number ) {
            bool result = true;
            bool ok;
            const int current = currentSuffix.toInt( &ok );
            result = ( result && ok );
            const int to = mToIdentifierSuffix.toInt( &ok );
            result = ( result && ok );
            if ( result ) {
                //processed files + files still to download
                mTotalFiles = mProcessedFiles + qAbs( to - current );
            }
        }
    }

    if ( mTotalFiles != -1 ) {
        setTotalAmount( Files, mTotalFiles );
    }
}

void ComicArchiveJob::findTotalNumberFromTo()
{
    if ( mTotalFiles != -1 ) {
        return;
    }

    if ( mIdentifierType == Date ) {
        const QDate from = QDate::fromString( mFromIdentifierSuffix, "yyyy-MM-dd" );
        const QDate to = QDate::fromString( mToIdentifierSuffix, "yyyy-MM-dd" );
        if ( from.isValid() && to.isValid() ) {
            mTotalFiles = qAbs( from.daysTo( to ) ) + 1;
        }
    } else if ( mIdentifierType == Number ) {
        bool result = true;
        bool ok;
        const int from = mFromIdentifierSuffix.toInt( &ok );
        result = ( result && ok );
        const int to = mToIdentifierSuffix.toInt( &ok );
        result = ( result && ok );
        if ( result ) {
            mTotalFiles = qAbs( to - from ) + 1;
        }
    }
}

QString ComicArchiveJob::suffixToIdentifier( const QString &suffix ) const
{
    return mPluginName + ':' + suffix;
}

void ComicArchiveJob::requestComic( QString identifier ) //krazy:exclude=passbyvalue
{
    mRequest.clear();
    if ( mSuspend ) {
        mRequest = identifier;
        return;
    }

    emit description( this, i18n( "Creating Comic Book Archive" ),
                      qMakePair( QString( "source" ), identifier ),
                      qMakePair( QString( "destination" ), mDest.prettyUrl() ) );

    mEngine->connectSource( identifier, this );
    mEngine->query( identifier );
}

bool ComicArchiveJob::addFileToZip( const QString &path )
{
    //We use 6 signs, e.g. number 1 --> 000001.png, 123 --> 000123.png
    //this way the comics should always be correctly sorted (otherwise evince e.g. has problems)
    static const int numSigns = 6;
    static const QString zero = QLatin1String( "0" );
    QString number = QString::number( ++mComicNumber );
    const int length = number.length();
    if ( length < numSigns ) {
        number = zero.repeated( numSigns - length ) + number;
    }

    return mZip->addLocalFile( path, number + QLatin1String( ".png" ) );
}

void ComicArchiveJob::createBackwardZip()
{
    for ( int i = mBackwardFiles.count() - 1; i >= 0; --i ) {
        if ( !addFileToZip( mBackwardFiles[i]->fileName() ) ) {
            kWarning() << "Failed adding a file to the archive.";
            setErrorText( i18n( "Failed adding a file to the archive." ) );
            setError( KilledJobError );
            emitResultIfNeeded();
            return;
        }
    }

    copyZipFileToDestination();
}

void ComicArchiveJob::copyZipFileToDestination()
{
    mZip->close();
    const bool worked = KIO::NetAccess::file_copy( KUrl( mZipFile->fileName() ), mDest );
    //store additional data using Nepomuk
    if (!worked) {
        kWarning() << "Could not copy the zip file to the specified destination:" << mDest;
        setErrorText( i18n( "Could not create the archive at the specified location." ) );
        setError( KilledJobError );
        emitResultIfNeeded();
        return;
    }

#ifdef HAVE_NEPOMUK
    //store additional data using Nepomuk
    Nepomuk::Resource res( mDest, NFO::FileDataObject() );

    Nepomuk::Resource comicTopic( "Comic", PIMO::Topic() );
    comicTopic.setLabel( i18n( "Comic" ) );

    if ( !mComicTitle.isEmpty() ) {
        Nepomuk::Resource topic( mComicTitle, PIMO::Topic() );
        topic.setLabel( mComicTitle );
        topic.setProperty( PIMO::superTopic(), comicTopic );
        res.addTag( topic );
    } else {
//             res.addTag( comicTopic );//TODO activate this, see below
        ;
    }

    //FIXME also set the comic topic as tag, this is redundant, as topic has this as super topic
    //though at this point the gui (Dolphin) does not manage to show the correct tags
    res.addTag( comicTopic );

    foreach ( QString author, mAuthors ) {
        author = author.trimmed();
        Nepomuk::Resource authorRes( author, NCO::PersonContact() );
        authorRes.setProperty( NCO::fullname(), author );
        res.addProperty( NCO::creator(), authorRes );
    }
#endif

    emitResultIfNeeded();
}

void ComicArchiveJob::emitResultIfNeeded()
{
    if ( !mDone ) {
        mDone = true;
        emitResult();
    }
}
