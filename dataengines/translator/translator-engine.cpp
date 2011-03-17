/*
    Copyright (C) 2011  Farhad Hedayati-Fard <hf.farhad@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "translator-engine.h"
#include "translator.h"

TranslatorEngine::TranslatorEngine(QObject* parent, const QVariantList& args): DataEngine(parent, args)
{
  translator = new Translator((QWidget*)this);
  connect (translator, SIGNAL(translated(QString)), this, SLOT(getDataFromTranslator(QString)));
  connect (translator, SIGNAL(error(QString)), this, SLOT(getDataFromTranslator(QString)));
}

bool TranslatorEngine::sourceRequestEvent(const QString& name)
{
    return updateSourceEvent(name);
}

bool TranslatorEngine::updateSourceEvent(const QString& text)
{
    QStringList args = text.split(':');
    qDebug() << args;
    QString from = args[0];
    QString to = args[1];
    m_original = args[2];
    translator->setFrom(from);
    translator->setTo(to);
    translator->translate(m_original);
    return true;
}

void TranslatorEngine::getDataFromTranslator(QString data)
{
    m_translated = data;
    setData(translator->pluginName(), m_original, m_translated);
}

K_EXPORT_PLASMA_DATAENGINE(translator, TranslatorEngine)


#include "translator-engine.moc"