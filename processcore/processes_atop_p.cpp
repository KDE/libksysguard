/*  This file is part of the KDE project

    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

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

#include "processes_atop_p.h"
#include "process.h"
#include "atop_p.h"

#include <klocale.h>
#include <zlib.h>

#include <QFile>
#include <QHash>
#include <QSet>
#include <QByteArray>
#include <QTextStream>
#include <QtEndian>

#include <QDebug>

namespace KSysGuard
{

  class ProcessesATop::Private
  {
    public:
      Private();
      ~Private();
      QFile atopLog;
      bool ready;

      bool loadDataForHistory(int index);
      bool loadHistoryFile(const QString &filename);

      RawHeader    rh;
      RawRecord    rr;
//      SStat        sstats;
      PStat        *pstats;
      QList<long>  pids; //This is a list of process pid's, in the exact same order as pstats
      QString lastError;

      QList<long>  historyOffsets; //< The file offset where each history is stored
      QList< QPair<QDateTime, uint> > historyTimes;  //< The end time for each history record and its interval, probably in order from oldest to newest
      int currentlySelectedIndex;
  };

ProcessesATop::Private::Private() :
    ready(false),
    pstats(NULL),
    currentlySelectedIndex(-1)
{
}

ProcessesATop::Private::~Private()
{
}

QString ProcessesATop::historyFileName() const {
    return d->atopLog.fileName();
}

bool ProcessesATop::loadHistoryFile(const QString &filename)
{
    return d->loadHistoryFile(filename);
}

bool ProcessesATop::Private::loadHistoryFile(const QString &filename) {
    atopLog.setFileName(filename);
    ready = false;
    currentlySelectedIndex = -1;
    if(!atopLog.exists()) {
        lastError = "File " + filename + " does not exist";
        return false;
    }

    if(!atopLog.open(QIODevice::ReadOnly)) {
        lastError = "Could not open file" + filename;
        return false;
    }

    int sizeRead = atopLog.read((char*)(&rh), sizeof(RawHeader));
    if(sizeRead != sizeof(RawHeader)) {
        lastError = "Could not read header from file" + filename;
        return false;
    }
    if(rh.magic != ATOPLOGMAGIC) {
        lastError = "File " + filename + " does not contain raw atop/atopsar output (wrong magic number)";
        return false;
    }
    if (/*rh.sstatlen   != sizeof(SStat)    ||*/
        rh.pstatlen   != sizeof(PStat)      ||
        rh.rawheadlen != sizeof(RawHeader)  ||
        rh.rawreclen  != sizeof(RawRecord)  )
    {
        lastError = "File " + filename + " has incompatible format";
        if (rh.aversion & 0x8000) {
            lastError = QString("(created by version %1.%2. This program understands the format written by version 1.23")
                        .arg((rh.aversion >> 8) & 0x7f, rh.aversion & 0xff);
        }
        return false;
    }


    /* Read the first data header */
    int offset = atopLog.pos();
    historyTimes.clear();
    historyOffsets.clear();
    while( !atopLog.atEnd() && atopLog.read((char*)(&rr), sizeof(RawRecord)) == sizeof(RawRecord) ) {
        historyOffsets << offset;
        historyTimes << QPair<QDateTime,uint>(QDateTime::fromTime_t(rr.curtime), rr.interval);
        offset +=  sizeof(RawRecord) + rr.scomplen + rr.pcomplen;
        atopLog.seek(offset);
    }
    if(currentlySelectedIndex >= historyOffsets.size())
        currentlySelectedIndex = historyOffsets.size() - 1;

    ready = true;
    return true;
}

bool ProcessesATop::Private::loadDataForHistory(int index)
{
    delete [] pstats;
    pstats = NULL;
    atopLog.seek(historyOffsets.at(index));
    /*Read the first data header */
    if( atopLog.read((char*)(&rr), sizeof(RawRecord)) != sizeof(RawRecord) ) {
        lastError = "Could not read data header";
        return false;
    }

    if( historyTimes.at(index).first != QDateTime::fromTime_t(rr.curtime) ||
           historyTimes.at(index).second != rr.interval) {
        lastError = "INTERNAL ERROR WITH loadDataForHistory";
        ready = false;
        return false;
    }

    atopLog.seek(atopLog.pos() + rr.scomplen);
    QByteArray processRecord;
    processRecord.resize(rr.pcomplen);
//    qToBigEndian( rr.pcomplen, (uchar*)processRecord.data() );
    unsigned int dataRead = 0;
    do {
        int ret = atopLog.read( processRecord.data() + dataRead, rr.pcomplen - dataRead);
        if(ret == -1) {
            lastError = "Stream interrupted while being read";
            return false;
        }
        dataRead += ret;
    } while(dataRead < rr.pcomplen);
    Q_ASSERT(dataRead == rr.pcomplen);
    //Q_ASSERT( (index + 1 ==historyTimes.count()) || atopLog.pos() == historyTimes.at(index+1));

    pstats = new PStat[ rr.nlist ];
    unsigned long uncompressedLength= sizeof(struct PStat) * rr.nlist;
    int ret = uncompress((Byte *)pstats, &uncompressedLength, (Byte *)processRecord.constData(), rr.pcomplen);
    if(ret != Z_OK && ret != Z_STREAM_END && ret != Z_NEED_DICT) {
        switch(ret) {
            case Z_MEM_ERROR:
                lastError = "Could not uncompress record data due to lack of memory";
                break;
            case Z_BUF_ERROR:
                lastError = "Could not uncompress record data due to lack of room in buffer";
                break;
            case Z_DATA_ERROR:
                lastError = "Could not uncompress record data due to corrupted data";
                break;
            default:
                lastError = "Could not uncompress record data due to unexpected error: " + QString::number(ret);
                break;
        }
        delete [] pstats;
        pstats = NULL;
        return false;
    }

    pids.clear();
    for(uint i = 0; i < rr.nlist; i++) {
        pids << pstats[i].gen.pid;
    }
    return true;
}

