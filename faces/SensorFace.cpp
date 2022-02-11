/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorFaceController.h"
#include "SensorFace_p.h"

#include <QDebug>

using namespace KSysGuard;

class SensorFace::Private
{
public:
    QPointer<QQuickItem> contentItem;
    SensorFaceController *controller = nullptr;
    FormFactor formFactor = Planar;
};

SensorFace::SensorFace(QQuickItem *parent)
    : QQuickItem(parent)
    , d(std::make_unique<Private>())
{
}

SensorFace::~SensorFace()
{
}

SensorFaceController *SensorFace::controller() const
{
    return d->controller;
}

// Not writable from QML
void SensorFace::setController(SensorFaceController *controller)
{
    d->controller = controller;
}

SensorFace::FormFactor SensorFace::formFactor() const
{
    return d->formFactor;
}

void SensorFace::setFormFactor(SensorFace::FormFactor formFactor)
{
    if (d->formFactor == formFactor) {
        return;
    }

    d->formFactor = formFactor;
    Q_EMIT formFactorChanged();
}

QQuickItem *SensorFace::contentItem() const
{
    return d->contentItem;
}

void SensorFace::setContentItem(QQuickItem *item)
{
    if (d->contentItem == item) {
        return;
    }
    d->contentItem = item;

    if (d->contentItem) {
        d->contentItem->setParentItem(this);
        d->contentItem->setX(0);
        d->contentItem->setY(0);
        d->contentItem->setSize(size());
    }

    Q_EMIT contentItemChanged();
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void SensorFace::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
#else
void SensorFace::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
#endif
{
    if (d->contentItem) {
        d->contentItem->setX(0);
        d->contentItem->setY(0);
        d->contentItem->setSize(newGeometry.size());
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
#else
    QQuickItem::geometryChange(newGeometry, oldGeometry);
#endif
}

#include "moc_SensorFace_p.cpp"
