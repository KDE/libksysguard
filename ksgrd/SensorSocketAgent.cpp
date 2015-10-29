/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

//#include <stdlib.h>

#include "processcore/processcore_debug.h"
#include <QDebug>
#include <klocalizedstring.h>

#include "SensorClient.h"
#include "SensorManager.h"

#include "SensorSocketAgent.h"

using namespace KSGRD;

SensorSocketAgent::SensorSocketAgent( SensorManager *sm )
  : SensorAgent( sm )
{

  connect( &mSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)) );
  connect( &mSocket, &QIODevice::bytesWritten, this, &SensorSocketAgent::msgSent );
  connect( &mSocket, &QIODevice::readyRead, this, &SensorSocketAgent::msgRcvd );
  connect( &mSocket, &QAbstractSocket::disconnected, this, &SensorSocketAgent::connectionClosed );
}

SensorSocketAgent::~SensorSocketAgent()
{
  mSocket.write( "quit\n", sizeof( "quit\n" ) );
  mSocket.flush();
}
	
bool SensorSocketAgent::start( const QString &host, const QString&,
                               const QString&, int port )
{
  if ( port <= 0 )
    qCDebug(LIBKSYSGUARD) << "SensorSocketAgent::start: Invalid port " << port;

  setHostName( host );
  mPort = port;

  mSocket.connectToHost( hostName(), mPort );

  return true;
}

void SensorSocketAgent::hostInfo( QString &shell, QString &command, int &port ) const
{
  shell.clear();
  command.clear();
  port = mPort;
}

void SensorSocketAgent::msgSent( )
{
  if ( mSocket.bytesToWrite() != 0 )
    return;

  // Try to send next request if available.
  executeCommand();
}

void SensorSocketAgent::msgRcvd()
{
  int buflen = mSocket.bytesAvailable();
  char* buffer = new char[ buflen ];

  mSocket.read( buffer, buflen );

  processAnswer( buffer, buflen ); 
  delete [] buffer;
}

void SensorSocketAgent::connectionClosed()
{
  setDaemonOnLine( false );
  if(sensorManager()) {
    sensorManager()->disengage( this ); //delete ourselves
  }
}

void SensorSocketAgent::error( QAbstractSocket::SocketError id )
{
  switch ( id ) {
    case QAbstractSocket::ConnectionRefusedError:
      SensorMgr->notify( i18n( "Connection to %1 refused" ,
                           hostName() ) );
      break;
    case QAbstractSocket::HostNotFoundError:
      SensorMgr->notify( i18n( "Host %1 not found" ,
                           hostName() ) );
      break;
    case QAbstractSocket::NetworkError:
      SensorMgr->notify( i18n( "An error occurred with the network (e.g. the network cable was accidentally unplugged) for host %1.",
                           hostName() ) );
      break;
    default:
      SensorMgr->notify( i18n( "Error for host %1: %2",
                           hostName(), mSocket.errorString() ) );
  }

  setDaemonOnLine( false );
  if(sensorManager())
    sensorManager()->disengage( this );
}

bool SensorSocketAgent::writeMsg( const char *msg, int len )
{
  int writtenLength = mSocket.write( msg, len );
  mSocket.flush();
  return writtenLength == len;
}


