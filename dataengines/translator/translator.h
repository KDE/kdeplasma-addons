/*
    <one line to give the program's name and a brief idea of what it does.>
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


#ifndef TRANSLATOR_H
#define TRANSLATOR_H
#include <QObject>
#include <kross/core/manager.h>
#include <kross/core/action.h>
#include "translatorpackage.h"

namespace Plasma {
class Package;
}
class KJob;

class Translator : public QObject
{
  Q_OBJECT
public:
  Translator(QWidget *parent = 0, QString="en", QString="fa", QString="googletranslate");
  QStringList supportedLangs();
  void retrivePage(QString);
  void setFrom(QString);
  void setTo(QString);
  void setProvider(QString);
  QString providerName();
  Plasma::PackageStructure::Ptr packageStructure();
  QStringList supportedScriptLangs();

signals:
  void translated(QString); // contains translated string
  void error(QString); // contains error string
  void retriveError(QString);

public slots:
  void translate(QString);
  void translationError(QString);
  void retrivalError(QString);
  void pageRetrived(KJob*);
  
private:
  Kross::Action *m_action;
  QString m_from;
  QString m_to;
  QString m_providerName;
  Plasma::PackageStructure::Ptr m_packageStructure;
  Plasma::Package *m_package;
};

#endif // TRANSLATOR_H
