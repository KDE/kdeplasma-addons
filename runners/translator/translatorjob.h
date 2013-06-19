/******************************************************************************
 *  Copyright (C) 2013 by David Baum <david.baum@naraesk.eu>                  *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#ifndef TRANSLATORJOB_H
#define TRANSLATORJOB_H

#include <QtNetwork/QNetworkReply>

class QNetworkReply;
class QNetworkAccessManager;

class TranslatorJob :  public QObject
{
    Q_OBJECT
public:
	TranslatorJob(const QString &, const QPair<QString, QString> &);
	QString result();
	void start();

private Q_SLOTS:
	void jobCompleted(QNetworkReply *);

Q_SIGNALS:
	void finished();

private:
	QNetworkAccessManager *m_manager;
	QString m_result;
};

#endif