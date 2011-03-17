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


#ifndef TRANSLATOR_ENGINE_H
#define TRANSLATOR_ENGINE_H

#include <Plasma/DataEngine>
class Translator;

class TranslatorEngine : public Plasma::DataEngine
{
  Q_OBJECT
public:
  TranslatorEngine(QObject* parent, const QVariantList& args);

protected:
  bool sourceRequestEvent(const QString& name);
  bool updateSourceEvent(const QString& text);
private slots:
  void getDataFromTranslator(QString);

private:
  Translator *translator;
  QString m_translated;
  QString m_original;
};

#endif // TRANSLATOR_H
