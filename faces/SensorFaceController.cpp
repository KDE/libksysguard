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
#include <KPluginMetaData>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/Predicate>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>

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

        KConfigGroup configGroup(KSharedConfig::openConfig(p.filePath("config", QStringLiteral("faceproperties"))), QStringLiteral("Config"));

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

QVector<QPair<QRegularExpression, QString>> KSysGuard::SensorFaceControllerPrivate::sensorIdReplacements;
QRegularExpression SensorFaceControllerPrivate::oldDiskSensor = QRegularExpression(QStringLiteral("^disk\\/(.+)_\\(\\d+:\\d+\\)"));
QRegularExpression SensorFaceControllerPrivate::oldPartitionSensor = QRegularExpression(QStringLiteral("^partitions(\\/.+)\\/"));

SensorFaceControllerPrivate::SensorFaceControllerPrivate()
{
    if (SensorFaceControllerPrivate::sensorIdReplacements.isEmpty()) {
        // A list of conversion rules to convert old sensor ids to new ones.
        // When loading, each regular expression tries to match to the sensor
        // id. If it matches, it will be be used to replace the sensor id with
        // the second argument.
        sensorIdReplacements = {
            { QRegularExpression(QStringLiteral("network/interfaces/(.*)")), QStringLiteral("network/\\1")},
            { QRegularExpression(QStringLiteral("network/all/receivedDataRate$")), QStringLiteral("network/all/download")},
            { QRegularExpression(QStringLiteral("network/all/sentDataRate$")), QStringLiteral("network/all/upload")},
            { QRegularExpression(QStringLiteral("network/all/totalReceivedData$")), QStringLiteral("network/all/totalDownload")},
            { QRegularExpression(QStringLiteral("network/all/totalSentData$")), QStringLiteral("network/all/totalUpload")},
            { QRegularExpression(QStringLiteral("(.*)/receiver/data$")), QStringLiteral("\\1/download")},
            { QRegularExpression(QStringLiteral("(.*)/transmitter/data$")), QStringLiteral("\\1/upload")},
            { QRegularExpression(QStringLiteral("(.*)/receiver/dataTotal$")), QStringLiteral("\\1/totalDownload")},
            { QRegularExpression(QStringLiteral("(.*)/transmitter/dataTotal$")), QStringLiteral("\\1/totalUpload")},
            { QRegularExpression(QStringLiteral("(.*)/Rate/rio")), QStringLiteral("\\1/read")},
            { QRegularExpression(QStringLiteral("(.*)/Rate/wio$")), QStringLiteral("\\1/write")},
            { QRegularExpression(QStringLiteral("(.*)/freespace$")), QStringLiteral("\\1/free")},
            { QRegularExpression(QStringLiteral("(.*)/filllevel$")), QStringLiteral("\\1/usedPercent")},
            { QRegularExpression(QStringLiteral("(.*)/usedspace$")), QStringLiteral("\\1/used")},
        };
    }
}

QString SensorFaceControllerPrivate::replaceDiskId(const QString &entryName) const
{
    const auto match = oldDiskSensor.match(entryName);
    if (!match.hasMatch()) {
        return entryName;
    }
    const QString device = match.captured(1);
    Solid::Predicate predicate(Solid::DeviceInterface::StorageAccess);
    predicate &= Solid::Predicate(Solid::DeviceInterface::Block, QStringLiteral("device"), QStringLiteral("/dev/%1").arg(device));
    const auto devices = Solid::Device::listFromQuery(predicate);
    if (devices.empty()) {
        return QString();
    }
    QString sensorId = entryName;
    const auto volume = devices[0].as<Solid::StorageVolume>();
    const QString id = volume->uuid().isEmpty() ? volume->label() : volume->uuid();
    return sensorId.replace(match.captured(0), QStringLiteral("disk/") + id);
}

QString SensorFaceControllerPrivate::replacePartitionId(const QString &entryName) const
{
    const auto match = oldPartitionSensor.match(entryName);
    if (!match.hasMatch()) {
        return entryName;
    }
    QString sensorId = entryName;

    if (match.captured(1) == QLatin1String("/all")) {
        return sensorId.replace(match.captured(0), QStringLiteral("disk/all/"));
    }

    const QString filePath = match.captured(1) == QLatin1String("/__root__") ? QStringLiteral("/") : match.captured(1);
    const Solid::Predicate predicate(Solid::DeviceInterface::StorageAccess, QStringLiteral("filePath"), filePath);
    const auto devices = Solid::Device::listFromQuery(predicate);
    if (devices.empty()) {
        return entryName;
    }
    const auto volume = devices[0].as<Solid::StorageVolume>();
    const QString id = volume->uuid().isEmpty() ? volume->label() : volume->uuid();
    return sensorId.replace(match.captured(0), QStringLiteral("disk/%1/").arg(id));
}

