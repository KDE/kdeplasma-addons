/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Farhad Hedayati-Fard <hf.farhad@gmail.com>
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "translator.h"
#include <QtDebug>
#include <KIO/Job>
#include <KStandardDirs>
#include <Plasma/Package>
#include <kross/core/interpreter.h>

Translator::Translator(QWidget* parent, QString from , QString to, QString pluginName): QObject(parent)
{
    const QString path = KStandardDirs::locate( "data", QLatin1String( "plasma/translators/" ) + pluginName + QLatin1Char( '/' ) );
    qDebug() << path;
    if (!path.isEmpty()) {
        m_package = new Plasma::Package(path, packageStructure());
        if (m_package->isValid()) {
            const QString mainscript = m_package->path() + m_package->structure()->contentsPrefix() +
            m_package->structure()->path( "mainscript" );
            QFileInfo info( mainscript );
            QStringList extensions = supportedScriptLangs();
            qDebug() << extensions;
            for ( int i = 0; i < extensions.count() && !info.exists(); ++i ) {
                info.setFile( mainscript + extensions.value( i ) );
                if (info.exists()) break;
            }
            qDebug() << info.filePath();
            if ( info.exists() ) {
                m_action = new Kross::Action(this, pluginName);
                m_action->setFile(info.filePath());
                
                m_action->trigger();
            }
        }
    }
    connect(this, SIGNAL(retriveError(QString)), this, SLOT(retrivalError(QString)));
    connect(this, SIGNAL(error(QString)), this, SLOT(translationError(QString)));
    m_from = from;
    m_to = to;
    m_pluginName = pluginName;
}

void Translator::translate(QString origText)
{
    QVariantList list;
    list << origText;
    list << m_from;
    list << m_to;
    QString url = m_action->callFunction("getUrl", list).toString();
    qDebug() << "Translator: " << url;
    retrivePage(url);
}

QStringList Translator::supportedLangs()
{
    QVariant var = m_action->callFunction("supportedLangs");
    return var.toStringList();
}

void Translator::retrivePage(QString u)
{
    KUrl url(u.toUtf8());
    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( pageRetrived( KJob* ) ) );
}

void Translator::pageRetrived(KJob* job)
{
    if (job->error()) {
        emit retrivalError(job->errorString());
        emit error(job->errorString());
    }
    else {
        KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob*>( job );
        QString page ;
        page = page.fromUtf8(storedJob->data());
        
        QVariantList l;
        l << page;
        QString translatedString = m_action->callFunction("parsePage", l).toString();
        qDebug() << translatedString;
        translatedString.toUtf8();
        emit translated(translatedString);
    }
}


void Translator::retrivalError(QString error)
{
    qDebug() << "Retrive error " << error ;
}

void Translator::translationError(QString error)
{
    qDebug() << "Translation error " << error;
}

void Translator::setFrom(QString from)
{
    m_from = from;
}

void Translator::setTo(QString to)
{
    m_to = to;
}

QString Translator::pluginName()
{
    return m_pluginName;
}

Plasma::PackageStructure::Ptr Translator::packageStructure() {
    if ( !m_packageStructure ) {
        m_packageStructure = new TranslatorPackage();
    }
    return m_packageStructure;
}

QStringList Translator::supportedScriptLangs()
{
    QStringList l;
    Kross::InterpreterInfo* info;
    QString wildcards;
    
    foreach( const QString &interpretername, Kross::Manager::self().interpreters() ) {
        info = Kross::Manager::self().interpreterInfo( interpretername );
        wildcards = info->wildcard();
        wildcards.remove( QLatin1Char( '*' ) );
        l << wildcards.split( QLatin1Char( ' ' ) );
    }
    return l;
}

