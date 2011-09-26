/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    Copyright (c) 2006 John Tapsell <tapsell@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_SENSORCLIENT_H
#define KSG_SENSORCLIENT_H

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QString>

namespace KSGRD {

/**
  Every object that should act as a client to a sensor must inherit from
  this class. A pointer to the client object is passed as SensorClient*
  to the SensorAgent. When the requested information is available or a
  problem occurred one of the member functions is called.
 */
class SensorClient
{
  public:
    explicit SensorClient() { }
    virtual ~SensorClient() { }

    /**
      This function is called whenever the information from the sensor has
      been received by the sensor agent. This function must be reimplemented
      by the sensor client to receive and process this information.
     */
    virtual void answerReceived( int id, const QList<QByteArray>& answer) {
        Q_UNUSED(id);
        Q_UNUSED(answer);
    }

    /**
      In case of an unexpected fatal problem with the sensor the sensor
      agent will call this function to notify the client about it.
     */
    virtual void sensorLost( int id ) {
        Q_UNUSED(id);
    }
};

/**
  The following classes are utility classes that provide a
  convenient way to retrieve pieces of information from the sensor
  answers. For each type of answer there is a separate class.
 */
class SensorTokenizer
{
  public:
    SensorTokenizer( const QByteArray &info, char separator )
    {
      if ( separator == '/' ) {
        //This is a special case where we assume that info is a '\' escaped string

        int i=0;
        int lastTokenAt = -1;

        for( ; i < info.length(); ++i ) {
          if( info[i] == '\\' ) {
            ++i;
          }
          else if ( info[i] == separator ) {
            mTokens.append( unEscapeString( info.mid( lastTokenAt + 1, i - lastTokenAt - 1 ) ) );
            lastTokenAt = i;
          }
        }

        //Add everything after the last token
        mTokens.append( unEscapeString( info.mid( lastTokenAt + 1, i - lastTokenAt - 1 ) ) );
      }
      else {
        mTokens = info.split( separator );
      }
    }

    ~SensorTokenizer() { }

    const QByteArray& operator[]( unsigned idx )
    {
      Q_ASSERT(idx < (unsigned)(mTokens.count()));
      return mTokens[ idx ];
    }

    uint count()
    {
      return mTokens.count();
    }

  private:
    QList<QByteArray> mTokens;

    QByteArray unEscapeString( QByteArray string ) {

      int i=0;
      for( ; i < string.length(); ++i ) {
        if( string[i] == '\\' ) {
          string.remove( i, 1 );
          ++i;
        }
      }

      return string;
    }
};

/**
  An integer info contains 4 fields separated by TABS, a description
  (name), the minimum and the maximum values and the unit.
  e.g. Swap Memory	0	133885952	KB
 */
class SensorIntegerInfo : public SensorTokenizer
{
  public:
    explicit SensorIntegerInfo( const QByteArray &info )
      : SensorTokenizer( info, '\t' ) { }

    ~SensorIntegerInfo() { }

    QString name()
    {
      if (count() > 0)
          return QString::fromUtf8((*this)[ 0 ]);
      return QString();
    }

    long long min()
    {
      if (count() > 1)
          return (*this)[ 1 ].toLongLong();
      return -1;
    }

    long long max()
    {
      if (count() > 2)
          return (*this)[ 2 ].toLongLong();
      return -1;
    }

    QString unit()
    {
      if (count() > 3)
          return QString::fromUtf8((*this)[ 3 ]);
      return QString();
    }
};

/**
  An float info contains 4 fields separated by TABS, a description
  (name), the minimum and the maximum values and the unit.
  e.g. CPU Voltage 0.0	5.0	V
 */
class SensorFloatInfo : public SensorTokenizer
{
  public:
    explicit SensorFloatInfo( const QByteArray &info )
      : SensorTokenizer( info, '\t' ) { }

    ~SensorFloatInfo() { }

    QString name()
    {
      if (count() > 0)
          return QString::fromUtf8((*this)[ 0 ]);
      return QString();
    }

    double min()
    {
      if (count() > 1)
          return (*this)[ 1 ].toDouble();
      return -1;
    }

    double max()
    {
      if (count() > 2)
          return (*this)[ 2 ].toDouble();
      return -1;
    }

    QString unit()
    {
      if (count() > 3)
          return QString::fromUtf8((*this)[ 3 ]);
      return QString();
    }
};


}

#endif
