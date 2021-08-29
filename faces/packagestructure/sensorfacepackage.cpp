/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLocalizedString>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

class SensorFacePackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    SensorFacePackage(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : KPackage::PackageStructure(parent, args)
    {
    }

    void initPackage(KPackage::Package *package) override
    {
        package->setDefaultPackageRoot(QStringLiteral("ksysguard/sensorfaces"));

        package->addDirectoryDefinition("ui", QStringLiteral("ui"), i18n("User Interface"));

        package->addFileDefinition("CompactRepresentation",
                                   QStringLiteral("ui/CompactRepresentation.qml"),
                                   i18n("The compact representation of the sensors plasmoid when collapsed, for instance in a panel."));
        package->setRequired("CompactRepresentation", true);

        package->addFileDefinition("FullRepresentation",
                                   QStringLiteral("ui/FullRepresentation.qml"),
                                   i18n("The representation of the plasmoid when it's fully expanded."));
        package->setRequired("FullRepresentation", true);

        package->addFileDefinition("ConfigUI", QStringLiteral("ui/Config.qml"), i18n("The optional configuration page for this face."));

        package->addDirectoryDefinition("config", QStringLiteral("config"), i18n("Configuration support"));
        package->addFileDefinition("mainconfigxml", QStringLiteral("config/main.xml"), i18n("KConfigXT xml file for face-specific configuration options."));

        package->addFileDefinition("FaceProperties",
                                   QStringLiteral("faceproperties"),
                                   i18n("The configuration file that describes face properties and capabilities."));
        package->setRequired("FaceProperties", true);
    }
};

K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(SensorFacePackage, "sensorface-packagestructure.json")

#include "sensorfacepackage.moc"
