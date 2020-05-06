/*
    Copyright (C) 2020 Marco Martin <mart@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "SensorFaceController.h"
#include "SensorFaceController_p.h"
#include "SensorFace_p.h"
#include <SensorQuery.h>

#include <QtQml>
#include <QDebug>

#include <KDesktopFile>
#include <KDeclarative/ConfigPropertyMap>
#include <KPackage/PackageLoader>
#include <KLocalizedString>
#include <KConfigLoader>

using namespace KSysGuard;

FacesModel::FacesModel(QObject *parent)
    : QStandardItemModel(parent)
{
    reload();
}

void FacesModel::reload()
{
    clear();

    auto list = KPackage::PackageLoader::self()->listPackages(QStringLiteral("KSysguard/SensorFace"));
    // NOTE: This will disable completely the internal in-memory cache 
    KPackage::Package p;
    p.install(QString(), QString());

    for (auto plugin : list) {
        QStandardItem *item = new QStandardItem(plugin.name());
        item->setData(plugin.pluginId(), FacesModel::PluginIdRole);
        appendRow(item);
    }
}

QString FacesModel::pluginId(int row)
{
    return data(index(row, 0), PluginIdRole).toString();
}

QHash<int, QByteArray> FacesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
 
    roles[PluginIdRole] = "pluginId";
    return roles;
}

PresetsModel::PresetsModel(QObject *parent)
    : QStandardItemModel(parent)
{
    reload();
}

void PresetsModel::reload()
{
    clear();
    QList<KPluginMetaData> plugins = KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), [](const KPluginMetaData &plugin) {
        return plugin.value(QStringLiteral("X-Plasma-RootPath")) == QStringLiteral("org.kde.plasma.systemmonitor");
    });

    QSet<QString> usedNames;

    // We iterate backwards because packages under ~/.local are listed first, while we want them last
    auto it = plugins.rbegin();
    for (; it != plugins.rend(); ++it) {
        const auto &plugin = *it;
        KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"), plugin.pluginId());
        KDesktopFile df(p.path() + QStringLiteral("metadata.desktop"));

        QString baseName = df.readName();
        QString name = baseName;
        int id = 0;

        while (usedNames.contains(name)) {
            name = baseName + QStringLiteral(" (") + QString::number(++id) + QStringLiteral(")");
        }
        usedNames << name;

        QStandardItem *item = new QStandardItem(baseName);

        // TODO config
        QVariantMap config;

        KConfigGroup configGroup(df.group("Config"));

        const QStringList keys = configGroup.keyList();
        for (const QString &key : keys) {
            // all strings for now, type conversion happens in QML side when we have the config property map
            config.insert(key, configGroup.readEntry(key));
        }

        item->setData(plugin.pluginId(), PresetsModel::PluginIdRole);
        item->setData(config, PresetsModel::ConfigRole);

        item->setData(QFileInfo(p.path() + QStringLiteral("metadata.desktop")).isWritable(), PresetsModel::WritableRole);

        appendRow(item);
    }
}

QHash<int, QByteArray> PresetsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    roles[PluginIdRole] = "pluginId";
    roles[ConfigRole] = "config";
    roles[WritableRole] = "writable";
    return roles;
}



class SensorFaceController::Private
{
public:
    Private();
    SensorFace *createGui(const QString &qmlPath);
    QQuickItem *createConfigUi(const QString &file, const QVariantMap &initialProperties);

    SensorFaceController *q;
    QString title;
    QQmlEngine *engine;

    KDesktopFile *faceMetadata = nullptr;
    KDeclarative::ConfigPropertyMap *faceConfiguration = nullptr;
    KConfigLoader *faceConfigLoader = nullptr;

    bool configNeedsSave = false;
    KPackage::Package facePackage;
    QString faceId;
    KConfigGroup configGroup;
    KConfigGroup appearanceGroup;
    KConfigGroup sensorsGroup;
    QPointer <SensorFace> fullRepresentation;
    QPointer <SensorFace> compactRepresentation;
    QPointer <QQuickItem> faceConfigUi;
    QPointer <QQuickItem> appearanceConfigUi;
    QPointer <QQuickItem> sensorsConfigUi;

    QTimer *syncTimer;
    FacesModel *availableFacesModel = nullptr;
    PresetsModel *availablePresetsModel = nullptr;
};

SensorFaceController::Private::Private()
{}

SensorFace *SensorFaceController::Private::createGui(const QString &qmlPath)
{
    QQmlComponent *component = new QQmlComponent(engine, qmlPath, nullptr);
    // TODO: eventually support async  components? (only useful for qml files from http, we probably don't want that)
    if (component->status() != QQmlComponent::Ready) {
        qCritical() << "Error creating component:";
        for (auto err : component->errors()) {
            qWarning() << err.toString();
        }
        component->deleteLater();
        return nullptr;
    }

    //TODO: add i18n context object
    QQmlContext *context = new QQmlContext(engine);
    QObject *guiObject = component->beginCreate(context);
    SensorFace *gui = qobject_cast<SensorFace *>(guiObject);
    if (!gui) {
        qWarning()<<"ERROR: QML gui" << guiObject << "not a SensorFace instance";
        guiObject->deleteLater();
        context->deleteLater();
        return nullptr;
    }
   // context->setParent(gui);

    gui->setController(q);

    component->completeCreate();

    component->deleteLater();
    return gui;
}

QQuickItem *SensorFaceController::Private::createConfigUi(const QString &file, const QVariantMap &initialProperties)
{
    QQmlComponent *component = new QQmlComponent(engine, file, nullptr);
    // TODO: eventually support async  components? (only useful for qml files from http, we probably don't want that)
    if (component->status() != QQmlComponent::Ready) {
        qCritical() << "Error creating component:";
        for (auto err : component->errors()) {
            qWarning() << err.toString();
        }
        component->deleteLater();
        return nullptr;
    }

    //TODO: add i18n context object
    QQmlContext *context = new QQmlContext(engine);
    QObject *guiObject = component->createWithInitialProperties(
        initialProperties, context);
    QQuickItem *gui = qobject_cast<QQuickItem *>(guiObject);
    Q_ASSERT(gui);

    component->deleteLater();

    return gui;
}





SensorFaceController::SensorFaceController(KConfigGroup &config, QQmlEngine *engine)
    : QObject(engine),
      d(std::make_unique<Private>())
{
    d->q = this;
    d->configGroup = config;
    d->appearanceGroup = KConfigGroup(&config, "Appearance");
    d->sensorsGroup = KConfigGroup(&config, "Sensors");
    d->engine = engine;
    d->syncTimer = new QTimer(this);
    d->syncTimer->setSingleShot(true);
    d->syncTimer->setInterval(5000);
    connect(d->syncTimer, &QTimer::timeout, this, [this]() {
        d->appearanceGroup.sync();
        d->sensorsGroup.sync();
    });

    setFaceId(d->appearanceGroup.readEntry("chartFace", QStringLiteral("org.kde.ksysguard.piechart")));
}

SensorFaceController::~SensorFaceController()
{
}

QString SensorFaceController::title() const
{
    return d->appearanceGroup.readEntry("title", name());
}

void SensorFaceController::setTitle(const QString &title)
{
    if (title == SensorFaceController::title()) {
        return;
    }

    d->appearanceGroup.writeEntry("title", title);
    d->syncTimer->start();
    
    emit titleChanged();
}

QStringList SensorFaceController::totalSensors() const
{
    return d->sensorsGroup.readEntry("totalSensors", QStringList());
}

void  SensorFaceController::setTotalSensors(const QStringList &totalSensors)
{
    if (totalSensors == SensorFaceController::totalSensors()) {
        return;
    }

    d->sensorsGroup.writeEntry("totalSensors", totalSensors);
    d->syncTimer->start();
    emit totalSensorsChanged();
}

QStringList SensorFaceController::highPrioritySensorIds() const
{
    return d->sensorsGroup.readEntry("highPrioritySensorIds", QStringList());
}

void SensorFaceController::setHighPrioritySensorIds(const QStringList &highPrioritySensorIds)
{
    if (highPrioritySensorIds == SensorFaceController::highPrioritySensorIds()) {
        return;
    }

    d->sensorsGroup.writeEntry("highPrioritySensorIds", highPrioritySensorIds);
    d->syncTimer->start();
    emit highPrioritySensorIdsChanged();
}

QStringList SensorFaceController::highPrioritySensorColors() const
{
    return d->sensorsGroup.readEntry("highPrioritySensorColors", QStringList());
}

void SensorFaceController::setHighPrioritySensorColors(const QStringList &highPrioritySensorColors)
{
    if (highPrioritySensorColors == SensorFaceController::highPrioritySensorColors()) {
        return;
    }

    d->sensorsGroup.writeEntry("highPrioritySensorColors", highPrioritySensorColors);
    d->syncTimer->start();
    emit highPrioritySensorColorsChanged();
}

QStringList SensorFaceController::lowPrioritySensorIds() const
{
    return d->sensorsGroup.readEntry("lowPrioritySensorIds", QStringList());
}

void SensorFaceController::setLowPrioritySensorIds(const QStringList &lowPrioritySensorIds)
{
    if (lowPrioritySensorIds == SensorFaceController::lowPrioritySensorIds()) {
        return;
    }

    d->sensorsGroup.writeEntry("lowPrioritySensorIds", lowPrioritySensorIds);
    d->syncTimer->start();
    emit lowPrioritySensorIdsChanged();
}

// from face config, immutable by the user
QString SensorFaceController::name() const
{
    if (!d->faceMetadata) {
        return QString();
    }
    return d->faceMetadata->readName();
}

const QString SensorFaceController::icon() const
{
    if (!d->faceMetadata) {
        return QString();
    }
    return d->faceMetadata->readIcon();
}

bool SensorFaceController::supportsSensorsColors() const
{
    if (!d->faceMetadata) {
        return false;
    }

    KConfigGroup cg(d->faceMetadata, QStringLiteral("Config"));
    return cg.readEntry("SupportsSensorsColors", false);
}

bool SensorFaceController::supportsTotalSensors() const
{
    if (!d->faceMetadata) {
        return false;
    }

    KConfigGroup cg(d->faceMetadata, QStringLiteral("Config"));
    return cg.readEntry("SupportsTotalSensor", false);
}

bool SensorFaceController::supportsLowPrioritySensors() const
{
    if (!d->faceMetadata) {
        return false;
    }

    KConfigGroup cg(d->faceMetadata, QStringLiteral("Config"));
    return cg.readEntry("SupportsLowPrioritySensors", false);
}

void SensorFaceController::setFaceId(const QString &face)
{
    if (d->faceId == face) {
        return;
    }

    if (d->fullRepresentation) {
        d->fullRepresentation->deleteLater();
        d->fullRepresentation.clear();
    }
    if (d->compactRepresentation) {
        d->compactRepresentation->deleteLater();
        d->fullRepresentation.clear();
    }

    d->faceId = face;

    d->facePackage = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KSysguard/SensorFace"), face);

    delete d->faceMetadata;
    d->faceMetadata = nullptr;
    if (d->faceConfiguration) {
        d->faceConfiguration->deleteLater();
        d->faceConfiguration = nullptr;
    }
    if (d->faceConfigLoader) {
        d->faceConfigLoader ->deleteLater();
        d->faceConfigLoader = nullptr;
    }

    if (!d->facePackage.isValid()) {
        emit faceIdChanged();
        return;
    }

    //TODO: should be in a different config file rather than metadata
    //d->faceMetadata = new KDesktopFile(d->facePackage.filePath("metadata"));
    d->faceMetadata = new KDesktopFile(d->facePackage.path() + QStringLiteral("metadata.desktop"));

    const QString xmlPath = d->facePackage.filePath("mainconfigxml");

    if (!xmlPath.isEmpty()) {
        QFile file(xmlPath);
        KConfigGroup cg(&d->configGroup, d->faceId);

        d->faceConfigLoader = new KConfigLoader(cg, &file, this);
        d->faceConfiguration = new KDeclarative::ConfigPropertyMap(d->faceConfigLoader, this);
    }

    d->appearanceGroup.writeEntry("chartFace", face);
    d->syncTimer->start();
    emit faceIdChanged();
    return;
}

QString SensorFaceController::faceId() const
{
    return d->faceId;
}

KDeclarative::ConfigPropertyMap *SensorFaceController::faceConfiguration() const
{
    return d->faceConfiguration;
}

QQuickItem *SensorFaceController::compactRepresentation()
{
    if (!d->facePackage.isValid()) {
        return nullptr;
    } else if (d->compactRepresentation) {
        return d->compactRepresentation;
    }

    d->fullRepresentation = d->createGui(d->facePackage.filePath("ui", QStringLiteral("CompactRepresentation.qml")));   
    return d->compactRepresentation;
}

QQuickItem *SensorFaceController::fullRepresentation()
{
    if (!d->facePackage.isValid()) {
        return nullptr;
    } else if (d->fullRepresentation) {
        return d->fullRepresentation;
    }

    d->fullRepresentation = d->createGui(d->facePackage.filePath("ui", QStringLiteral("FullRepresentation.qml")));

    return d->fullRepresentation;
}

QQuickItem *SensorFaceController::faceConfigUi()
{
    if (!d->facePackage.isValid()) {
        return nullptr;
    } else if (d->faceConfigUi) {
        return d->faceConfigUi;
    }

    d->faceConfigUi = d->createConfigUi(QStringLiteral(":/FaceDetailsConfig.qml"),
    {{QStringLiteral("controller"), QVariant::fromValue(this)},
         {QStringLiteral("source"), d->facePackage.filePath("ui", QStringLiteral("Config.qml"))}});

    return d->faceConfigUi;
}

QQuickItem *SensorFaceController::appearanceConfigUi()
{
    if (d->appearanceConfigUi) {
        return d->appearanceConfigUi;
    }

    d->appearanceConfigUi = d->createConfigUi(QStringLiteral(":/ConfigAppearance.qml"), {{QStringLiteral("controller"), QVariant::fromValue(this)}});

    return d->appearanceConfigUi;
}

QQuickItem *SensorFaceController::sensorsConfigUi()
{
    if (d->sensorsConfigUi) {
        return d->sensorsConfigUi;
    }

    d->sensorsConfigUi = d->createConfigUi(QStringLiteral(":/ConfigSensors.qml"), {{QStringLiteral("controller"), QVariant::fromValue(this)}});

    return d->sensorsConfigUi;
}

QAbstractItemModel *SensorFaceController::availableFacesModel()
{
    if (d->availableFacesModel) {
        return d->availableFacesModel;
    }

    d->availableFacesModel = new FacesModel(this);
    return d->availableFacesModel;
}

QAbstractItemModel *SensorFaceController::availablePresetsModel()
{
    if (d->availablePresetsModel) {
        return d->availablePresetsModel;
    }

    d->availablePresetsModel = new PresetsModel(this);

    return d->availablePresetsModel;
}

void SensorFaceController::reloadConfig()
{
    if (d->faceConfigLoader) {
        d->faceConfigLoader->load();
    }

    //Force to re-read all the values
    setFaceId(d->appearanceGroup.readEntry("chartFace", QStringLiteral("org.kde.ksysguard.textonly")));
    titleChanged();
    totalSensorsChanged();
    highPrioritySensorIdsChanged();
    highPrioritySensorColorsChanged();
    lowPrioritySensorIdsChanged();
}

void SensorFaceController::loadPreset(const QString &preset)
{
    if (preset.isEmpty()) {
        return;
    }

    auto presetPackage = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"));

    presetPackage.setPath(preset);

    if (!presetPackage.isValid()) {
        return;
    }

    if (presetPackage.metadata().value(QStringLiteral("X-Plasma-RootPath")) != QStringLiteral("org.kde.plasma.systemmonitor")) {
        return;
    }

    KDesktopFile df(presetPackage.path() + QStringLiteral("metadata.desktop"));
    KConfigGroup presetGroup(df.group("Config"));

    // Load the title
    setTitle(df.readName());

    //Remove the "custon" value from presets models
    if (d->availablePresetsModel &&
        d->availablePresetsModel->data(d->availablePresetsModel->index(0, 0), PresetsModel::PluginIdRole).toString().isEmpty()) {
        d->availablePresetsModel->removeRow(0);
    }


    auto loadSensors = [this](const QStringList &partialEntries) {
        QStringList sensors;

        for (const QString &id : partialEntries) {
            KSysGuard::SensorQuery query{id};
            query.execute();
            query.waitForFinished();

            sensors.append(query.sensorIds());
        }
        return sensors;
    };

    setTotalSensors(presetGroup.readEntry(QStringLiteral("totalSensors"), QStringList()));
    setHighPrioritySensorIds(loadSensors(presetGroup.readEntry(QStringLiteral("highPrioritySensorIds"), QStringList())));
    setLowPrioritySensorIds(loadSensors(presetGroup.readEntry(QStringLiteral("lowPrioritySensorIds"), QStringList())));
    setHighPrioritySensorColors(presetGroup.readEntry(QStringLiteral("highPrioritySensorColors"), QStringList()));
    setFaceId(presetGroup.readEntry(QStringLiteral("chartFace"), QStringLiteral("org.kde.ksysguard.piechart")));

    if (d->faceConfigLoader) {
        presetGroup = KConfigGroup(df.group("FaceConfig"));
        for (const QString &key : presetGroup.keyList()) {
            KConfigSkeletonItem *item = d->faceConfigLoader->findItemByName(key);
            if (item) {
                if (item->property().type() == QVariant::StringList) {
                    item->setProperty(presetGroup.readEntry(key, QStringList()));
                } else {
                    item->setProperty(presetGroup.readEntry(key));
                }
                d->faceConfigLoader->save();
                d->faceConfigLoader->read();
            }
        }
    }
}

void SensorFaceController::savePreset()
{
    QString pluginName = QStringLiteral("org.kde.plasma.systemmonitor.") + title().simplified().replace(QLatin1Char(' '), QChar()).toLower();
    int suffix = 0;

    auto presetPackage = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"));

    presetPackage.setPath(pluginName);
    if (presetPackage.isValid()) {
        do {
            presetPackage.setPath(QString());
            presetPackage.setPath(pluginName + QString::number(++suffix));
        } while (presetPackage.isValid());

        pluginName += QString::number(suffix);
    }

    QTemporaryDir dir;
    if (!dir.isValid()) {
        return;
    }

    KConfig c(dir.path() % QLatin1Literal("/metadata.desktop"));

    KConfigGroup cg(&c, "Desktop Entry");
    cg.writeEntry("Name", title());
    cg.writeEntry("Icon", "ksysguardd");
    cg.writeEntry("X-Plasma-API", "declarativeappletscript");
    cg.writeEntry("X-Plasma-MainScript", "ui/main.qml");
    cg.writeEntry("X-Plasma-Provides", "org.kde.plasma.systemmonitor");
    cg.writeEntry("X-Plasma-RootPath", "org.kde.plasma.systemmonitor");
    cg.writeEntry("X-KDE-PluginInfo-Name", pluginName);
    cg.writeEntry("X-KDE-ServiceTypes", "Plasma/Applet");
    cg.writeEntry("X-KDE-PluginInfo-Category", "System Information");
    cg.writeEntry("X-KDE-PluginInfo-License", "LGPL 2.1+");
    cg.writeEntry("X-KDE-PluginInfo-EnabledByDefault", "true");
    cg.writeEntry("X-KDE-PluginInfo-Version", "0.1");
    cg.sync();

    KConfigGroup configGroup(&c, "Config");
    configGroup.writeEntry(QStringLiteral("totalSensors"), totalSensors());
    configGroup.writeEntry(QStringLiteral("highPrioritySensorIds"), highPrioritySensorIds());
    configGroup.writeEntry(QStringLiteral("lowPrioritySensorIds"), lowPrioritySensorIds());
    configGroup.writeEntry(QStringLiteral("highPrioritySensorColors"), highPrioritySensorColors());
    

    auto *job = presetPackage.install(dir.path());

    connect(job, &KJob::finished, this, [this, pluginName] () {
        d->availablePresetsModel->reload();
    });
}

void SensorFaceController::uninstallPreset(const QString &pluginId)
{
    auto presetPackage = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Applet"), pluginId);

    if (presetPackage.metadata().value(QStringLiteral("X-Plasma-RootPath")) != QStringLiteral("org.kde.plasma.systemmonitor")) {
        return;
    }

    QDir root(presetPackage.path());
    root.cdUp();
    auto *job = presetPackage.uninstall(pluginId, root.path());

    connect(job, &KJob::finished, this, [this] () {
        d->availablePresetsModel->reload();
    });
}

#include "moc_SensorFaceController.cpp"
