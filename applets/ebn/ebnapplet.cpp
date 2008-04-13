/*
 * Copyright (C) 2007 by Alex Merry <alex.merry@kdemail.net>
 *
 * Thanks to the following for the Dict applet, which helped me with this
 *   Thomas Georgiou <TAGeorgiou@gmail.com>
 *   Jeff Cooper <weirdsox11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ebnapplet.h"

#include <QPainter>
#include <QWidget>

#include <KToolInvocation>
#include <KColorScheme>

#include <Plasma/Theme>

using namespace Plasma;

EbnApplet::EbnApplet(QObject *parent, const QVariantList &args)
    : Applet(parent, args),
      m_viewEdit(0),
      watchSource('/')
{
    resize(400,400);
}

EbnApplet::~EbnApplet()
{
}

void EbnApplet::init()
{
    m_viewEdit = new Plasma::LineEdit(this);
    connect(m_viewEdit, SIGNAL(linkActivated(const QString&)),
            this, SLOT(doLink(const QString&)));

    m_viewEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_viewEdit->setFont(Plasma::Theme::self()->font());
    m_viewEdit->setDefaultTextColor(Plasma::Theme::self()->textColor());
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View,
                             Plasma::Theme::self()->colors());
    link = "<a href=\"%2\"><font color=\"%1\">%3</font></a>";
    link = link.arg(colorScheme.foreground(KColorScheme::LinkText).color().name());
    htmlHeader = "<html><body><h1>%1</h1>";
    htmlFooter = "</body></html>";

    // Load the default page
    go();
}

void EbnApplet::dataUpdated(const QString& source,
                            const Plasma::DataEngine::Data& data)
{
    if ( source != watchSource ) {
        kDebug() << "Source was" << source << "but we wanted" << watchSource;
        // ignore all other sources
        return;
    }

    QString title( data.value("title").toString() );
    //QString weblink( data.value("link").toString() ); // browser link
    QVariantMap items( data.value("items").toMap() );

    QString content = "<ul>";

    QMapIterator<QString, QVariant> it(items);
    while (it.hasNext()) {
        it.next();
        QVariantMap map = it.value().toMap();
        content += "<li>";
        if ( map.contains("source") ) {
            content += link.arg(map["source"].toString()).arg(it.key());
        } else {
            content += link.arg(map["link"].toString()).arg(it.key());
        }
        content += ": " + i18np("%1 issue", "%1 issues", map["issues"].toString().toInt()) + "</li>";
    }
    content += "</ul>";

    QString parentSource(source.section('/', 0, -2));
    if ( parentSource.isEmpty() ) {
        parentSource = "/";
    }

    content += "<p>" + link.arg(parentSource).arg(i18n("Back")) + "</p>";
    kDebug() << "parentSource:" << parentSource;
    m_viewEdit->setHtml(htmlHeader.arg(title) + content + htmlFooter);

    updateGeometry();
}

void EbnApplet::doLink(const QString& source)
{
    if ( source.contains("://") ) {
        KToolInvocation::invokeBrowser(source);
    } else {
        go(source);
    }
}

void EbnApplet::go(const QString& source)
{
    if ( watchSource == "/" ) {
        dataEngine("ebn")->disconnectSource(watchSource, this);
    }
    if ( source == "/" ) {
        QString content = "<ul>";
        content += "<li>" + link.arg("apidocs").arg(i18n("API Documentation")) + "</li>";
        content += "<li>" + link.arg("krazy").arg(i18n("Code Checking")) + "</li>";
        content += "<li>" + link.arg("sanitizer").arg(i18n("Documentation Sanitizer")) + "</li>";
        content += "<li>" + link.arg("unittests").arg(i18n("Unit Testing")) + "</li>";
        content += "</ul>";
        m_viewEdit->setHtml(htmlHeader.arg("English Breakfast Network") +
                            content + htmlFooter);
    } else {
        m_viewEdit->setHtml(htmlHeader.arg(i18n("Loading...")) +
                            "<p>" +
                            link.arg(watchSource).arg(i18n("Cancel")) +
                            "</p>" +
                            htmlFooter);
        dataEngine("ebn")->connectSource(source, this);
    }
    watchSource = source;
    updateGeometry();
}

#include "ebnapplet.moc"
