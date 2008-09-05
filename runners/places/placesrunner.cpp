/*
 *   Copyright 2008 David Edmundson <kde@davidedmundson.co.uk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

#include "placesrunner.h"

#include <KIcon>
#include <KRun>
#include <KUrl>

PlacesRunner::PlacesRunner(QObject* parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName("Places");
    m_filePlaces = new KFilePlacesModel(this);
    connect(m_filePlaces, SIGNAL(setupDone(QModelIndex, bool)), SLOT(setupComplete(QModelIndex, bool)));
}

PlacesRunner::~PlacesRunner()
{
}

void PlacesRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    QList<Plasma::QueryMatch> matches;

    if (term.length() < 3)
        return;

    for (int i = 0; i <= m_filePlaces->rowCount();i++) {
        QModelIndex current_index = m_filePlaces->index(i, 0);
        Plasma::QueryMatch::Type type = Plasma::QueryMatch::NoMatch;
        qreal relevance = 0;

        if (m_filePlaces->text(current_index).toLower() == term.toLower()) {
            type = Plasma::QueryMatch::ExactMatch;
            relevance = 1.0;
        } else if (m_filePlaces->text(current_index).contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.7;
        }

        if (type != Plasma::QueryMatch::NoMatch) {
            Plasma::QueryMatch match(this);
            match.setType(type);
            match.setRelevance(relevance);
            match.setIcon(KIcon(m_filePlaces->icon(current_index)));
            match.setText(m_filePlaces->text(current_index));

            //if we have to mount it set the device udi instead of the URL, as we can't open it directly
            if (m_filePlaces->isDevice(current_index) && m_filePlaces->setupNeeded(current_index)) {
                match.setData(m_filePlaces->deviceForIndex(current_index).udi());
            } else {
                match.setData(m_filePlaces->url(current_index));
            }
            matches << match;
        }
    }
    context.addMatches(term, matches);
}


void PlacesRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action)
{
    Q_UNUSED(context);
    //I don't just pass the model index because the list could change before the user clicks on it, which would make everything go wrong. Ideally we don't want things to go wrong.
    if (action.data().canConvert<KUrl>()) {
        new KRun(action.data().value<KUrl>().url(), 0);
    } else if (action.data().canConvert<QString>()) {
        //search our list for the device with the same udi, then set it up (mount it).
        QString deviceUdi = action.data().toString();

        for (int i = 0; i <= m_filePlaces->rowCount();i++) {
            QModelIndex current_index = m_filePlaces->index(i, 0);
            if (m_filePlaces->isDevice(current_index) && m_filePlaces->deviceForIndex(current_index).udi() == deviceUdi) {
                m_filePlaces->requestSetup(current_index);
                break;
            }
        }
    }
}

//if a device needed mounting, this slot gets called when it's finished.
void PlacesRunner::setupComplete(QModelIndex index, bool success)
{
    if (success) {
        new KRun(m_filePlaces->url(index), 0);
    }
}

#include "placesrunner.moc"
