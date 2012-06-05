/*
 *   Copyright 2012 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * QOAuth example
 *      http://blog.ayoy.net/2009/6/24/oauth
 *
 * Twitter OAuth Docs:
 *      https://dev.twitter.com/docs/auth/oauth
 *
 * How does OAuth work diagram:
 *      https://dev.twitter.com/sites/default/files/images_documentation/oauth_diagram.png
 *
 *
 */

#include <QUrl>
#include <KRun>
#include <KDialog>

#include <KWebView>
#include <KWebPage>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>

#include "koauthwebhelper.h"
#include <KDebug>

namespace KOAuth {

class KOAuthWebHelperPrivate {

public:
    KOAuthWebHelperPrivate()
    {
        webView = 0;
        dialog = 0;
        busy = false;
    }
    QString user;
    QString password;
    QString serviceBaseUrl;
    QHash<QString, QString> authorizeUrls;

    QWebView *webView;
    KDialog *dialog;
    bool busy;
    QTimer *timer;
};


KOAuthWebHelper::KOAuthWebHelper(QObject* parent)
    : QObject(parent),
      d(new KOAuthWebHelperPrivate)
{
    setObjectName(QLatin1String("QOAuthWebHelper"));
    d->timer = new QTimer();
    d->timer->setInterval(3000);
    d->timer->setSingleShot(true);
    connect(d->timer, SIGNAL(timeout()), SLOT(showDialog()));
}

KOAuthWebHelper::~KOAuthWebHelper()
{
    kDebug();
    delete d;
}

void KOAuthWebHelper::setUser(const QString& user)
{
    d->user = user;
}

void KOAuthWebHelper::setPassword(const QString& password)
{
    d->password = password;
}

void KOAuthWebHelper::setServiceBaseUrl(const QString &url)
{
    d->serviceBaseUrl = url;
}

void KOAuthWebHelper::showDialog()
{
    if (d->dialog) {
        kDebug() << "Showing dialog for URL: " << d->webView->page()->mainFrame()->toHtml();
        d->dialog->show();
    }
}

void KOAuthWebHelper::authorizeApp(const QString &serviceBaseUrl, const QString &authorizeUrl, const QString &pageUrl)
{
    Q_UNUSED(serviceBaseUrl);
    Q_UNUSED(authorizeUrl);
    if (d->serviceBaseUrl == "/" || d->serviceBaseUrl.isEmpty()) return;
    if (!d->webView) {
        d->dialog = new KDialog();
        d->dialog->setCaption( "authorize application" );
        d->dialog->setButtons( KDialog::Ok | KDialog::Cancel);

        //d->webView = new KWebView(d->dialog, KWebPage::KIOIntegration);
        d->webView = new QWebView(d->dialog);
        d->dialog->setMainWidget(d->webView);
//         d->dialog->show(); // remove
        connect(d->webView->page(), SIGNAL(loadFinished(bool)), SLOT(loadFinished()));
    }

    // Set an empty cookiejar, we don't want to recycle already logged in users
    QNetworkAccessManager *qnam = d->webView->page()->networkAccessManager();
    delete qnam->cookieJar();
    QNetworkCookieJar *jar = new QNetworkCookieJar(this);
    qnam->setCookieJar(jar);
    d->authorizeUrls[authorizeUrl] = d->serviceBaseUrl;
    d->webView->page()->mainFrame()->load(pageUrl);
}

bool KOAuthWebHelper::isIdentica()
{
    return d->serviceBaseUrl.toLower().contains("identi.ca");
}

void KOAuthWebHelper::loadFinished()
{
    QWebPage *page = dynamic_cast<QWebPage*>(sender());
    if (!page) {
        return;
    }

    QWebFrame* mf = page->mainFrame();
    QString u = page->mainFrame()->url().toString();
    if (d->authorizeUrls.contains(u)) {
        QString pin;
        foreach (const QWebElement &code, mf->findAllElements("CODE")) {
            pin = code.toPlainText();
//             kDebug() << "tag:" << code.tagName() << "PIN:" << pin;
        };

        foreach (const QWebElement &code, mf->findAllElements("#oauth_pin")) {
            pin = code.toPlainText();
//             kDebug() << "tag:" << code.tagName() << "PIN:" << pin;
        };
        if (!isIdentica() || !pin.isEmpty()) {
//             kDebug() << "We're done!" << u << pin;
            d->timer->stop(); // No need to show the dialog. :)
            emit statusUpdated(d->user, d->serviceBaseUrl, "Busy", "Application authorized");
            emit appAuthSucceeded(u, pin);
            if (d->dialog) {
                d->dialog->close();
            }
            return;
        }
    } else {
        // The remote service challenges app authorization, this means that the
        // user has to log in to the remote site to retrieve a PIN back, which
        // in turn will produce the accessToken and accessTokenSecret pair in
        // the next, final authorization step.
        emit statusUpdated(d->user, d->serviceBaseUrl, "Waiting", "App Authorization needed");

        // we have to log in, let's see if we can do that automatically...

        // For Twitter, there are two cases:
        // - the user is already logged in, we just have to click the accept button
        // - the user is not logged in, in that case the user/password have to be
        //   be filled in. We happen to know that, as it's the argument to the "auth"
        //   operation in the dataengine.
//         kDebug() << "logging in " << d->user << d->password;
        if (!isIdentica()) {
            // Try to fill in user/pass into the form
            //kDebug() << "twitter.com hacks JavaScript hacks: " << u;
            QString script = "var userName = document.getElementById(\"username_or_email\"); userName.value = \"" + d->user + "\";\n";
            script.append("var passWord = document.getElementById(\"password\"); passWord.value = \"" + d->password + "\";\n");
            mf->evaluateJavaScript(script);
            //kDebug() << "ran script 1: " << script;
            // Evaluate the button click separately, as the above script might abort,
            // and not all lines are actually executed.
            script = QString();
            script.append("var ackButton = document.getElementById(\"allow\"); ackButton.click();");
            mf->evaluateJavaScript(script);
            //kDebug() << "ran script 2: " << script;
        } else if (d->serviceBaseUrl.toLower().contains("identi.ca")) {
            //kDebug() << "Identi.ca hacks JavaScript hacks";
            QVariant r;
            QString script = "var userName = document.getElementById(\"nickname\"); userName.value = \"" + d->user + "\";\n";
            script.append("var passWord = document.getElementById(\"password\"); passWord.value = \"" + d->password + "\";\n");
            r = mf->evaluateJavaScript(script);
            kDebug() << "Ran script 1" << script << r;
            // Evaluate the button click separately, as the above script might abort,
            // and not all lines are actually executed.
            script = "";
            script.append("var ackButton = document.getElementById(\"allow_submit\"); ackButton.click();");
            r = mf->evaluateJavaScript(script);
            kDebug() << "Ran script 2" << script << r;
        }
        d->timer->start();
    }
}

}
#include "koauthwebhelper.moc"
