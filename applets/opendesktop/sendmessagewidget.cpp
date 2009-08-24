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

#include "sendmessagewidget.h"

#include <QtGui/QGraphicsLinearLayout>

#include <KConfigGroup>
#include <KTextEdit>

#include <Plasma/Service>

#include "utils.h"


SendMessageWidget::SendMessageWidget(Plasma::DataEngine* engine, const QString& id, QGraphicsWidget* parent)
    : Frame(parent),
      m_engine(engine),
      m_id(id),
      m_query(personQuery(id))
{
    m_label = new Plasma::Label(this);

    Frame* subjectFrame = new Frame(this);
    subjectFrame->setFrameShadow(Sunken);
    m_subject = new Plasma::TextEdit(subjectFrame);
    m_subject->setText("Subject");
    (new QGraphicsLinearLayout(subjectFrame))->addItem(m_subject);
    
    Frame* bodyFrame = new Frame(this);
    bodyFrame->setFrameShadow(Sunken);
    m_body = new Plasma::TextEdit(bodyFrame);
    m_body->setText("Body");
    (new QGraphicsLinearLayout(bodyFrame))->addItem(m_body);

    m_submit = new Plasma::PushButton(this);
    m_submit->setText(i18n("Send"));
    connect(m_submit, SIGNAL(clicked()), SIGNAL(send()));
    
    Plasma::PushButton* cancel = new Plasma::PushButton(this);
    cancel->setText(i18n("Cancel"));
    connect(cancel, SIGNAL(clicked()), SIGNAL(done()));
    
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    layout->addItem(m_label);
    layout->addItem(subjectFrame);
    layout->addItem(bodyFrame);
    layout->addItem(m_submit);
    layout->addItem(cancel);

    m_engine->connectSource(m_query, this);
    dataUpdated(m_query, m_engine->query(m_query));
    
    connect(m_submit, SIGNAL(clicked()), SLOT(send()));
}


void SendMessageWidget::dataUpdated(const QString& source, const Plasma::DataEngine::Data& data)
{
    Q_UNUSED(source);
    
    Plasma::DataEngine::Data personData = data[personQuery(m_id)].value<Plasma::DataEngine::Data>();
    m_label->setText(i18n("To: %1 (%2)").arg(personData["Name"].toString()).arg(personData["Id"].toString()));
}


void SendMessageWidget::send()
{
    Plasma::Service* service = m_engine->serviceForSource("Message");
    KConfigGroup cg = service->operationDescription("sendMessage");
    cg.writeEntry("To", m_id);
    cg.writeEntry("Subject", m_subject->nativeWidget()->toPlainText());
    cg.writeEntry("Body", m_body->nativeWidget()->toPlainText());
    service->startOperationCall(cg);
    
    // FIXME: We do not wait for the result atm
    emit done();
}


#include "sendmessagewidget.moc"