QJsonArray SensorFaceControllerPrivate::readSensors(const KConfigGroup &read, const QString &entryName)
{
    auto original = QJsonDocument::fromJson(read.readEntry(entryName, QString()).toUtf8()).array();
    QJsonArray newSensors;
    for (auto entry : original) {
        QString sensorId = entry.toString();
        for (auto replacement : qAsConst(sensorIdReplacements)) {
            auto match = replacement.first.match(sensorId);
            if (match.hasMatch()) {
                sensorId.replace(replacement.first, replacement.second);
            }
        }
        sensorId = replaceDiskId(sensorId);
        sensorId = replacePartitionId(sensorId);
        newSensors.append(sensorId);
    }

    return newSensors;
}

QJsonArray SensorFaceControllerPrivate::readAndUpdateSensors(KConfigGroup& config, const QString& entryName)
{
    auto original = QJsonDocument::fromJson(config.readEntry(entryName, QString()).toUtf8()).array();

    const KConfigGroup &group = config;
    auto newSensors = readSensors(group, entryName);

    if (newSensors != original) {
        config.writeEntry(entryName, QJsonDocument(newSensors).toJson(QJsonDocument::Compact));
    }

    return newSensors;
}

QJsonArray SensorFaceControllerPrivate::resolveSensors(const QJsonArray &partialEntries)
{
    QJsonArray sensors;

    for (const auto &id : partialEntries) {
        KSysGuard::SensorQuery query{id.toString()};
        query.execute();
        query.waitForFinished();
        auto ids = query.sensorIds();
        std::stable_sort(ids.begin(), ids.end());
        for (const auto &fitleredId : qAsConst(ids)) {
            sensors.append(QJsonValue(fitleredId));
        }
    }
    return sensors;
};

SensorFace *SensorFaceControllerPrivate::createGui(const QString &qmlPath)
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

    QQmlContext *context = new QQmlContext(engine);
    context->setContextObject(contextObj);
    QObject *guiObject = component->beginCreate(context);
    SensorFace *gui = qobject_cast<SensorFace *>(guiObject);
    if (!gui) {
        qWarning()<<"ERROR: QML gui" << guiObject << "not a SensorFace instance";
        guiObject->deleteLater();
        context->deleteLater();
        return nullptr;
    }
    context->setParent(gui);

    gui->setController(q);

    component->completeCreate();

    component->deleteLater();
    return gui;
}

QQuickItem *SensorFaceControllerPrivate::createConfigUi(const QString &file, const QVariantMap &initialProperties)
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

    QQmlContext *context = new QQmlContext(engine);
    context->setContextObject(contextObj);
    QObject *guiObject = component->createWithInitialProperties(
        initialProperties, context);
    QQuickItem *gui = qobject_cast<QQuickItem *>(guiObject);
    Q_ASSERT(gui);
    context->setParent(gui);

    component->deleteLater();

    return gui;
}


SensorFaceController::SensorFaceController(KConfigGroup &config, QQmlEngine *engine)
    : QObject(engine),
      d(std::make_unique<SensorFaceControllerPrivate>())
{
    d->q = this;
    d->configGroup = config;
    d->appearanceGroup = KConfigGroup(&config, "Appearance");
    d->sensorsGroup = KConfigGroup(&config, "Sensors");
    d->colorsGroup = KConfigGroup(&config, "SensorColors");
    d->engine = engine;
    d->syncTimer = new QTimer(this);
    d->syncTimer->setSingleShot(true);
    d->syncTimer->setInterval(5000);
    connect(d->syncTimer, &QTimer::timeout, this, [this]() {
        if (!d->shouldSync) {
            return;
        }
        d->appearanceGroup.sync();
        d->sensorsGroup.sync();
    });

    d->contextObj = new KLocalizedContext(this);

    d->totalSensors = d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("totalSensors")));
    d->lowPrioritySensorIds = d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("lowPrioritySensorIds")));
    d->highPrioritySensorIds = d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("highPrioritySensorIds")));

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

QJsonArray SensorFaceController::totalSensors() const
{
    return d->totalSensors;
}

