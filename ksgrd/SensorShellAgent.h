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

#ifndef KSG_SENSORSHELLAGENT_H
#define KSG_SENSORSHELLAGENT_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QProcess>

#include "SensorAgent.h"

class QString;

class KProcess;

namespace KSGRD {

class SensorManager;

/**
  The SensorShellAgent starts a ksysguardd process and handles the
  asynchronous communication. It keeps a list of pending requests
  that have not been answered yet by ksysguard. The current
  implementation only allowes one pending requests. Incoming requests
  are queued in an input FIFO.
 */
class SensorShellAgent : public SensorAgent
{
  Q_OBJECT

  public:
    explicit SensorShellAgent( SensorManager *sm );
    ~SensorShellAgent() override;

    bool start( const QString &host, const QString &shell,
                const QString &command = QLatin1String(QLatin1String("")), int port = -1 ) override;

    void hostInfo( QString &shell, QString &command, int &port) const override;

  private Q_SLOTS:
    void msgRcvd( );
    void errMsgRcvd( );
    void daemonExited(  int exitCode, QProcess::ExitStatus exitStatus );
    void daemonError( QProcess::ProcessError errorStatus );

  private:
    bool writeMsg( const char *msg, int len ) override;
    int mRetryCount;
    QPointer<KProcess> mDaemon;
    QString mShell;
    QString mCommand;
};

}
	
#endif
