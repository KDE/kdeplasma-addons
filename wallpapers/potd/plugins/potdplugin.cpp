/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "potdprovidermodel.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

Q_GLOBAL_STATIC(PotdProviderModel, potdProviderModelSelf)

PotdProviderModel *self(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    QQmlEngine::setObjectOwnership(potdProviderModelSelf, QQmlEngine::CppOwnership);

    return potdProviderModelSelf;
}

class PotdPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(uri == QByteArrayLiteral("org.kde.plasma.wallpapers.potd"));

        qmlRegisterType<PotdProviderModel>(uri, 1, 0, "PotdProviderModel");
        qmlRegisterSingletonType<PotdProviderModel>(uri, 1, 0, "PotdProviderModelInstance", self);
        qmlRegisterUncreatableType<PotdProviderModel>(uri, 1, 0, "Global", QStringLiteral("Error: only enums"));
    }
};

#include "potdplugin.moc"