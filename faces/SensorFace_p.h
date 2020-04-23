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

#pragma once

#include <QQuickItem>

#include "faces_export.h"

class SensorFaceController;

class FACES_EXPORT SensorFace : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(SensorFaceController *controller READ controller CONSTANT)
    Q_PROPERTY(SensorFace::FormFactor formFactor READ formFactor WRITE setFormFactor NOTIFY formFactorChanged)
    Q_PROPERTY(QQuickItem *contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)

public:
    enum FormFactor {
        Planar,
        Vertical,
        Horizontal
    };
    Q_ENUM(FormFactor)

    SensorFace(QQuickItem *parent = nullptr);
    ~SensorFace();

    SensorFaceController *controller() const;
    // Not writable from QML
    void setController(SensorFaceController *controller);

    SensorFace::FormFactor formFactor() const;
    void setFormFactor(SensorFace::FormFactor formFactor);

    QQuickItem * contentItem() const;
    void setContentItem(QQuickItem *item);

Q_SIGNALS:
    void formFactorChanged();
    void contentItemChanged();

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    class Private;
    const std::unique_ptr<Private> d;
};
