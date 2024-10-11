/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "FaceLoader.h"
#include <QQmlEngine>

using namespace KSysGuard;

class Q_DECL_HIDDEN FaceLoader::Private
{
public:
    Private(FaceLoader *qq)
        : q(qq)
    {
    }
    void setupController();

    FaceLoader *q;

    SensorFaceController *parentController = nullptr;
    SensorFaceController *controller = nullptr;

    QString groupName;

    bool complete = false;

    QJsonArray sensors;
    QString faceId;
    QVariantMap colors;
    QVariantMap labels;
    int updateRateLimit = 0;
    bool readOnly = true;
    bool showTitle = false;
};

FaceLoader::FaceLoader(QObject *parent)
    : QObject(parent)
    , d(new Private{this})
{
}

FaceLoader::~FaceLoader() = default;

SensorFaceController *FaceLoader::parentController() const
{
    return d->parentController;
}

void FaceLoader::setParentController(SensorFaceController *newParentController)
{
    if (newParentController == d->parentController) {
        return;
    }

    if (d->parentController) {
        d->parentController->disconnect(this);
    }

    if (d->controller) {
        d->controller->deleteLater();
    }

    d->parentController = newParentController;

    d->setupController();

    Q_EMIT parentControllerChanged();
}

QString FaceLoader::faceId() const
{
    return d->faceId;
}

void FaceLoader::setFaceId(const QString &newFaceId)
{
    if (newFaceId == d->faceId) {
        return;
    }

    d->faceId = newFaceId;
    if (d->controller) {
        d->controller->setFaceId(d->faceId);
    }

    Q_EMIT faceIdChanged();
}

QString FaceLoader::groupName() const
{
    return d->groupName;
}

void FaceLoader::setGroupName(const QString &newGroupName)
{
    if (newGroupName == d->groupName) {
        return;
    }

    d->groupName = newGroupName;

    d->setupController();

    Q_EMIT groupNameChanged();
}

QJsonArray FaceLoader::sensors() const
{
    return d->sensors;
}

void FaceLoader::setSensors(const QJsonArray &newSensors)
{
    if (newSensors == d->sensors) {
        return;
    }

    d->sensors = newSensors;

    if (d->controller) {
        d->controller->setHighPrioritySensorIds(d->sensors);
    }

    Q_EMIT sensorsChanged();
}

QVariantMap FaceLoader::colors() const
{
    return d->colors;
}

void FaceLoader::setColors(const QVariantMap &newColors)
{
    if (newColors == d->colors) {
        return;
    }

    d->colors = newColors;
    if (d->controller) {
        d->controller->setSensorColors(d->colors);
        // Ensure we emit a change signal for colors even if the controller thinks
        // we shouldn't, to ensure all instances of the face are correctly updated.
        Q_EMIT d->controller->sensorColorsChanged();
    }
    Q_EMIT colorsChanged();
}

QVariantMap FaceLoader::labels() const
{
    return d->labels;
}

void FaceLoader::setLabels(const QVariantMap &newLabels)
{
    if (newLabels == d->labels) {
        return;
    }

    d->labels = newLabels;
    if (d->controller) {
        d->controller->setSensorLabels(d->labels);
        // Ensure we emit a change signal for labels even if the controller thinks
        // we shouldn't, to ensure all instances of the face are correctly updated.
        Q_EMIT d->controller->sensorLabelsChanged();
    }
    Q_EMIT labelsChanged();
}

int FaceLoader::updateRateLimit() const
{
    return d->updateRateLimit;
}

void FaceLoader::setUpdateRateLimit(int newLimit)
{
    if (newLimit == d->updateRateLimit) {
        return;
    }

    d->updateRateLimit = newLimit;
    if (d->controller) {
        d->controller->setUpdateRateLimit(d->updateRateLimit);
        // Ensure we emit a change signal for the limit even if the controller thinks
        // we shouldn't, to ensure all instances of the face are correctly updated.
        Q_EMIT d->controller->updateRateLimitChanged();
    }
    Q_EMIT updateRateLimitChanged();
}

bool FaceLoader::readOnly() const
{
    return d->readOnly;
}

void FaceLoader::setReadOnly(bool newReadOnly)
{
    if (newReadOnly == d->readOnly) {
        return;
    }

    d->readOnly = newReadOnly;
    if (d->controller) {
        d->controller->setShouldSync(!d->readOnly);
    }
    Q_EMIT readOnlyChanged();
}

SensorFaceController *FaceLoader::controller() const
{
    return d->controller;
}

void FaceLoader::reload()
{
    d->controller->reloadFaceConfiguration();
}

void FaceLoader::classBegin()
{
}

void FaceLoader::componentComplete()
{
    d->complete = true;
    d->setupController();
}

void FaceLoader::Private::setupController()
{
    if (!parentController || groupName.isEmpty() || !complete) {
        return;
    }

    auto configGroup = parentController->configGroup().group(groupName);
    controller = new SensorFaceController(configGroup, qmlEngine(q), new QQmlEngine(q));
    controller->setShouldSync(readOnly);
    controller->setHighPrioritySensorIds(sensors);
    controller->setSensorColors(colors);
    controller->setSensorLabels(labels);
    controller->setShowTitle(showTitle);
    controller->setFaceId(faceId);
    controller->setUpdateRateLimit(updateRateLimit);

    Q_EMIT q->controllerChanged();
}
