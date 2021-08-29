/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorFaceController.h"
#include "SensorFaceController_p.h"
#include "SensorFace_p.h"
#include <Sensor.h>
#include <SensorQuery.h>

#include <QtQml>

#include <KConfigLoader>
#include <KDeclarative/ConfigPropertyMap>
#include <KDesktopFile>
#include <KLocalizedString>
#include <KPackage/PackageLoader>
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
    QList<KPluginMetaData> plugins =
        KPackage::PackageLoader::self()->findPackages(QStringLiteral("Plasma/Applet"), QString(), [](const KPluginMetaData &plugin) {
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

        KConfigGroup configGroup(KSharedConfig::openConfig(p.filePath("config", QStringLiteral("faceproperties")), KConfig::SimpleConfig),
                                 QStringLiteral("Config"));

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
            {QRegularExpression(QStringLiteral("network/interfaces/(.*)")), QStringLiteral("network/\\1")},
            {QRegularExpression(QStringLiteral("network/all/receivedDataRate$")), QStringLiteral("network/all/download")},
            {QRegularExpression(QStringLiteral("network/all/sentDataRate$")), QStringLiteral("network/all/upload")},
            {QRegularExpression(QStringLiteral("network/all/totalReceivedData$")), QStringLiteral("network/all/totalDownload")},
            {QRegularExpression(QStringLiteral("network/all/totalSentData$")), QStringLiteral("network/all/totalUpload")},
            {QRegularExpression(QStringLiteral("(.*)/receiver/data$")), QStringLiteral("\\1/download")},
            {QRegularExpression(QStringLiteral("(.*)/transmitter/data$")), QStringLiteral("\\1/upload")},
            {QRegularExpression(QStringLiteral("(.*)/receiver/dataTotal$")), QStringLiteral("\\1/totalDownload")},
            {QRegularExpression(QStringLiteral("(.*)/transmitter/dataTotal$")), QStringLiteral("\\1/totalUpload")},
            {QRegularExpression(QStringLiteral("(.*)/Rate/rio")), QStringLiteral("\\1/read")},
            {QRegularExpression(QStringLiteral("(.*)/Rate/wio$")), QStringLiteral("\\1/write")},
            {QRegularExpression(QStringLiteral("(.*)/freespace$")), QStringLiteral("\\1/free")},
            {QRegularExpression(QStringLiteral("(.*)/filllevel$")), QStringLiteral("\\1/usedPercent")},
            {QRegularExpression(QStringLiteral("(.*)/usedspace$")), QStringLiteral("\\1/used")},
            {QRegularExpression(QStringLiteral("cpu/system/(.*)$")), QStringLiteral("cpu/all/\\1")},
            {QRegularExpression(QStringLiteral("cpu/(.*)/sys$")), QStringLiteral("cpu/\\1/system")},
            {QRegularExpression(QStringLiteral("cpu/(.*)/TotalLoad$")), QStringLiteral("cpu/\\1/usage")},
            {QRegularExpression(QStringLiteral("cpu/cpu(\\d+)/clock$")), QStringLiteral("cpu/cpu\\1/frequency")},
            {QRegularExpression(QStringLiteral("mem/(.*)level")), QStringLiteral("mem/\\1Percent")},
            {QRegularExpression(QStringLiteral("mem/physical/allocated")), QStringLiteral("memory/physical/used")},
            {QRegularExpression(QStringLiteral("mem/physical/available")), QStringLiteral("memory/physical/free")},
            {QRegularExpression(QStringLiteral("mem/physical/buf")), QStringLiteral("memory/physical/buffer")},
            {QRegularExpression(QStringLiteral("mem/physical/cached")), QStringLiteral("memory/physical/cache")},
            {QRegularExpression(QStringLiteral("^mem/(.*)")), QStringLiteral("memory/\\1")},
            {QRegularExpression(QStringLiteral("nvidia/(.*)/temperature$")), QStringLiteral("gpu/\\1/temperature")},
            {QRegularExpression(QStringLiteral("nvidia/(.*)/memoryClock$")), QStringLiteral("gpu/\\1/memoryFrequency")},
            {QRegularExpression(QStringLiteral("nvidia/(.*)/processorClock$")), QStringLiteral("gpu/\\1/coreFrequency")},
            {QRegularExpression(QStringLiteral("nvidia/(.*)/(memory|sharedMemory)$")), QStringLiteral("gpu/\\1/usedVram")},
            {QRegularExpression(QStringLiteral("nvidia/(.*)/(encoderUsage|decoderUsage)$")), QStringLiteral("gpu/\\1/usage")},
            {QRegularExpression(QStringLiteral("^(uptime|system/uptime/uptime)$")), QStringLiteral("os/system/uptime")},
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

QJsonArray SensorFaceControllerPrivate::readAndUpdateSensors(KConfigGroup &config, const QString &entryName)
{
    auto original = QJsonDocument::fromJson(config.readEntry(entryName, QString()).toUtf8()).array();

    const KConfigGroup &group = config;
    auto newSensors = readSensors(group, entryName);

    if (newSensors != original) {
        config.writeEntry(entryName, QJsonDocument(newSensors).toJson(QJsonDocument::Compact));
    }

    return newSensors;
}

void SensorFaceControllerPrivate::resolveSensors(const QJsonArray &partialEntries, std::function<void(const QJsonArray &)> callback)
{
    if (partialEntries.isEmpty()) {
        callback(partialEntries);
        return;
    }

    auto sensors = std::make_shared<QJsonArray>();

    for (const auto &id : partialEntries) {
        auto query = new KSysGuard::SensorQuery{id.toString()};
        query->connect(query, &KSysGuard::SensorQuery::finished, q, [this, query, sensors, callback] {
            query->sortByName();
            const auto ids = query->sensorIds();
            delete query;
            std::transform(ids.begin(), ids.end(), std::back_inserter(*sensors), [](const QString &id) {
                return QJsonValue(id);
            });
            if (sensors.use_count() == 1) {
                // We are the last query
                callback(*sensors);
            }
        });
        query->execute();
    }
}

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
        qWarning() << "ERROR: QML gui" << guiObject << "not a SensorFace instance";
        guiObject->deleteLater();
        context->deleteLater();
        return nullptr;
    }
    context->setParent(gui);

    gui->setController(q);
    gui->setParent(q);

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
    QObject *guiObject = component->createWithInitialProperties(initialProperties, context);
    QQuickItem *gui = qobject_cast<QQuickItem *>(guiObject);
    Q_ASSERT(gui);
    context->setParent(gui);
    gui->setParent(q);

    component->deleteLater();

    return gui;
}

