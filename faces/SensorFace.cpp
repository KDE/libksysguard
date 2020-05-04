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

#include "SensorFace_p.h"
#include "SensorFaceController.h"

#include <QDebug>

using namespace KSysGuard;

class SensorFace::Private {
public:
    QPointer<QQuickItem> contentItem;
    SensorFaceController *controller = nullptr;
    FormFactor formFactor = Planar;
};

SensorFace::SensorFace(QQuickItem *parent)
    : QQuickItem(parent),
      d(std::make_unique<Private>())
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
    emit formFactorChanged();
}

QQuickItem * SensorFace::contentItem() const
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

    emit contentItemChanged();
}

void SensorFace::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (d->contentItem) {
        d->contentItem->setX(0);
        d->contentItem->setY(0);
        d->contentItem->setSize(newGeometry.size());
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

#include "moc_SensorFace_p.cpp"
