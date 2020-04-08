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
#include "SensorFace.h"

#include <QtQml>
#include <QDebug>

#include <KDesktopFile>
#include <KDeclarative/ConfigPropertyMap>
#include <KPackage/PackageLoader>


class SensorFaceController::Private
{
public:
    Private();
    SensorFace *createGui(const QString &qmlPath);

    SensorFaceController *q;
    QString title;
    QQmlEngine *engine;

    KDesktopFile *faceMetadata = nullptr;
    KDeclarative::ConfigPropertyMap *faceConfiguration = nullptr;
    KConfigLoader *faceConfigLoader = nullptr;

    KPackage::Package facePackage;
    QString faceId;
    KConfigGroup configGroup;
    KConfigGroup appearanceGroup;
    KConfigGroup sensorsGroup;
    QPointer <SensorFace> fullRepresentation;
    QPointer <SensorFace> compactRepresentation;

    QTimer *syncTimer;
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
    context->setParent(gui);

    gui->setController(q);

    component->completeCreate();

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
    emit titleChanged();
}

QString SensorFaceController::totalSensor() const
{
    return d->sensorsGroup.readEntry("totalSensor", QString());
}

void SensorFaceController::setTotalSensor(const QString &totalSensor)
{
    if (totalSensor == SensorFaceController::totalSensor()) {
        return;
    }

    d->sensorsGroup.writeEntry("totalSensor", totalSensor);
    d->syncTimer->start();
    emit totalSensorChanged();
}

QStringList SensorFaceController::sensorIds() const
{
    return d->sensorsGroup.readEntry("sensorIds", QStringList());
}

void SensorFaceController::setSensorIds(const QStringList &sensorIds)
{
    if (sensorIds == SensorFaceController::sensorIds()) {
        return;
    }

    d->sensorsGroup.writeEntry("sensorIds", sensorIds);
    d->syncTimer->start();
    emit sensorIdsChanged();
}

QStringList SensorFaceController::sensorColors() const
{
    return d->sensorsGroup.readEntry("sensorColors", QStringList());
}

void SensorFaceController::setSensorColors(const QStringList &sensorColors)
{
    if (sensorColors == SensorFaceController::sensorColors()) {
        return;
    }

    d->sensorsGroup.writeEntry("sensorColors", sensorColors);
    d->syncTimer->start();
    emit sensorColorsChanged();
}

QStringList SensorFaceController::textOnlySensorIds() const
{
    return d->sensorsGroup.readEntry("textOnlySensorIds", QStringList());
}

void SensorFaceController::setTextOnlySensorIds(const QStringList &textOnlySensorIds)
{
    if (textOnlySensorIds == SensorFaceController::textOnlySensorIds()) {
        return;
    }

    d->sensorsGroup.writeEntry("textOnlySensorIds", textOnlySensorIds);
    d->syncTimer->start();
    emit textOnlySensorIdsChanged();
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

bool SensorFaceController::supportsTotalSensor() const
{
    if (!d->faceMetadata) {
        return false;
    }

    KConfigGroup cg(d->faceMetadata, QStringLiteral("Config"));
    return cg.readEntry("SupportsTotalSensor", false);
}

bool SensorFaceController::supportsTextOnlySensors() const
{
    if (!d->faceMetadata) {
        return false;
    }

    KConfigGroup cg(d->faceMetadata, QStringLiteral("Config"));
    return cg.readEntry("SupportsTextOnlySensors", false);
}

void SensorFaceController::setFaceId(const QString &face)
{
    if (d->faceId == face) {
        return;
    }

    if (d->fullRepresentation) {
        d->fullRepresentation->deleteLater();
    }
    if (d->compactRepresentation) {
        d->compactRepresentation->deleteLater();
    }

    d->faceId = face;

    d->facePackage = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/SensorApplet"), face);
qWarning()<<"AAAAAAAAAA"<<d->faceId ;
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
}

QString SensorFaceController::faceId() const
{
    return d->faceId;
}

KDeclarative::ConfigPropertyMap *SensorFaceController::faceConfig() const
{
    return d->faceConfiguration;
}

SensorFace *SensorFaceController::compactRepresentation()
{
    if (!d->facePackage.isValid()) {
        return nullptr;
    } else if (d->fullRepresentation) {
        return d->fullRepresentation;
    }

    return d->createGui(d->facePackage.filePath("ui", QStringLiteral("CompactRepresentation.qml")));   
}

SensorFace *SensorFaceController::fullRepresentation()
{
    if (!d->facePackage.isValid()) {
        return nullptr;
    } else if (d->fullRepresentation) {
        return d->fullRepresentation;
    }

    return d->createGui(d->facePackage.filePath("ui", QStringLiteral("FullRepresentation.qml")));   
}


#include "moc_SensorFaceController.cpp"