void  SensorFaceController::setTotalSensors(const QJsonArray &totalSensors)
{
    QJsonArray resolvedSensors = d->resolveSensors(totalSensors);

    if (resolvedSensors == d->totalSensors) {
        return;
    }
    d->totalSensors = resolvedSensors;

    d->sensorsGroup.writeEntry("totalSensors", QJsonDocument(totalSensors).toJson(QJsonDocument::Compact));
    d->syncTimer->start();
    emit totalSensorsChanged();
}

QJsonArray SensorFaceController::highPrioritySensorIds() const
{
    return d->highPrioritySensorIds;
}


void SensorFaceController::setHighPrioritySensorIds(const QJsonArray &highPrioritySensorIds)
{
    QJsonArray resolvedSensors = d->resolveSensors(highPrioritySensorIds);

    if (resolvedSensors == d->highPrioritySensorIds) {
        return;
    }
    d->highPrioritySensorIds = resolvedSensors;

    d->sensorsGroup.writeEntry("highPrioritySensorIds", QJsonDocument(highPrioritySensorIds).toJson(QJsonDocument::Compact));
    d->syncTimer->start();
    emit highPrioritySensorIdsChanged();
}

QVariantMap SensorFaceController::sensorColors() const
{
    QVariantMap colors;
    for (const auto &key : d->colorsGroup.keyList()) {
        QColor color = d->colorsGroup.readEntry(key, QColor());

        if (color.isValid()) {
            colors[key] = color;
        }
    }
    return colors;
}

void SensorFaceController::setSensorColors(const QVariantMap &colors)
{
    if (colors == this->sensorColors()) {
        return;
    }

    d->colorsGroup.deleteGroup();
    d->colorsGroup = KConfigGroup(&d->configGroup, "SensorColors");

    auto it = colors.constBegin();
    for (; it != colors.constEnd(); ++it) {
        d->colorsGroup.writeEntry(it.key(), it.value());
    }

    d->syncTimer->start();
    emit sensorColorsChanged();
}

QJsonArray SensorFaceController::lowPrioritySensorIds() const
{
    return d->lowPrioritySensorIds;
}

void SensorFaceController::setLowPrioritySensorIds(const QJsonArray &lowPrioritySensorIds)
{
    QJsonArray resolvedSensors = d->resolveSensors(lowPrioritySensorIds);
    if (resolvedSensors == d->lowPrioritySensorIds) {
        return;
    }
    d->lowPrioritySensorIds = lowPrioritySensorIds;

    d->sensorsGroup.writeEntry("lowPrioritySensorIds", QJsonDocument(lowPrioritySensorIds).toJson(QJsonDocument::Compact));
    d->syncTimer->start();
    emit lowPrioritySensorIdsChanged();
}

// from face config, immutable by the user
QString SensorFaceController::name() const
{
    return d->facePackage.metadata().name();
}

const QString SensorFaceController::icon() const
{
    return d->facePackage.metadata().iconName();
}

bool SensorFaceController::supportsSensorsColors() const
{
    return d->faceProperties.readEntry("SupportsSensorsColors", false);
}

bool SensorFaceController::supportsTotalSensors() const
{
    return d->faceProperties.readEntry("SupportsTotalSensors", false);
}

bool SensorFaceController::supportsLowPrioritySensors() const
{
    return d->faceProperties.readEntry("SupportsLowPrioritySensors", false);
}

