/*
    This file is part of KDE.

    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "loginwidget.h"

#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsLinearLayout>

#include <KConfigGroup>
#include <KDebug>
#include <KIconLoader>
#include <KJob>
#include <KCMultiDialog>
#include <KLineEdit>
#include <KToolInvocation>

#include <Plasma/IconWidget>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

#include "contactimage.h"
#include "utils.h"


using namespace Plasma;

LoginWidget::LoginWidget(DataEngine* engine, QGraphicsWidget* parent)
        : QGraphicsWidget(parent),
        m_engine(engine)
{
    //int avatarSize = KIconLoader::SizeMedium;
    //int actionSize = KIconLoader::SizeSmallMedium;
    
    m_serverLabel = new Label;
    m_serverLabel->setText(i18n("<b>Login to \"openDesktop.org\"</b>"));
    
    m_userLabel = new Label;
    m_passwordLabel = new Label;
    m_userLabel->setText(i18n("Username:"));
    m_passwordLabel->setText(i18n("Password:"));
    
    m_userEdit = new LineEdit;
    m_passwordEdit = new LineEdit;
    m_passwordEdit->nativeWidget()->setPasswordMode(true);

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    layout->addItem(m_serverLabel);
    layout->addItem(m_userLabel);
    layout->addItem(m_userEdit);
    layout->addItem(m_passwordLabel);
    layout->addItem(m_passwordEdit);
    
    
    int buttonsize = KIconLoader::SizeMedium + 4;    
    Plasma::IconWidget* loginButton = new Plasma::IconWidget;
    loginButton->setIcon("dialog-ok");
    loginButton->setText(i18n("Login"));      
    loginButton->setOrientation(Qt::Horizontal);
    loginButton->setMaximumHeight(buttonsize);
    loginButton->setDrawBackground(true);
    loginButton->setTextBackgroundColor(QColor(Qt::transparent));
    loginButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layout->addItem(loginButton);

    Plasma::IconWidget* registerButton = new Plasma::IconWidget;
    registerButton->setIcon("list-add-user");
    registerButton->setText(i18n("Register new account..."));
    registerButton->setOrientation(Qt::Horizontal);
    registerButton->setMaximumHeight(buttonsize);
    registerButton->setDrawBackground(true);
    registerButton->setTextBackgroundColor(QColor(Qt::transparent));
    registerButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layout->addItem(registerButton);
    
    connect(loginButton, SIGNAL(clicked()), SLOT(login()));
    connect(registerButton, SIGNAL(clicked()), SLOT(registerNewAccount()));
}

void LoginWidget::setProvider(const QString& provider)
{
    m_provider = provider;
}

void LoginWidget::login()
{
    if (!m_userEdit->text().isEmpty()) {
        kDebug() << "set credentials: " << m_provider << m_userEdit->text() <<  m_passwordEdit->text();
        Service* service = m_engine->serviceForSource(settingsQuery(m_provider, "setCredentials"));
        KConfigGroup cg = service->operationDescription("setCredentials");
        cg.writeEntry("username", m_userEdit->text());
        cg.writeEntry("password", m_passwordEdit->text());
        ServiceJob* job = service->startOperationCall(cg);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(loginJobFinished(KJob*)));
        connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
        delete service;
    }
}

void LoginWidget::loginJobFinished(KJob* job)
{
    kDebug() << "Login Job finished: " << job->error();
    if (!job->error()) {
        emit loginFinished();
    }
}

void LoginWidget::registerNewAccount()
{
    KToolInvocation::invokeBrowser("https://www.opendesktop.org/usermanager/new.php");

    /* TODO: use the kcm instead
    KCMultiDialog KCM;
    KCM.setWindowTitle( i18n( "Open Collaboration Providers" ) );
    KCM.addModule( "kcm_attica" );

    KCM.exec();
    */
}


#include "loginwidget.moc"
