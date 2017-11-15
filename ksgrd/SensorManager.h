/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy 
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef KSG_SENSORMANAGER_H
#define KSG_SENSORMANAGER_H

#include <kconfig.h>

#include <QtCore/QEvent>
#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QObject>
#include <QtCore/QPointer>

#include "SensorAgent.h"

namespace KSGRD {

class SensorManagerIterator;

/**
  The SensorManager handles all interaction with the connected
  hosts. Connections to a specific hosts are handled by
  SensorAgents. Use engage() to establish a connection and
  disengage() to terminate the connection.
 */
class Q_DECL_EXPORT SensorManager : public QObject
{
  Q_OBJECT

  friend class SensorManagerIterator;

  public:
    class Q_DECL_EXPORT MessageEvent : public QEvent
    {
      public:
        MessageEvent( const QString &message );

        QString message() const;

      private:
        QString mMessage;
    };

    explicit SensorManager(QObject * parent = nullptr);
    ~SensorManager() override;

    /*! Number of hosts connected to */
    int count() const;

    bool engage( const QString &hostName, const QString &shell = QStringLiteral("ssh"),
                 const QString &command = QLatin1String(""), int port = -1 );
    /* Returns true if we are connected or trying to connect to the host given
     */
    bool isConnected( const QString &hostName );
    bool disengage( SensorAgent *agent );
    bool disengage( const QString &hostName );
    bool resynchronize( const QString &hostName );
    void notify( const QString &msg ) const;

    void setBroadcaster( QWidget *wdg );

    bool sendRequest( const QString &hostName, const QString &request,
                      SensorClient *client, int id = 0 );

    const QString hostName( const SensorAgent *sensor ) const;
    bool hostInfo( const QString &host, QString &shell,
                   QString &command, int &port );

    QString translateUnit( const QString &unit ) const;
    QString translateSensorPath( const QString &path ) const;
    QString translateSensorType( const QString &type ) const;
    QString translateSensor(const QString& u) const;

    void readProperties( const KConfigGroup& cfg );
    void saveProperties( KConfigGroup& cfg );

    void disconnectClient( SensorClient *client );
    /** Call to retranslate all the strings - for example if the language has changed */
    void retranslate();

  public Q_SLOTS:
    void reconfigure( const SensorAgent *agent );

  Q_SIGNALS:
    void update();
    void hostAdded(KSGRD::SensorAgent *sensorAgent, const QString &hostName);
    void hostConnectionLost( const QString &hostName );

  protected:
    QHash<QString, SensorAgent*> mAgents;

  private:
    /**
      These dictionary stores the localized versions of the sensor
      descriptions and units.
     */
    QHash<QString, QString> mDescriptions;
    QHash<QString, QString> mUnits;
    QHash<QString, QString> mDict;
    QHash<QString, QString> mTypes;

    /** Store the data from the config file to pass to the MostConnector dialog box*/
    QStringList mHostList;
    QStringList mCommandList;

    QPointer<QWidget> mBroadcaster;
};

Q_DECL_EXPORT extern SensorManager* SensorMgr;

class Q_DECL_EXPORT SensorManagerIterator : public QHashIterator<QString, SensorAgent*>
{
  public:
    explicit SensorManagerIterator( const SensorManager *sm )
      : QHashIterator<QString, SensorAgent*>( sm->mAgents ) { }

    ~SensorManagerIterator() { }
};

}

#endif