int SensorFaceController::maxTotalSensors() const
{
    return d->faceProperties.readEntry("MaxTotalSensors", 1);
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
    if (d->faceConfigUi) {
        d->faceConfigUi->deleteLater();
        d->faceConfigUi.clear();
    }

    d->faceId = face;

    d->facePackage = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KSysguard/SensorFace"), face);

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

    d->contextObj->setTranslationDomain(QLatin1String("ksysguard_face_") + face);

    d->faceProperties = KConfigGroup(KSharedConfig::openConfig(d->facePackage.filePath("FaceProperties")), QStringLiteral("Config"));

    const QString xmlPath = d->facePackage.filePath("mainconfigxml");

    if (!xmlPath.isEmpty()) {
        QFile file(xmlPath);
        KConfigGroup cg(&d->configGroup, d->faceId);

        d->faceConfigLoader = new KConfigLoader(cg, &file, this);
        d->faceConfiguration = new KDeclarative::ConfigPropertyMap(d->faceConfigLoader, this);
        d->faceConfiguration->setAutosave(d->shouldSync);
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

    d->compactRepresentation = d->createGui(d->facePackage.filePath("ui", QStringLiteral("CompactRepresentation.qml")));   
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

    const QString filePath = d->facePackage.filePath("ui", QStringLiteral("Config.qml"));

    if (filePath.isEmpty()) {
        return nullptr;
    }

    d->faceConfigUi = d->createConfigUi(QStringLiteral(":/FaceDetailsConfig.qml"),
    {{QStringLiteral("controller"), QVariant::fromValue(this)},
         {QStringLiteral("source"), filePath}});

    if (d->faceConfigUi && !d->faceConfigUi->property("item").value<QQuickItem *>()) {
        d->faceConfigUi->deleteLater();
        d->faceConfigUi.clear();
    }
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

    d->totalSensors = d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("totalSensors")));
    d->lowPrioritySensorIds = d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("lowPrioritySensorIds")));
    d->highPrioritySensorIds = d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("highPrioritySensorIds")));

    //Force to re-read all the values
    setFaceId(d->appearanceGroup.readEntry("chartFace", QStringLiteral("org.kde.ksysguard.textonly")));
    titleChanged();
    totalSensorsChanged();
    highPrioritySensorIdsChanged();
    sensorColorsChanged();
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

    auto c = KSharedConfig::openConfig(presetPackage.filePath("config", QStringLiteral("faceproperties")));
    const KConfigGroup presetGroup(c, QStringLiteral("Config"));
    const KConfigGroup colorsGroup(c, QStringLiteral("SensorColors"));

    // Load the title
    setTitle(df.readName());

    //Remove the "custon" value from presets models
    if (d->availablePresetsModel &&
        d->availablePresetsModel->data(d->availablePresetsModel->index(0, 0), PresetsModel::PluginIdRole).toString().isEmpty()) {
        d->availablePresetsModel->removeRow(0);
    }

    setTotalSensors(d->readSensors(presetGroup, QStringLiteral("totalSensors")));
    setHighPrioritySensorIds(d->readSensors(presetGroup, QStringLiteral("highPrioritySensorIds")));
    setLowPrioritySensorIds(d->readSensors(presetGroup, QStringLiteral("lowPrioritySensorIds")));

    setFaceId(presetGroup.readEntry(QStringLiteral("chartFace"), QStringLiteral("org.kde.ksysguard.piechart")));

    colorsGroup.copyTo(&d->colorsGroup);
    emit sensorColorsChanged();

    if (d->faceConfigLoader) {
        KConfigGroup presetGroup(KSharedConfig::openConfig(presetPackage.filePath("FaceProperties")), QStringLiteral("FaceConfig"));

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
    QString pluginName = QStringLiteral("org.kde.plasma.systemmonitor.") + title().simplified().replace(QLatin1Char(' '), QStringLiteral("")).toLower();
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

    KConfig c(dir.path() % QStringLiteral("/metadata.desktop"));

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

    QDir subDir(dir.path());
    subDir.mkdir(QStringLiteral("contents"));
    KConfig faceConfig(subDir.path() % QStringLiteral("/contents/faceproperties"));

    KConfigGroup configGroup(&faceConfig, "Config");
    configGroup.writeEntry(QStringLiteral("totalSensors"), QJsonDocument(totalSensors()).toJson(QJsonDocument::Compact));
    configGroup.writeEntry(QStringLiteral("highPrioritySensorIds"), QJsonDocument(highPrioritySensorIds()).toJson(QJsonDocument::Compact));
    configGroup.writeEntry(QStringLiteral("lowPrioritySensorIds"), QJsonDocument(lowPrioritySensorIds()).toJson(QJsonDocument::Compact));
    
    KConfigGroup colorsGroup(&faceConfig, "SensorColors");
    d->colorsGroup.copyTo(&colorsGroup);
    colorsGroup.sync();

    configGroup = KConfigGroup(&faceConfig, "FaceConfig");
    if (d->faceConfigLoader) {
        const auto &items = d->faceConfigLoader->items();
        for (KConfigSkeletonItem *item : items) {
            configGroup.writeEntry(item->key(), item->property());
        }
    }
    configGroup.sync();

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

bool SensorFaceController::shouldSync() const
{
    return d->shouldSync;
}

void SensorFaceController::setShouldSync(bool sync)
{
    d->shouldSync = sync;
    if (d->faceConfiguration) {
        d->faceConfiguration->setAutosave(sync);
    }
    if (!d->shouldSync && d->syncTimer->isActive()) {
        d->syncTimer->stop();
    }
}


#include "moc_SensorFaceController.cpp"
