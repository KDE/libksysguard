/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "systemstats/SensorContainer.h"
#include "systemstats/SensorObject.h"
#include "systemstats/SensorPlugin.h"

using namespace KSysGuard;

class SensorContainerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void deletedObjectIsRemovedFromContainer();
    void destroyedHandlerDoesNotEvictReplacement();
    void objectDeletedBeforeQueuedRegistration();
};

// A SensorObject deleted without going through aboutToBeRemoved/removeObject (as
// happens when a plugin drops a device) must not leave a dangling pointer in its
// container, or iterating objects() and reading their ids crashes (bug 523562).
void SensorContainerTest::deletedObjectIsRemovedFromContainer()
{
    SensorPlugin plugin(nullptr, {});
    auto container = new SensorContainer(QStringLiteral("test"), QStringLiteral("Test"), &plugin);

    auto object = new SensorObject(QStringLiteral("obj"), container);
    // Registration with the container is queued from the constructor.
    QTRY_COMPARE(container->objects().size(), 1);

    delete object;

    QCOMPARE(container->objects().size(), 0);
    // The exact operation AggregateSensor::updateSensors() performs; a use-after-free
    // before the fix.
    for (auto obj : container->objects()) {
        obj->id();
    }
}

// The destroyed() cleanup keys on the object id, so it must compare identity and not
// evict a replacement object that reused the same id.
void SensorContainerTest::destroyedHandlerDoesNotEvictReplacement()
{
    SensorPlugin plugin(nullptr, {});
    auto container = new SensorContainer(QStringLiteral("test"), QStringLiteral("Test"), &plugin);

    auto first = new SensorObject(QStringLiteral("dup"), container);
    QTRY_COMPARE(container->objects().size(), 1);

    // Detach the first object (still alive), then register a replacement with the same id.
    container->removeObject(first);
    QCOMPARE(container->objects().size(), 0);

    auto second = new SensorObject(QStringLiteral("dup"), container);
    QTRY_COMPARE(container->object(QStringLiteral("dup")), second);

    // Destroying the detached first object must not evict the replacement.
    delete first;
    QCOMPARE(container->object(QStringLiteral("dup")), second);
}

// A SensorObject deleted before its queued registration runs must not cause
// addObject() to be called on a dangling pointer.
void SensorContainerTest::objectDeletedBeforeQueuedRegistration()
{
    SensorPlugin plugin(nullptr, {});
    auto container = new SensorContainer(QStringLiteral("test"), QStringLiteral("Test"), &plugin);

    auto object = new SensorObject(QStringLiteral("ephemeral"), container);
    delete object;

    // Drain the event loop; the queued addObject() must have been discarded.
    QTest::qWait(50);
    QCOMPARE(container->objects().size(), 0);
}

QTEST_GUILESS_MAIN(SensorContainerTest)

#include "sensorcontainertest.moc"
