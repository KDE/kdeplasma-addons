/***************************************************************************
 *   Copyright (C) 2009 by Sebastian K?gler <sebas@kde.org>                *
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

#include "webslice.h"

#include <kgraphicswebslice.h>
#include <limits.h>

// Qt
#include <QGraphicsSceneResizeEvent>
#include <QLabel>

#include <QGraphicsLinearLayout>

// KDE
#include <KConfigDialog>
#include <KServiceTypeTrader>

// Plasma
#include <Plasma/Label>

WebSlice::WebSlice(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
    m_slice(0),
    m_url(0),
    m_size(192, 192)
{
    setPopupIcon("internet-web-browser");
    setAspectRatioMode(Plasma::IgnoreAspectRatio );
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);

    setMinimumSize(64, 64);
    resize(300, 300);
}

void WebSlice::init()
{
    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName());
    const KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet",
                                                                    constraint);
    foreach (const KService::Ptr &service, offers) {
        QStringList args = service->property("X-Plasma-Args").toStringList();
        if (args.count() >= 1) {
            m_url = KUrl(args[0]);
            if (args.count() >= 2) {
                m_element = args[1];
                if (args.count() >= 6) {
                    m_sliceGeometry = QRectF(args[2].toInt(), args[3].toInt(),
                                            args[4].toInt(), args[5].toInt());
                }
            }
        }
    }

    KConfigGroup cg = config();
    if (!m_url.isValid() || m_url.isEmpty()) {
        m_url = cg.readEntry("url", "http://dot.kde.org/");
        m_element = cg.readEntry("element", "#block-views-planetkde-block_1");

        // for testing geometry
        //m_url = cg.readEntry("url", "http://buienradar.nl/");
        //m_sliceGeometry = cg.readEntry("size", QRectF(258, 102, 550, 511));
        m_sliceGeometry = cg.readEntry("sliceGeometry", QRectF());
    }
    m_size = cg.readEntry("size", m_size);
    setAssociatedApplicationUrls(KUrl::List(m_url));

    //kDebug() << "url/element/slicegeometry:" << m_url << m_element << m_sliceGeometry;
}

WebSlice::~WebSlice ()
{
}

QGraphicsWidget* WebSlice::graphicsWidget()
{
    if (!m_slice) {
        m_widget = new QGraphicsWidget(this);
        QGraphicsLinearLayout *l = new QGraphicsLinearLayout(m_widget);
        m_widget->setLayout(l);


        m_slice = new KGraphicsWebSlice(m_widget);
        connect(m_slice, SIGNAL(sizeChanged(QSizeF)), this, SLOT(sizeChanged(QSizeF)));
        connect(m_slice, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        setBusy(true);
        m_slice->setLoadingText(i18nc("displayed in the widget while loading", "<h1>Loading...</h1>"));
        m_slice->setUrl(m_url);
        m_slice->setElement(m_element);
        m_slice->setSliceGeometry(m_sliceGeometry);
        m_slice->hide();
        l->addItem(m_slice);
    }
    return m_widget;
}

void WebSlice::createConfigurationInterface(KConfigDialog *parent)
{
    QLabel *info = new QLabel(parent);
    info->setWordWrap(true);
    info->setTextInteractionFlags(Qt::TextBrowserInteraction);
    info->setText(i18n("<p>The Webslice Widget allows you to display a part of a webpage on your desktop or in a panel. The webslice is fully interactive.</p>"
    "<p>"
    "Specify the URL of the webpage in the URL field."
    "In the <em>Element to Show</em> field, fill in a CSS identifier (for example #mybox for elements with the id \"mybox\").  This is the preferred method as it works best with layout changes on the webpage."
    "</p>"
    "<p>"
    "Alternatively, you can specify a rectangle on the webpage to use as slice. Use \"x,y,width,height\" in pixels, for example <em>\"100,80,300,360\"</em>. This method is a fallback for webpages that do not provide enough semantic markup for the above mechanism."
    "</p>"
    "If both methods (element and geometry) are used, the element will take precedence."));
    parent->addPage(info, i18nc("informational page", "Info"), "help-hint");

    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18nc("general config page", "Webpage"), Applet::icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    ui.urlEdit->setText(m_url.toString());
    ui.geometryEdit->setText(sliceGeometryToString());
    ui.elementEdit->setText(m_element);
}

void WebSlice::configAccepted()
{
    if (m_url.toString() != ui.urlEdit->text() ||
        m_element != ui.elementEdit->text() ||
        ui.geometryEdit->text() != sliceGeometryToString()) {

        m_url = QUrl(ui.urlEdit->text());
        m_element = ui.elementEdit->text();

        QString geo = ui.geometryEdit->text();
        QStringList gel = geo.split(',');
        qreal x, y, w, h;
        bool ok = true;
        if (gel.length() == 4) {
            x = (qreal)(gel[0].toDouble(&ok));
            y = (qreal)(gel[1].toDouble(&ok));
            w = (qreal)(gel[2].toDouble(&ok));
            h = (qreal)(gel[3].toDouble(&ok));
            if (!ok) {
                kWarning() << "a conversion error occurred." << gel;
            } else {
                m_sliceGeometry = QRectF(x, y, w, h);
                config().writeEntry("sliceGeometry", m_sliceGeometry);
                //kDebug() << "new slice geometry:" << m_sliceGeometry;
            }
        } else {
            if (m_element.isEmpty()) {
                kWarning() << "format error, use x,y,w,h" << gel << gel.length();
            }
        }

        m_slice->setUrl(m_url);
        setAssociatedApplicationUrls(KUrl::List(m_url));
        m_slice->setElement( m_element );
        m_slice->hide();

        KConfigGroup cg = config();
        cg.writeEntry("url", m_url.toString());
        cg.writeEntry("element", m_element);
        if (!m_element.isEmpty()) {
            m_sliceGeometry = QRectF();
        }
        emit configNeedsSaving();
        //kDebug() << "config changed" << m_element << m_url;
    }
}

QString WebSlice::sliceGeometryToString()
{
    QString s = QString("%1,%2,%3,%4").arg(m_sliceGeometry.x()).arg(m_sliceGeometry.y()).arg(m_sliceGeometry.width()).arg(m_sliceGeometry.height());
    return s;
}


void WebSlice::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & (Plasma::FormFactorConstraint | Plasma::SizeConstraint)) {
        //kDebug() << "Constraint changed:" << mapToScene(contentsRect());
        if (m_slice) {
            //kDebug() << "resizing slice to:" << contentsRect().size();
            m_slice->setMaximumSize(contentsRect().size());
            m_widget->setMinimumSize(64, 64);
            m_slice->refresh();
        }
    }
}

void WebSlice::loadFinished(bool ok)
{
    setBusy(false);

    if (!ok) {
      // TODO set error image
      kDebug() << "loading failed, but at least we know...";
      return;
    }

    m_slice->show();
    m_size = m_slice->geometry().size();

    setAspectRatioMode(Plasma::KeepAspectRatio );

    kDebug() << "done loading, resizing slice to:" << contentsRect().size();
    m_slice->setMaximumSize(contentsRect().size());
}

void WebSlice::sizeChanged(QSizeF newsize)
{
    if (m_slice && m_size != newsize) {
        QSizeF m_size = QSizeF(newsize.width() + 28, newsize.height() + 28);
        m_slice->resize(m_size);

        QRectF g = QRectF(mapToScene(contentsRect().topLeft()), m_size);
        mapFromScene(contentsRect().topLeft());

        m_widget->setMinimumSize(m_size);
        m_slice->setMaximumSize(contentsRect().size());
        setPreferredSize(m_size);
        kDebug() << "size is now:" << m_size;
        KConfigGroup cg = config();
        cg.writeEntry("size", m_size);
        emit configNeedsSaving();
    }
}

#include "webslice.moc"
