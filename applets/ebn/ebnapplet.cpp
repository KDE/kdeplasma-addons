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

using namespace Plasma;

EbnApplet::EbnApplet(QObject* parent, const QVariantList &args)
    : Applet( parent, args )
    , m_viewEdit( new Plasma::LineEdit(this) )
    , watchSource('/')
{
    setHasConfigurationInterface(false);
    setDrawStandardBackground(true);

    connect(m_viewEdit, SIGNAL(linkActivated(const QString&)),
            this, SLOT(doLink(const QString&)));

    m_viewEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);

    // Load the default page
    go();
}

EbnApplet::~EbnApplet()
{
}

QSizeF EbnApplet::contentSizeHint() const
{
    return m_viewEdit->geometry().size();
}

void EbnApplet::constraintsUpdated(Plasma::Constraints)
{
    prepareGeometryChange();
}

void EbnApplet::updated(const QString& source,
                        const Plasma::DataEngine::Data& data)
{
    prepareGeometryChange();
    if ( source != watchSource ) {
        // ignore all other sources
        return;
    }

    QString title( data.value("title").toString() );
    //QString weblink( data.value("link").toString() ); // browser link
    QVariantMap items( data.value("items").toMap() );

    QString content = "<html><h1>" + title + "</h1><ul>";

    QMapIterator<QString, QVariant> it(items);
    while (it.hasNext()) {
        it.next();
        QVariantMap map = it.value().toMap();
        content += "<li>";
        if ( map.contains("source") ) {
            content += "<a href=\"" + map["source"].toString() + "\">"
                       + it.key() + "</a>";
        } else {
            content += "<a href=\"" + map["link"].toString() + "\">"
                       + it.key() + "</a>";
        }
        content += ": " + i18np("%1 issue", "%1 issues", map["issues"].toString().toInt()) + "</li>";
    }
    content += "</ul>";

    QString parentSource(source.section('/', 0, -2));
    if ( parentSource.isEmpty() ) {
        parentSource = "/";
    }
    
    content += "<p><a href=\"" + parentSource + "\">Back</a></p></html>";
    kDebug()<<"parentSource :"<<parentSource<<endl;
    m_viewEdit->setHtml(content);

    update();
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
    prepareGeometryChange();
    if ( watchSource == "/" ) {
        dataEngine("ebn")->disconnectSource(watchSource, this);
    }
    if ( source == "/" ) {
        m_viewEdit->setHtml(QString("<html>"
                              "<h1>English Breakfast Network</h1>"
                              "<ul>"
                                "<li><a href=\"apidocs\">%1</a></li>"
                                "<li><a href=\"krazy\">%2</a></li>"
                                "<li><a href=\"sanitizer\">%3</a></li>"
                                "<li><a href=\"unittests\">%4</a></li>"
                              "</ul>"
                            "</html>").arg( i18n("API Documentation")).arg(i18n("Code Checking")).arg(i18n("Documentation Sanitizer")).arg(i18n("Unit Testing")));
    } else {
        m_viewEdit->setHtml(QString("<html>"
                              "<h1>%1</h1>"
                              "<p><a href=\"%2\">%3</a></p>"
                            "</html>").arg(i18n("Loading...")).arg(watchSource).arg(i18n("Cancel")));
        dataEngine("ebn")->connectSource(source, this);
    }
    watchSource = source;
    update();
}

void EbnApplet::paintInterface(QPainter* p,
                               const QStyleOptionGraphicsItem* option,
                               const QRect& rect)
{
    Q_UNUSED(option);
    Q_UNUSED(rect);

    p->setRenderHint(QPainter::Antialiasing);
}

#include "ebnapplet.moc"
