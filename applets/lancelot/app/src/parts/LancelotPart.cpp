/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "LancelotPart.h"
#include "KDebug"
#include "KIcon"
#include <QGraphicsLayoutItem>
#include <QGraphicsLayout>
#include <QDataStream>

#include "../models/Devices.h"
#include "../models/Places.h"
#include "../models/SystemServices.h"
#include "../models/RecentDocuments.h"
#include "../models/OpenDocuments.h"
#include "../models/NewDocuments.h"
#include "../models/FolderModel.h"
#include "../models/FavoriteApplications.h"
#include "../models/Applications.h"
#include "../models/Runner.h"
#include "../Serializator.h"

class FullLayout: public QGraphicsLayout {
public:
    FullLayout(QGraphicsLayoutItem * parent = 0)
        : QGraphicsLayout(parent)
    {
    }

    virtual int count() const
    {
        return m_items.size();
    }

    void setGeometry(const QRectF & rect)
    {
        QGraphicsLayout::setGeometry(rect);
        invalidate();
    }

    virtual void invalidate()
    {
        foreach (QGraphicsLayoutItem * item, m_items) {
            item->setGeometry(geometry());
            item->setPreferredSize(geometry().size());
        }
    }

    virtual QGraphicsLayoutItem * itemAt(int i) const
    {
        return m_items.at(i);
    }

    virtual void removeAt(int i)
    {
        return m_items.removeAt(i);
    }

    void addItem(QGraphicsLayoutItem * item)
    {
        m_items.append(item);
    }

    virtual QSizeF sizeHint(Qt::SizeHint hint, const QSizeF & constraint) const
    {
        QSizeF size;
        foreach (QGraphicsLayoutItem * item, m_items) {
            size.expandedTo(item->effectiveSizeHint(hint, constraint));
        }
        return size;
    }
private:
    QList< QGraphicsLayoutItem * > m_items;
};

LancelotPart::LancelotPart(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args), m_instance(NULL), m_list(NULL),
    m_model(NULL), m_cmdarg(QString())
{
    m_instance = new Lancelot::Instance();
    FullLayout * fullLayout = new FullLayout(this);
    m_layout = fullLayout;

    m_list = new Lancelot::ActionListView(this);
    fullLayout->addItem(m_list);
    setMinimumSize(QSizeF(150, 200));

    setLayout(m_layout);

    m_instance->activateAll();
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    if (args.size() > 0) {
        m_cmdarg = args[0].toString();
    }
}

void LancelotPart::init()
{
    resize(200, 300);
    bool loaded;
    loaded = loadConfig();

    if (!loaded && !m_cmdarg.isEmpty()) {
        QFile file(QUrl(m_cmdarg).toLocalFile());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                loaded = load(line);
                if (loaded) {
                    saveConfig(line);
                    break;
                }
            }
        }
    }

    if (!loaded) {
        setFailedToLaunch(true, i18n("Lancelot part definition not found"));
    }
}

bool LancelotPart::load(const QString & input)
{
    QMap < QString, QString > data = Serializator::deserialize(input);

    if (!data.contains("version")) {
        return false;
    }

    bool loaded = false;

    if (data["version"] <= "1.0") {
        if (data["type"] == "list") {
            QString modelID = data["model"];
            if (m_model && m_deleteModel) {
                delete m_model;
            }
            m_deleteModel = true;
            m_model = NULL;

            if (modelID == "Places") {
                m_model = new Models::Places();
            } else if (modelID == "System") {
                m_model = new Models::SystemServices();
            } else if (modelID == "Devices/Removable") {
                m_model = new Models::Devices(Models::Devices::Removable);
            } else if (modelID == "Devices/Fixed") {
                m_model = new Models::Devices(Models::Devices::Fixed);
            } else if (modelID == "NewDocuments") {
                m_model = new Models::NewDocuments();
            } else if (modelID == "OpenDocuments") {
                m_model = new Models::OpenDocuments();
            } else if (modelID =="RecentDocuments") {
                m_model = new Models::RecentDocuments();
            } else if (modelID == "FavoriteApplications") {
                m_model = Models::FavoriteApplications::instance();
                m_deleteModel = false;
            }
            loaded = (m_model != NULL);
            m_list->setModel(m_model);
        }
    }

    return loaded;
}

LancelotPart::~LancelotPart()
{
    if (!hasFailedToLaunch()) {
        m_layout->removeAt(0);
        delete m_model;
        delete m_list;
    }
    delete m_instance;
}

void LancelotPart::saveConfig(const QString & data)
{
    KConfigGroup kcg = config();
    kcg.writeEntry("partData", data);
    kcg.sync();
}

bool LancelotPart::loadConfig()
{
    KConfigGroup kcg = config();

    QString data = kcg.readEntry("partData", QString());
    if (data.isEmpty()) {
        return false;
    }
    return load(data);
}

//#include "LancelotPart.maoc"