SensorFaceController::SensorFaceController(KConfigGroup &config, QQmlEngine *engine)
    : QObject(engine)
    , d(std::make_unique<SensorFaceControllerPrivate>())
{
    d->q = this;
    d->configGroup = config;
    d->appearanceGroup = KConfigGroup(&config, "Appearance");
    d->sensorsGroup = KConfigGroup(&config, "Sensors");
    d->colorsGroup = KConfigGroup(&config, "SensorColors");
    d->labelsGroup = KConfigGroup(&config, "SensorLabels");
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

    d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("totalSensors")), [this](const QJsonArray &resolvedSensors) {
        d->totalSensors = resolvedSensors;
        Q_EMIT totalSensorsChanged();
    });
    d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("lowPrioritySensorIds")), [this](const QJsonArray &resolvedSensors) {
        d->lowPrioritySensorIds = resolvedSensors;
        Q_EMIT lowPrioritySensorIdsChanged();
    });
    d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("highPrioritySensorIds")), [this](const QJsonArray &resolvedSensors) {
        d->highPrioritySensorIds = resolvedSensors;
        Q_EMIT highPrioritySensorIdsChanged();
    });

    setFaceId(d->appearanceGroup.readEntry("chartFace", QStringLiteral("org.kde.ksysguard.piechart")));
}

SensorFaceController::~SensorFaceController()
{
    auto forceSave = d->faceProperties.readEntry(QStringLiteral("ForceSaveOnDestroy"), false);
    if (!forceSave) {
        if (!d->shouldSync) {
            // If we should not sync automatically, clear all changes before we
            // destroy the config objects, otherwise they will be written during
            // destruction.
            d->appearanceGroup.markAsClean();
            d->colorsGroup.markAsClean();
            d->labelsGroup.markAsClean();
            if (d->faceConfigLoader && d->faceConfigLoader->isSaveNeeded()) {
                d->faceConfigLoader->load();
            }
        }
    } else {
        d->faceConfigLoader->save();
    }
}

KConfigGroup KSysGuard::SensorFaceController::configGroup() const
{
    return d->configGroup;
}

