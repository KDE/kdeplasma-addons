/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>

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

#include "requestfriendshipwidget.h"

#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsLinearLayout>

#include <KConfigGroup>
#include <KIconLoader>
#include <KTextEdit>

#include <Plasma/IconWidget>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

#include "contactimage.h"
#include "utils.h"


using namespace Plasma;

RequestFriendshipWidget::RequestFriendshipWidget(DataEngine* engine, QGraphicsWidget* parent)
        : Frame(parent),
        m_engine(engine),
        m_personWatch(engine)
{
    m_updateTimer.setInterval(1000);
    m_updateTimer.setSingleShot(true);

    int avatarSize = KIconLoader::SizeMedium;
    int actionSize = KIconLoader::SizeSmallMedium;
    
    Label* title = new Label;
    title->setText(i18n("<b>Add as friend</b>"));

    // Recipient
    m_image = new ContactImage(m_engine);
    m_image->setMinimumHeight(avatarSize);
    m_image->setMinimumWidth(avatarSize);
    m_image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_toLabel = new Label;
    m_toEdit = new LineEdit;
    
    QGraphicsGridLayout* toLayout = new QGraphicsGridLayout;
    toLayout->setColumnFixedWidth(0, avatarSize * 1.2);
    toLayout->addItem(m_image, 0, 0, 2, 1);
    toLayout->addItem(m_toLabel, 0, 1);
    toLayout->addItem(m_toEdit, 1, 1);

    Label* bodyLabel = new Label;
    bodyLabel->setText(i18n("Message:"));

    Frame* bodyFrame = new Frame(this);
    bodyFrame->setFrameShadow(Sunken);
    bodyFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_body = new TextEdit;
    (new QGraphicsLinearLayout(bodyFrame))->addItem(m_body);

    Plasma::IconWidget* cancel = new Plasma::IconWidget;
    cancel->setIcon("go-previous-view");
    cancel->setToolTip(i18n("Back"));
    cancel->setMinimumHeight(actionSize);
    cancel->setMaximumHeight(actionSize);
    cancel->setMinimumWidth(actionSize);
    cancel->setMaximumWidth(actionSize);

    m_submit = new Plasma::IconWidget;
    m_submit->setIcon("dialog-ok");
    m_submit->setToolTip(i18n("Send"));
    m_submit->setMinimumHeight(actionSize);
    m_submit->setMaximumHeight(actionSize);
    m_submit->setMinimumWidth(actionSize);
    m_submit->setMaximumWidth(actionSize);
    m_submit->setEnabled(false);

    QGraphicsLinearLayout* buttonLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    buttonLayout->addItem(cancel);
    buttonLayout->addStretch();
    buttonLayout->addItem(m_submit);

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    layout->addItem(title);
    layout->addItem(toLayout);
    layout->addItem(bodyLabel);
    layout->addItem(bodyFrame);
    layout->addItem(buttonLayout);

    connect(m_submit, SIGNAL(clicked()), SLOT(send()));
    connect(cancel, SIGNAL(clicked()), SIGNAL(done()));
    connect(&m_updateTimer, SIGNAL(timeout()), SLOT(updateTo()));
    connect(m_toEdit, SIGNAL(editingFinished()), SLOT(updateTo()));
    connect(m_toEdit, SIGNAL(textEdited(QString)), SLOT(updateSendAction()));
    connect(m_toEdit, SIGNAL(textEdited(QString)), SLOT(toChanged(QString)));
    connect(m_toEdit, SIGNAL(returnPressed()), SLOT(switchToBody()));
    connect(&m_personWatch, SIGNAL(updated()), SLOT(personUpdated()));
    connect(m_body, SIGNAL(textChanged()), SLOT(updateSendAction()));
}


void RequestFriendshipWidget::personUpdated()
{
    DataEngine::Data personData = m_personWatch.data();
    m_toLabel->setText(personData.value("Name").toString());
    m_image->setUrl(personData.value("AvatarUrl").toUrl());
}


void RequestFriendshipWidget::send() {
    Service* service = m_engine->serviceForSource(personQuery(m_provider, m_id));
    KConfigGroup cg = service->operationDescription("invite");
    cg.writeEntry("Message", m_body->nativeWidget()->toPlainText());
    KJob *job = service->startOperationCall(cg);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));

    // FIXME: We do not wait for the result atm
    emit done();
    m_id.clear();
    m_toEdit->setText(QString());
    m_personWatch.setId(QString());
    m_body->setText(QString());
}


void RequestFriendshipWidget::switchToBody()
{
    m_body->setFocus();
}


void RequestFriendshipWidget::toChanged(const QString& to)
{
    m_id.clear();
    updateTo();
    m_id = to;
    m_updateTimer.stop();
    m_updateTimer.start();
}


void RequestFriendshipWidget::updateSendAction()
{
    m_submit->setEnabled(!m_toEdit->text().isEmpty() && !m_body->nativeWidget()->toPlainText().isEmpty());
}


void RequestFriendshipWidget::updateTo()
{
    m_personWatch.setId(m_id);
}


void RequestFriendshipWidget::setId(const QString& id)
{
    m_id = id;
    m_toEdit->setText(m_id);
    m_personWatch.setId(m_id);
}


void RequestFriendshipWidget::setProvider(const QString& provider)
{
    m_id.clear();
    m_provider = provider;
    m_toEdit->setText(m_id);
    m_personWatch.setId(m_id);
    m_personWatch.setProvider(m_provider);
}


#include "requestfriendshipwidget.moc"