ProcessesATop::ProcessesATop(bool loadDefaultFile) : d(new Private())
{
    if(loadDefaultFile)
        loadHistoryFile("/var/log/atop.log");
}

bool ProcessesATop::isHistoryAvailable() const
{
    return d->ready;
}

long ProcessesATop::getParentPid(long pid) {
    int index = d->pids.indexOf(pid);
    if(index < 0)
        return 0;
    return d->pstats[index].gen.ppid;
}

bool ProcessesATop::updateProcessInfo( long pid, Process *process)
{
    int index = d->pids.indexOf(pid);
    if(index < 0)
        return false;
    PStat &p = d->pstats[index];
    process->parent_pid = p.gen.ppid;
    process->setUid(p.gen.ruid);
    process->setEuid(p.gen.ruid);
    process->setSuid(p.gen.ruid);
    process->setFsuid(p.gen.ruid);
    process->setGid(p.gen.rgid);
    process->setEgid(p.gen.rgid);
    process->setSgid(p.gen.rgid);
    process->setFsgid(p.gen.rgid);
    process->setTracerpid(-1);
    process->setNumThreads(p.gen.nthr);
//    process->setTty
    process->setUserTime(p.cpu.utime * 100/d->rh.hertz);//check - divide by interval maybe?
    process->setSysTime(p.cpu.stime * 100/d->rh.hertz); //check
    process->setUserUsage( process->userTime / d->rr.interval );
    process->setSysUsage( process->sysTime / d->rr.interval );
    process->setNiceLevel(p.cpu.nice);
//    process->setscheduler(p.cpu.policy);
    process->setVmSize(p.mem.vmem);
    process->setVmRSS(p.mem.rmem);
    process->vmSizeChange = p.mem.vgrow;
    process->vmRSSChange = p.mem.rgrow;
    process->setVmURSS(0);
    process->vmURSSChange = 0;

    /* Fill in name and command */
    QString name = QString::fromUtf8(p.gen.name, qstrnlen(p.gen.name,PNAMLEN));
    QString command = QString::fromUtf8(p.gen.cmdline, qstrnlen(p.gen.cmdline,CMDLEN));
    //cmdline separates parameters with the NULL character
    if(!command.isEmpty()) {
        if(command.startsWith(name)) {
            int index = command.indexOf(QChar('\0'));
            name = command.left(index);
        }
        command.replace('\0', ' ');
    }
    process->setName(name);
    process->setCommand(command);


    /* Fill in state */
    switch(p.gen.state) {
        case 'E':
            process->setStatus(Process::Ended);
            break;
        case 'R':
            process->setStatus(Process::Running);
            break;
        case 'S':
            process->setStatus(Process::Sleeping);
            break;
        case 'D':
            process->setStatus(Process::DiskSleep);
            break;
        case 'Z':
            process->setStatus(Process::Zombie);
            break;
        case 'T':
            process->setStatus(Process::Stopped);
            break;
        case 'W':
            process->setStatus(Process::Paging);
            break;
        default:
            process->setStatus(Process::OtherStatus);
            break;
    }


    return true;
}
QDateTime ProcessesATop::viewingTime() const
{
    if(!d->ready)
        return QDateTime();
    return d->historyTimes.at(d->currentlySelectedIndex).first; 
}
bool ProcessesATop::setViewingTime(const QDateTime &when)
{
    QPair<QDateTime, uint> tmpWhen(when, 0);
    QList< QPair<QDateTime,uint> >::iterator i = qUpperBound(d->historyTimes.begin(), d->historyTimes.end(), tmpWhen);

    if(i->first == when || (i->first > when && i->first.addSecs(-i->second) <= when)) {
        //We found the time :)
        d->currentlySelectedIndex = i - d->historyTimes.begin();
        bool success = d->loadDataForHistory(d->currentlySelectedIndex);
        if(!success)
            qWarning() << d->lastError;
        return success;
    }
    return false;
}
QList< QPair<QDateTime, uint> > ProcessesATop::historiesAvailable() const
{
    return d->historyTimes;
}

QSet<long> ProcessesATop::getAllPids( )
{
    return d->pids.toSet();
}

bool ProcessesATop::sendSignal(long pid, int sig) {
    Q_UNUSED(pid);
    Q_UNUSED(sig);

    errorCode = Processes::NotSupported;
    return false;
}

bool ProcessesATop::setNiceness(long pid, int priority) {
    Q_UNUSED(pid);
    Q_UNUSED(priority);

    errorCode = Processes::NotSupported;
    return false;
}

bool ProcessesATop::setScheduler(long pid, int priorityClass, int priority) {
    Q_UNUSED(pid);
    Q_UNUSED(priorityClass);
    Q_UNUSED(priority);

    errorCode = Processes::NotSupported;
    return false;
}


bool ProcessesATop::setIoNiceness(long pid, int priorityClass, int priority) {
    Q_UNUSED(pid);
    Q_UNUSED(priorityClass);
    Q_UNUSED(priority);

    errorCode = Processes::NotSupported;
    return false;
}

bool ProcessesATop::supportsIoNiceness() {
    return false;
}

long long ProcessesATop::totalPhysicalMemory() {
    return 0;
}
ProcessesATop::~ProcessesATop()
{
  delete d;
}

}
