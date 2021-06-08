/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

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
#include <QTest>
#include <QObject>
#define private public
#include "cgroup.h"
#define private private

class CGroupTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testAppUnitRegex_data()
    {
        QTest::addColumn<QString>("id");
        QTest::addColumn<QString>("desktopName");
        QTest::newRow("service") << "app-gnome-org.gnome.Evince@12345.service" << "org.gnome.Evince";
        QTest::newRow("service .desktop") << "app-flatpak-org.telegram.desktop@12345.service" << "org.telegram.desktop";
        QTest::newRow("service no launcher") << "app-org.kde.okular@12345.service" << "org.kde.okular";
        QTest::newRow("service no random") << "app-KDE-org.kde.okular.service" << "org.kde.okular";
        QTest::newRow("service no launcher no random") << "app-org.kde.amarok.service" << "org.kde.amarok";
        QTest::newRow("scope") << "app-gnome-org.gnome.Evince-12345.scope" << "org.gnome.Evince";
        QTest::newRow("scope no launcher") << "app-org.gnome.Evince-12345.scope" << "org.gnome.Evince";
    }

    void testAppUnitRegex()
    {
        QFETCH(QString, id);
        QFETCH(QString, desktopName);
        KSysGuard::CGroup c(id);
        if (c.service()->menuId().isEmpty()) {
            // The service is not known on this machine and we constructed a service with the id as name
            QCOMPARE(c.service()->name(), desktopName);
        } else {
            QCOMPARE(c.service()->desktopEntryName(), desktopName.toLower());
        }
    }
};
QTEST_MAIN(CGroupTest);
#include "cgrouptest.moc"