QString SensorFaceController::title() const
{
    // both Title and title can exist to allow i18n of Title
    if (d->appearanceGroup.hasKey("title")) {
        return d->appearanceGroup.readEntry("title");
    } else {
        // if neither exist fall back to name
        return d->appearanceGroup.readEntry("Title", i18n("System Monitor Sensor"));
    }
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

bool SensorFaceController::showTitle() const
{
    return d->appearanceGroup.readEntry("showTitle", true);
}

void SensorFaceController::setShowTitle(bool show)
{
    if (show == showTitle()) {
        return;
    }

    d->appearanceGroup.writeEntry("showTitle", show);
    d->syncTimer->start();

    emit showTitleChanged();
}

QJsonArray SensorFaceController::totalSensors() const
{
    return d->totalSensors;
}

void SensorFaceController::setTotalSensors(const QJsonArray &totalSensors)
{
    if (totalSensors == d->totalSensors) {
        return;
    }
    const auto currentEntry = QJsonDocument::fromJson(d->sensorsGroup.readEntry("totalSensors").toUtf8()).array();
    if (totalSensors == currentEntry) {
        return;
    }
    d->sensorsGroup.writeEntry("totalSensors", QJsonDocument(totalSensors).toJson(QJsonDocument::Compact));
    // Until we have resolved
    d->totalSensors = totalSensors;
    d->syncTimer->start();
    Q_EMIT totalSensorsChanged();
    d->resolveSensors(totalSensors, [this](const QJsonArray &resolvedSensors) {
        if (resolvedSensors == d->totalSensors) {
            return;
        }
        d->totalSensors = resolvedSensors;
        Q_EMIT totalSensorsChanged();
    });
}

QJsonArray SensorFaceController::highPrioritySensorIds() const
{
    return d->highPrioritySensorIds;
}

void SensorFaceController::setHighPrioritySensorIds(const QJsonArray &highPrioritySensorIds)
{
    if (highPrioritySensorIds == d->highPrioritySensorIds) {
        return;
    }
    const auto currentEntry = QJsonDocument::fromJson(d->sensorsGroup.readEntry("highPrioritySensorIds").toUtf8()).array();
    if (highPrioritySensorIds == currentEntry) {
        return;
    }
    d->sensorsGroup.writeEntry("highPrioritySensorIds", QJsonDocument(highPrioritySensorIds).toJson(QJsonDocument::Compact));
    // Until we have resolved
    d->syncTimer->start();
    d->highPrioritySensorIds = highPrioritySensorIds;
    Q_EMIT highPrioritySensorIdsChanged();
    d->resolveSensors(highPrioritySensorIds, [this](const QJsonArray &resolvedSensors) {
        if (resolvedSensors == d->highPrioritySensorIds) {
            return;
        }
        d->highPrioritySensorIds = resolvedSensors;
        Q_EMIT highPrioritySensorIdsChanged();
    });
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

QVariantMap SensorFaceController::sensorLabels() const
{
    QVariantMap labels;
    for (const auto &key : d->labelsGroup.keyList()) {
        labels[key] = d->labelsGroup.readEntry(key);
    }
    return labels;
}

void SensorFaceController::setSensorLabels(const QVariantMap &labels)
{
    if (labels == this->sensorLabels()) {
        return;
    }

    d->labelsGroup.deleteGroup();
    d->labelsGroup = KConfigGroup(&d->configGroup, "SensorLabels");

    for (auto it = labels.cbegin(); it != labels.cend(); ++it) {
        const auto label = it.value().toString();
        if (!label.isEmpty()) {
            d->labelsGroup.writeEntry(it.key(), label);
        }
    }

    d->syncTimer->start();
    emit sensorLabelsChanged();
}

QJsonArray SensorFaceController::lowPrioritySensorIds() const
{
    return d->lowPrioritySensorIds;
}

void SensorFaceController::setLowPrioritySensorIds(const QJsonArray &lowPrioritySensorIds)
{
    if (lowPrioritySensorIds == d->lowPrioritySensorIds) {
        return;
    }
    const auto currentEntry = QJsonDocument::fromJson(d->sensorsGroup.readEntry("lowPrioritySensorIds").toUtf8()).array();
    if (lowPrioritySensorIds == currentEntry) {
        return;
    }
    d->sensorsGroup.writeEntry("lowPrioritySensorIds", QJsonDocument(lowPrioritySensorIds).toJson(QJsonDocument::Compact));
    // Until we have resolved
    d->lowPrioritySensorIds = lowPrioritySensorIds;
    d->syncTimer->start();
    Q_EMIT lowPrioritySensorIdsChanged();
    d->resolveSensors(lowPrioritySensorIds, [this](const QJsonArray &resolvedSensors) {
        if (resolvedSensors == d->lowPrioritySensorIds) {
            return;
        }
        d->lowPrioritySensorIds = resolvedSensors;
        Q_EMIT lowPrioritySensorIdsChanged();
    });
}

int SensorFaceController::updateRateLimit() const
{
    return d->appearanceGroup.readEntry<int>(QStringLiteral("updateRateLimit"), 0);
}

void SensorFaceController::setUpdateRateLimit(int limit)
{
    if (limit == updateRateLimit()) {
        return;
    }

    d->appearanceGroup.writeEntry("updateRateLimit", limit);
    d->syncTimer->start();

    Q_EMIT updateRateLimitChanged();
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
        d->compactRepresentation.clear();
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
        d->faceConfigLoader->deleteLater();
        d->faceConfigLoader = nullptr;
    }

    if (!d->facePackage.isValid()) {
        emit faceIdChanged();
        return;
    }

    d->contextObj->setTranslationDomain(QLatin1String("ksysguard_face_") + face);

    d->faceProperties = KConfigGroup(KSharedConfig::openConfig(d->facePackage.filePath("FaceProperties"), KConfig::SimpleConfig), QStringLiteral("Config"));

    reloadFaceConfiguration();

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
                                        {{QStringLiteral("controller"), QVariant::fromValue(this)}, {QStringLiteral("source"), filePath}});

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

    if (d->faceProperties.readEntry("SupportsSensors", true)) {
        d->sensorsConfigUi = d->createConfigUi(QStringLiteral(":/ConfigSensors.qml"), {{QStringLiteral("controller"), QVariant::fromValue(this)}});
    } else {
        d->sensorsConfigUi = new QQuickItem;
    }
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

    d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("totalSensors")), [this](const QJsonArray &resolvedSensors) {
        d->totalSensors = resolvedSensors;
        Q_EMIT totalSensorsChanged();
    });
    d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("lowPrioritySensorIds")), [this](const QJsonArray &resolvedSensors) {
        d->lowPrioritySensorIds = resolvedSensors;
        Q_EMIT lowPrioritySensorIdsChanged();
    });
    d->resolveSensors(d->readAndUpdateSensors(d->sensorsGroup, QStringLiteral("highPrioritySensorIds")), [this](const QJsonArray &resolvedSensors) {
        d->highPrioritySensorIds = resolvedSensors;
        Q_EMIT highPrioritySensorIdsChanged();
    });

    // Force to re-read all the values
    setFaceId(d->appearanceGroup.readEntry("chartFace", QStringLiteral("org.kde.ksysguard.textonly")));
    Q_EMIT titleChanged();
    Q_EMIT sensorColorsChanged();
    Q_EMIT sensorLabelsChanged();
    Q_EMIT showTitleChanged();
    Q_EMIT updateRateLimitChanged();
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

    auto c = KSharedConfig::openConfig(presetPackage.filePath("config", QStringLiteral("faceproperties")), KConfig::SimpleConfig);
    const KConfigGroup presetGroup(c, QStringLiteral("Config"));
    const KConfigGroup colorsGroup(c, QStringLiteral("SensorColors"));

    // Load the title
    setTitle(df.readName());

    // Remove the "custon" value from presets models
    if (d->availablePresetsModel && d->availablePresetsModel->data(d->availablePresetsModel->index(0, 0), PresetsModel::PluginIdRole).toString().isEmpty()) {
        d->availablePresetsModel->removeRow(0);
    }

    setTotalSensors(d->readSensors(presetGroup, QStringLiteral("totalSensors")));
    setHighPrioritySensorIds(d->readSensors(presetGroup, QStringLiteral("highPrioritySensorIds")));
    setLowPrioritySensorIds(d->readSensors(presetGroup, QStringLiteral("lowPrioritySensorIds")));

    setFaceId(presetGroup.readEntry(QStringLiteral("chartFace"), QStringLiteral("org.kde.ksysguard.piechart")));

    colorsGroup.copyTo(&d->colorsGroup);
    emit sensorColorsChanged();

    if (d->faceConfigLoader) {
        KConfigGroup presetGroup(KSharedConfig::openConfig(presetPackage.filePath("FaceProperties"), KConfig::SimpleConfig), QStringLiteral("FaceConfig"));

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

    connect(job, &KJob::finished, this, [this, pluginName]() {
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

    connect(job, &KJob::finished, this, [this]() {
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

void SensorFaceController::reloadFaceConfiguration()
{
    const QString xmlPath = d->facePackage.filePath("mainconfigxml");

    if (!xmlPath.isEmpty()) {
        QFile file(xmlPath);
        KConfigGroup cg(&d->configGroup, d->faceId);

        if (d->faceConfigLoader) {
            delete d->faceConfigLoader;
        }

        if (d->faceConfiguration) {
            delete d->faceConfiguration;
        }

        d->faceConfigLoader = new KConfigLoader(cg, &file, this);
        d->faceConfiguration = new KDeclarative::ConfigPropertyMap(d->faceConfigLoader, this);
        d->faceConfiguration->setAutosave(d->shouldSync);
        connect(d->faceConfiguration, &KDeclarative::ConfigPropertyMap::valueChanged, this, [this](const QString &key) {
            auto item = d->faceConfigLoader->findItemByName(key);
            if (item) {
                item->writeConfig(d->faceConfigLoader->config());
            }
        });

        Q_EMIT faceConfigurationChanged();
    }
}

#include "moc_SensorFaceController.cpp"
