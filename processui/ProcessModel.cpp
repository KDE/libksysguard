/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    Copyright (c) 2006-2007 John Tapsell <john.tapsell@kde.org>

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


#include "ProcessModel.h"
#include "ProcessModel_p.h"

#include "processcore/processes.h"
#include "processcore/process.h"

#include <kapplication.h>
#include <kcolorscheme.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <QBitmap>
#include <QFont>
#include <QIcon>
#include <QPixmap>
#include <QList>
#include <QMimeData>
#include <QTextDocument>

#define HEADING_X_ICON_SIZE 16
#define MILLISECONDS_TO_SHOW_RED_FOR_KILLED_PROCESS 2000
#define GET_OWN_ID

#ifdef GET_OWN_ID
/* For getuid*/
#include <unistd.h>
#include <sys/types.h>
#endif

#include "ProcessModel.moc"
#include "ProcessModel_p.moc"

#ifdef HAVE_XRES
#include <X11/extensions/XRes.h>
#endif

extern KApplication* Kapp;

static QString formatByteSize(qlonglong amountInKB, int units) {
    enum { UnitsAuto, UnitsKB, UnitsMB, UnitsGB, UnitsTB, UnitsPB };
    static QString kString = i18n("%1 K", QString::fromLatin1("%1"));
    static QString mString = i18n("%1 M", QString::fromLatin1("%1"));
    static QString gString = i18n("%1 G", QString::fromLatin1("%1"));
    static QString tString = i18n("%1 T", QString::fromLatin1("%1"));
    static QString pString = i18n("%1 P", QString::fromLatin1("%1"));
    double amount;

    if (units == UnitsAuto) {
        if (amountInKB < 1024.0*0.9)
            units = UnitsKB; // amount < 0.9 MiB == KiB
        else if (amountInKB < 1024.0*1024.0*0.9)
            units = UnitsMB; // amount < 0.9 GiB == MiB
        else if (amountInKB < 1024.0*1024.0*1024.0*0.9)
            units = UnitsGB; // amount < 0.9 TiB == GiB
        else if (amountInKB < 1024.0*1024.0*1024.0*1024.0*0.9)
            units = UnitsTB; // amount < 0.9 PiB == TiB
        else
            units = UnitsPB;
    }

    switch(units) {
      case UnitsKB:
        return kString.arg(KGlobal::locale()->formatNumber(amountInKB, 0));
      case UnitsMB:
        amount = amountInKB/1024.0;
        return mString.arg(KGlobal::locale()->formatNumber(amount, 1));
      case UnitsGB:
        amount = amountInKB/(1024.0*1024.0);
        if(amount < 0.1 && amount > 0.05) amount = 0.1;
        return gString.arg(KGlobal::locale()->formatNumber(amount, 1));
      case UnitsTB:
        amount = amountInKB/(1024.0*1024.0*1024.0);
        if(amount < 0.1 && amount > 0.05) amount = 0.1;
        return tString.arg(KGlobal::locale()->formatNumber(amount, 1));
      case UnitsPB:
        amount = amountInKB/(1024.0*1024.0*1024.0*1024.0);
        if(amount < 0.1 && amount > 0.05) amount = 0.1;
        return pString.arg(KGlobal::locale()->formatNumber(amount, 1));
      default:
          return "";  // error
    }
}

ProcessModelPrivate::ProcessModelPrivate() :  mBlankPixmap(HEADING_X_ICON_SIZE,1)
{
    mBlankPixmap.fill(QColor(0,0,0,0));
    mSimple = true;
    mIsLocalhost = true;
    mMemTotal = -1;
    mNumProcessorCores = 1;
    mProcesses = NULL;
    mShowChildTotals = true;
    mShowCommandLineOptions = false;
    mShowingTooltips = true;
    mNormalizeCPUUsage = true;
    mIoInformation = ProcessModel::ActualBytes;
#ifdef HAVE_XRES
    mHaveXRes = false;
#endif
    mHaveTimer = false,
    mTimerId = -1,
    mMovingRow = false;
    mRemovingRow = false;
    mInsertingRow = false;
}

ProcessModelPrivate::~ProcessModelPrivate()
{
#ifdef Q_WS_X11
    qDeleteAll(mPidToWindowInfo);
#endif
    delete mProcesses;
    mProcesses = NULL;
}
ProcessModel::ProcessModel(QObject* parent, const QString &host)
    : QAbstractItemModel(parent), d(new ProcessModelPrivate)
{
    KGlobal::locale()->insertCatalog("processui");  //Make sure we include the translation stuff.  This needs to be run before any i18n call here
    d->q=this;
#ifdef HAVE_XRES
    int event, error, major, minor;
    d->mHaveXRes = XResQueryExtension(QX11Info::display(), &event, &error) && XResQueryVersion(QX11Info::display(), &major, &minor);
#endif

    if(host.isEmpty() || host == "localhost") {
        d->mHostName = QString();
        d->mIsLocalhost = true;
    } else {
        d->mHostName = host;
        d->mIsLocalhost = false;
    }
    setupHeader();
    d->setupProcesses();
#ifdef Q_WS_X11
    d->setupWindows();
#endif
    d->mUnits = UnitsKB;
    d->mIoUnits = UnitsKB;


}

bool ProcessModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    //Because we need to sort Descendingly by default for most of the headings, we often return left > right
    KSysGuard::Process *processLeft = reinterpret_cast< KSysGuard::Process * > (left.internalPointer());
    KSysGuard::Process *processRight = reinterpret_cast< KSysGuard::Process * > (right.internalPointer());
    Q_ASSERT(processLeft);
    Q_ASSERT(processRight);
    Q_ASSERT(left.column() == right.column());
    switch(left.column()) {
        case HeadingUser:
        {
            /* Sorting by user will be the default and the most common.
               We want to sort in the most useful way that we can. We need to return a number though.
               This code is based on that sorting ascendingly should put the current user at the top
               First the user we are running as should be at the top.
               Then any other users in the system.
               Then at the bottom the 'system' processes.
               We then sort by cpu usage to sort by that, then finally sort by memory usage */

            /* First, place traced processes at the very top, ignoring any other sorting criteria */
            if(processLeft->tracerpid >= 0)
                return true;
            if(processRight->tracerpid >= 0)
                return false;

            /* Sort by username.  First group into own user, normal users, system users */
            if(processLeft->uid != processRight->uid) {
                //We primarily sort by username
                if(d->mIsLocalhost) {
                    int ownUid = getuid();
                    if(processLeft->uid == ownUid)
                        return true; //Left is our user, right is not.  So left is above right
                    if(processRight->uid == ownUid)
                        return false; //Left is not our user, right is.  So right is above left
                }
                bool isLeftSystemUser = processLeft->uid < 100 || !canUserLogin(processLeft->uid);
                bool isRightSystemUser = processRight->uid < 100 || !canUserLogin(processRight->uid);
                if(isLeftSystemUser && !isRightSystemUser)
                    return false; //System users are less than non-system users
                if(!isLeftSystemUser && isRightSystemUser)
                    return true;
                //They are either both system users, or both non-system users.
                //So now sort by username
                return d->getUsernameForUser(processLeft->uid, false) < d->getUsernameForUser(processRight->uid, false);
            }

            /* 2nd sort order - Graphics Windows */
            //Both columns have the same user.  Place processes with windows at the top
            if(processLeft->hasManagedGuiWindow && !processRight->hasManagedGuiWindow)
                return true;
            if(!processLeft->hasManagedGuiWindow && processRight->hasManagedGuiWindow)
                return false;

            /* 3rd sort order - CPU Usage */
            int leftCpu, rightCpu;
            if(d->mSimple || !d->mShowChildTotals) {
                leftCpu = processLeft->userUsage + processLeft->sysUsage;
                rightCpu = processRight->userUsage + processRight->sysUsage;
            } else {
                leftCpu = processLeft->totalUserUsage + processLeft->totalSysUsage;
                rightCpu = processRight->totalUserUsage + processRight->totalSysUsage;
            }
            if(leftCpu != rightCpu)
                return leftCpu > rightCpu;

            /* 4th sort order - Memory Usage */
            qlonglong memoryLeft = (processLeft->vmURSS != -1)?processLeft->vmURSS:processLeft->vmRSS;
            qlonglong memoryRight = (processRight->vmURSS != -1)?processRight->vmURSS:processRight->vmRSS;
            return memoryLeft > memoryRight;
        }
        case HeadingCPUUsage: {
            int leftCpu, rightCpu;
            if(d->mSimple || !d->mShowChildTotals) {
                leftCpu = processLeft->userUsage + processLeft->sysUsage;
                rightCpu = processRight->userUsage + processRight->sysUsage;
            } else {
                leftCpu = processLeft->totalUserUsage + processLeft->totalSysUsage;
                rightCpu = processRight->totalUserUsage + processRight->totalSysUsage;
            }
            return leftCpu > rightCpu;
         }
        case HeadingCPUTime: {
            return (processLeft->userTime + processLeft->sysTime) > (processRight->userTime + processRight->sysTime);
        }
        case HeadingMemory: {
            qlonglong memoryLeft = (processLeft->vmURSS != -1)?processLeft->vmURSS:processLeft->vmRSS;
            qlonglong memoryRight = (processRight->vmURSS != -1)?processRight->vmURSS:processRight->vmRSS;
            return memoryLeft > memoryRight;
        }
        case HeadingXMemory:
            return processLeft->pixmapBytes > processRight->pixmapBytes;
        case HeadingVmSize:
            return processLeft->vmSize > processRight->vmSize;
        case HeadingSharedMemory: {
            qlonglong memoryLeft = (processLeft->vmURSS != -1)?processLeft->vmRSS - processLeft->vmURSS:0;
            qlonglong memoryRight = (processRight->vmURSS != -1)?processRight->vmRSS - processRight->vmURSS:0;
            return memoryLeft > memoryRight;
        }
        case HeadingPid:
            return processLeft->pid > processRight->pid;
        case HeadingNiceness:
            //Sort by scheduler first
            if(processLeft->scheduler != processRight->scheduler) {
                if(processLeft->scheduler == KSysGuard::Process::RoundRobin || processLeft->scheduler == KSysGuard::Process::Fifo)
                    return true;
                if(processRight->scheduler == KSysGuard::Process::RoundRobin || processRight->scheduler == KSysGuard::Process::Fifo)
                    return false;
                if(processLeft->scheduler == KSysGuard::Process::Other)
                    return true;
                if(processRight->scheduler == KSysGuard::Process::Other)
                    return false;
                if(processLeft->scheduler == KSysGuard::Process::Batch)
                    return true;
            }
            if(processLeft->niceLevel == processRight->niceLevel)
                return processLeft->pid < processRight->pid; //Subsort by pid if the niceLevel is the same
            return processLeft->niceLevel < processRight->niceLevel;
        case HeadingTty: {
            if( processLeft->tty == processRight->tty)
                return processLeft->pid < processRight->pid; //Both ttys are the same.  Sort by pid
            if(processLeft->tty.isEmpty())
                return false; //Only left is empty (since they aren't the same)
            else if(processRight->tty.isEmpty())
                return true; //Only right is empty

            //Neither left or right is empty. The tty string is like  "tty10"  so split this into "tty" and "10"
            //and sort by the string first, then sort by the number
            QRegExp regexpLeft("^(\\D*)(\\d*)$");
            QRegExp regexpRight(regexpLeft);
            if(regexpLeft.indexIn(processLeft->tty) == -1 || regexpRight.indexIn(processRight->tty) == -1)
                return processLeft->tty < processRight->tty;
            int nameMatch = regexpLeft.cap(1).compare(regexpRight.cap(1));
            if(nameMatch < 0)
                return true;
            if(nameMatch > 0)
                return false;
            return regexpLeft.cap(2).toInt() < regexpRight.cap(2).toInt();
        }
        case HeadingIoRead:
            switch(d->mIoInformation) {
                case ProcessModel::Bytes:
                    return processLeft->ioCharactersRead > processRight->ioCharactersRead;
                case ProcessModel::Syscalls:
                    return processLeft->ioReadSyscalls > processRight->ioReadSyscalls;
                case ProcessModel::ActualBytes:
                    return processLeft->ioCharactersActuallyRead > processRight->ioCharactersActuallyRead;
                case ProcessModel::BytesRate:
                    return processLeft->ioCharactersReadRate > processRight->ioCharactersReadRate;
                case ProcessModel::SyscallsRate:
                    return processLeft->ioReadSyscallsRate > processRight->ioReadSyscallsRate;
                case ProcessModel::ActualBytesRate:
                    return processLeft->ioCharactersActuallyReadRate > processRight->ioCharactersActuallyReadRate;

            }
        case HeadingIoWrite:
            switch(d->mIoInformation) {
                case ProcessModel::Bytes:
                    return processLeft->ioCharactersWritten > processRight->ioCharactersWritten;
                case ProcessModel::Syscalls:
                    return processLeft->ioWriteSyscalls > processRight->ioWriteSyscalls;
                case ProcessModel::ActualBytes:
                    return processLeft->ioCharactersActuallyWritten > processRight->ioCharactersActuallyWritten;
                case ProcessModel::BytesRate:
                    return processLeft->ioCharactersWrittenRate > processRight->ioCharactersWrittenRate;
                case ProcessModel::SyscallsRate:
                    return processLeft->ioWriteSyscallsRate > processRight->ioWriteSyscallsRate;
                case ProcessModel::ActualBytesRate:
                    return processLeft->ioCharactersActuallyWrittenRate > processRight->ioCharactersActuallyWrittenRate;
        }
    }
    //Sort by the display string if we do not have an explicit sorting here
    return data(left, Qt::DisplayRole).toString() < data(right, Qt::DisplayRole).toString();
}

ProcessModel::~ProcessModel()
{
    delete d;
}

KSysGuard::Processes *ProcessModel::processController() const
{
    return d->mProcesses;
}

#ifdef Q_WS_X11
void ProcessModelPrivate::windowRemoved(WId wid) {
    WindowInfo *window = mWIdToWindowInfo.take(wid);
    if(!window) return;
    qlonglong pid = window->pid;

    QMultiHash<qlonglong, WindowInfo*>::iterator i = mPidToWindowInfo.find(pid);
    while (i != mPidToWindowInfo.end() && i.key() == pid) {
        if(i.value()->wid == wid) {
            i = mPidToWindowInfo.erase(i);
            break;
        } else
            i++;
    }
    delete window;

    //Update the model so that it redraws and resorts
    KSysGuard::Process *process = mProcesses->getProcess(pid);
    if(!process) return;

    int row;
    if(mSimple)
        row = process->index;
    else
        row = process->parent->children.indexOf(process);
    QModelIndex index2 = q->createIndex(row, ProcessModel::HeadingXTitle, process);
    emit q->dataChanged(index2, index2);
}
#endif

#ifdef Q_WS_X11
void ProcessModelPrivate::setupWindows() {
    connect( KWindowSystem::self(), SIGNAL(windowChanged(WId,uint)), this, SLOT(windowChanged(WId,uint)));
    connect( KWindowSystem::self(), SIGNAL(windowAdded(WId)), this, SLOT(windowAdded(WId)));
    connect( KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));

    //Add all the windows that KWin is managing - i.e. windows that the user can see
    QList<WId>::ConstIterator it;
    for ( it = KWindowSystem::windows().begin(); it != KWindowSystem::windows().end(); ++it ) {
        updateWindowInfo(*it, ~0u, true);
    }
}
#endif
#ifdef HAVE_XRES
bool ProcessModelPrivate::updateXResClientData() {
    XResClient *clients;
    int count;

    KXErrorHandler handler;
    XResQueryClients(QX11Info::display(), &count, &clients);
    if (handler.error( false ) )
        return false;

    mXResClientResources.clear();
    for (int i=0; i < count; i++)
        mXResClientResources.insert(-(qlonglong)(clients[i].resource_base), clients[i].resource_mask);

    if(clients)
        XFree(clients);
    return true;
}
void ProcessModelPrivate::queryForAndUpdateAllXWindows() {
//    qDebug() << "updating xres info:" << QTime::currentTime().toString("hh:mm:ss.zzz");
    updateXResClientData();
    Window       *children, dummy;
    unsigned int  count;
    Status result = XQueryTree(QX11Info::display(), QX11Info::appRootWindow(), &dummy, &dummy, &children, &count);
    if(!result)
        return;
    if(!updateXResClientData())
        return;
    for (uint i=0; i < count; ++i) {
        WId wid = children[i];
        QMap<qlonglong, XID>::iterator iter = mXResClientResources.lowerBound(-(qlonglong)(wid));
        if(iter == mXResClientResources.end())
            continue; //We couldn't find it this time :-/

        if(-iter.key() != (qlonglong)(wid & ~iter.value()))
            continue; //Already added this window

        //Get the PID for this window if we do not know it
        KXErrorHandler handler;
        NETWinInfo info( QX11Info::display(), wid, QX11Info::appRootWindow(), NET::WMPid );
        if (handler.error( false ) )
            continue;  //info is invalid - window just closed or something probably

        qlonglong pid = info.pid();
        if(!pid)
            continue;
        //We found a window with this client
        mXResClientResources.erase(iter);
        KSysGuard::Process *process = mProcesses->getProcess(pid);
        if(!process) return; //shouldn't really happen.. maybe race condition etc
        unsigned long previousPixmapBytes = process->pixmapBytes;
        //Now update the pixmap bytes for this window
        bool success = XResQueryClientPixmapBytes(QX11Info::display(), wid, &process->pixmapBytes);
        if(!success)
            process->pixmapBytes = 0;

        if(previousPixmapBytes != process->pixmapBytes) {
            int row;
            if(mSimple)
                row = process->index;
            else
                row = process->parent->children.indexOf(process);
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingXMemory, process);
            emit q->dataChanged(index, index);
        }
    }
    if(children)
        XFree((char*)children);
//    qDebug() << "done updating xres info:" << QTime::currentTime().toString("hh:mm:ss.zzz");
}
#endif
void ProcessModelPrivate::setupProcesses() {
    if(mProcesses) {
#ifdef Q_WS_X11_DISABLE
        mWIdToWindowInfo.clear();
        mPidToWindowInfo.clear();
#endif
        delete mProcesses;
        mProcesses = 0;
        q->reset();
    }

    mProcesses = new KSysGuard::Processes(mHostName);

    connect( mProcesses, SIGNAL(processChanged(KSysGuard::Process*,bool)), this, SLOT(processChanged(KSysGuard::Process*,bool)));
    connect( mProcesses, SIGNAL(beginAddProcess(KSysGuard::Process*)), this, SLOT(beginInsertRow(KSysGuard::Process*)));
    connect( mProcesses, SIGNAL(endAddProcess()), this, SLOT(endInsertRow()));
    connect( mProcesses, SIGNAL(beginRemoveProcess(KSysGuard::Process*)), this, SLOT(beginRemoveRow(KSysGuard::Process*)));
    connect( mProcesses, SIGNAL(endRemoveProcess()), this, SLOT(endRemoveRow()));
    connect( mProcesses, SIGNAL(beginMoveProcess(KSysGuard::Process*,KSysGuard::Process*)), this,
            SLOT(beginMoveProcess(KSysGuard::Process*,KSysGuard::Process*)));
    connect( mProcesses, SIGNAL(endMoveProcess()), this, SLOT(endMoveRow()));
    mNumProcessorCores = mProcesses->numberProcessorCores();
    if(mNumProcessorCores < 1) mNumProcessorCores=1;  //Default to 1 if there was an error getting the number
}

#ifdef Q_WS_X11
void ProcessModelPrivate::windowChanged(WId wid, unsigned int properties)
{
    updateWindowInfo(wid, properties, false);
}

void ProcessModelPrivate::windowAdded(WId wid)
{
    updateWindowInfo(wid, ~0u, true);
}

void ProcessModelPrivate::updateWindowInfo(WId wid, unsigned int properties, bool newWindow)
{
    properties &= (NET::WMPid | NET::WMVisibleName | NET::WMName | NET::WMIcon);

    if(!properties)
        return; //Nothing interesting changed

    WindowInfo *w = mWIdToWindowInfo.value(wid);
    if(!w && !newWindow)
        return; //We do not have a record of this window and this is not a new window

    if(properties == NET::WMIcon) {
        if(w)
            w->icon = KWindowSystem::icon(wid, HEADING_X_ICON_SIZE, HEADING_X_ICON_SIZE, true);
        return;
    }
    /* Get PID for window */
    KXErrorHandler handler;
    NETWinInfo info( QX11Info::display(), wid, QX11Info::appRootWindow(), properties & ~NET::WMIcon );
    if (handler.error( false ) ) {
        return;  //info is invalid - window just closed or something probably
    }

    if(!w) {
        //We know that this must be a newWindow
        qlonglong pid = info.pid();
        if(!(properties & NET::WMPid && pid))
            return; //No PID for the window - this happens if the process did not set _NET_WM_PID

        //If we are to get the PID only, we are only interested in the XRes info for this,
        //so don't bother if we already have this info
        if(properties == NET::WMPid && mPidToWindowInfo.contains(pid))
            return;

        w = new WindowInfo(wid, pid);
        mPidToWindowInfo.insertMulti(pid, w);
        mWIdToWindowInfo.insert(wid, w);
    }

    if(w && (properties & NET::WMIcon))
        w->icon = KWindowSystem::icon(wid, HEADING_X_ICON_SIZE, HEADING_X_ICON_SIZE, true);
    if(properties & NET::WMVisibleName && info.visibleName())
        w->name = QString::fromUtf8(info.visibleName());
    else if(properties & NET::WMName)
        w->name = QString::fromUtf8(info.name());
    else if(properties & (NET::WMName | NET::WMVisibleName))
        w->name.clear();

    KSysGuard::Process *process = mProcesses->getProcess(w->pid);
    if(!process) {
        mProcesses->updateOrAddProcess(w->pid);
        return; //This happens when a new window is detected before we've read in the process
    }

    int row;
    if(mSimple)
        row = process->index;
    else
        row = process->parent->children.indexOf(process);
    if(!process->hasManagedGuiWindow) {
        process->hasManagedGuiWindow = true;;
        //Since this is the first window for a process, invalidate HeadingName so that
        //if we are sorting by name this gets taken into account
        QModelIndex index1 = q->createIndex(row, ProcessModel::HeadingName, process);
        emit q->dataChanged(index1, index1);
    }
    QModelIndex index2 = q->createIndex(row, ProcessModel::HeadingXTitle, process);
    emit q->dataChanged(index2, index2);
}
#endif

void ProcessModel::update(long updateDurationMSecs, KSysGuard::Processes::UpdateFlags updateFlags) {
//    kDebug() << "update all processes: " << QTime::currentTime().toString("hh:mm:ss.zzz");
    if(updateFlags != KSysGuard::Processes::XMemory) {
        d->mProcesses->updateAllProcesses(updateDurationMSecs, updateFlags);
        if(d->mMemTotal <= 0)
            d->mMemTotal = d->mProcesses->totalPhysicalMemory();
    }

//    kDebug() << "finished:             " << QTime::currentTime().toString("hh:mm:ss.zzz");
#ifdef HAVE_XRES
    //Add all the rest of the windows
    if(d->mHaveXRes && updateFlags.testFlag(KSysGuard::Processes::XMemory))
        d->queryForAndUpdateAllXWindows();
#endif

}

QString ProcessModelPrivate::getStatusDescription(KSysGuard::Process::ProcessStatus status) const
{
    switch( status) {
        case KSysGuard::Process::Running:
            return i18n("- Process is doing some work.");
        case KSysGuard::Process::Sleeping:
            return i18n("- Process is waiting for something to happen.");
        case KSysGuard::Process::Stopped:
            return i18n("- Process has been stopped. It will not respond to user input at the moment.");
        case KSysGuard::Process::Zombie:
            return i18n("- Process has finished and is now dead, but the parent process has not cleaned up.");
        case KSysGuard::Process::Ended:
//            return i18n("- Process has finished and no longer exists.");
        default:
            return QString();
    }
}

KSysGuard::Process *ProcessModel::getProcessAtIndex(int index) const
{
    Q_ASSERT(d->mSimple);
    return d->mProcesses->getAllProcesses().at(index);
}
int ProcessModel::rowCount(const QModelIndex &parent) const
{
    if(d->mSimple) {
        if(parent.isValid()) return 0; //In flat mode, none of the processes have children
        return d->mProcesses->processCount();
    }

    //Deal with the case that we are showing it as a tree
    KSysGuard::Process *process;
    if(parent.isValid()) {
        if(parent.column() != 0) return 0;  //For a treeview we say that only the first column has children
        process = reinterpret_cast< KSysGuard::Process * > (parent.internalPointer()); //when parent is invalid, it must be the root level which we set as 0
    } else {
        process = d->mProcesses->getProcess(-1);
    }
    Q_ASSERT(process);
    int num_rows = process->children.count();
    return num_rows;
}

int ProcessModel::columnCount ( const QModelIndex & ) const
{
    return d->mHeadings.count();
}

bool ProcessModel::hasChildren ( const QModelIndex & parent = QModelIndex() ) const
{

    if(d->mSimple) {
        if(parent.isValid()) return 0; //In flat mode, none of the processes have children
        return !d->mProcesses->getAllProcesses().isEmpty();
    }

    //Deal with the case that we are showing it as a tree
    KSysGuard::Process *process;
    if(parent.isValid()) {
        if(parent.column() != 0) return false;  //For a treeview we say that only the first column has children
        process = reinterpret_cast< KSysGuard::Process * > (parent.internalPointer()); //when parent is invalid, it must be the root level which we set as 0
    } else {
        process = d->mProcesses->getProcess(-1);
    }
    Q_ASSERT(process);
    bool has_children = !process->children.isEmpty();

    Q_ASSERT((rowCount(parent) > 0) == has_children);
    return has_children;
}

QModelIndex ProcessModel::index ( int row, int column, const QModelIndex & parent ) const
{
    if(row<0) return QModelIndex();
    if(column<0 || column >= d->mHeadings.count() ) return QModelIndex();

    if(d->mSimple) {
        if( parent.isValid()) return QModelIndex();
        if( d->mProcesses->processCount() <= row) return QModelIndex();
        return createIndex( row, column, d->mProcesses->getAllProcesses().at(row));
    }

    //Deal with the case that we are showing it as a tree
    KSysGuard::Process *parent_process = 0;

    if(parent.isValid()) //not valid for init or children without parents, so use our special item with pid of 0
        parent_process = reinterpret_cast< KSysGuard::Process * > (parent.internalPointer());
    else
        parent_process = d->mProcesses->getProcess(-1);
    Q_ASSERT(parent_process);

    if(parent_process->children.count() > row)
        return createIndex(row,column, parent_process->children[row]);
    else
    {
        return QModelIndex();
    }
}

bool ProcessModel::isSimpleMode() const
{
    return d->mSimple;
}

void ProcessModelPrivate::processChanged(KSysGuard::Process *process, bool onlyTotalCpu)
{
    int row;
    if(mSimple)
        row = process->index;
    else
        row = process->parent->children.indexOf(process);

    if (!process->timeKillWasSent.isNull()) {
        int elapsed = process->timeKillWasSent.elapsed();
        if (elapsed < MILLISECONDS_TO_SHOW_RED_FOR_KILLED_PROCESS) {
            if (!mPidsToUpdate.contains(process->pid))
                mPidsToUpdate.append(process->pid);
            QModelIndex index1 = q->createIndex(row, 0, process);
            QModelIndex index2 = q->createIndex(row, mHeadings.count()-1, process);
            emit q->dataChanged(index1, index2);
            if (!mHaveTimer) {
                mHaveTimer = true;
                mTimerId = startTimer(100);
            }
        }
    }
    int totalUpdated = 0;
    Q_ASSERT(row != -1);  //Something has gone very wrong
    if(onlyTotalCpu) {
        if(mShowChildTotals) {
            //Only the total cpu usage changed, so only update that
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingCPUUsage, process);
            emit q->dataChanged(index, index);
        }
        return;
    } else {
        if(process->changes == KSysGuard::Process::Nothing) {
            return; //Nothing changed
        }
        if(process->changes & KSysGuard::Process::Uids) {
            totalUpdated++;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingUser, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & KSysGuard::Process::Tty) {
            totalUpdated++;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingTty, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & (KSysGuard::Process::Usage | KSysGuard::Process::Status) || (process->changes & KSysGuard::Process::TotalUsage && mShowChildTotals)) {
            totalUpdated+=2;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingCPUUsage, process);
            emit q->dataChanged(index, index);
            index = q->createIndex(row, ProcessModel::HeadingCPUTime, process);
            emit q->dataChanged(index, index);
            //Because of our sorting, changing usage needs to also invalidate the User column
            index = q->createIndex(row, ProcessModel::HeadingUser, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & KSysGuard::Process::NiceLevels) {
            totalUpdated++;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingNiceness, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & KSysGuard::Process::VmSize) {
            totalUpdated++;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingVmSize, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & (KSysGuard::Process::VmSize | KSysGuard::Process::VmRSS | KSysGuard::Process::VmURSS)) {
            totalUpdated+=2;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingMemory, process);
            emit q->dataChanged(index, index);
            QModelIndex index2 = q->createIndex(row, ProcessModel::HeadingSharedMemory, process);
            emit q->dataChanged(index2, index2);
            //Because of our sorting, changing usage needs to also invalidate the User column
            index = q->createIndex(row, ProcessModel::HeadingUser, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & KSysGuard::Process::Name) {
            totalUpdated++;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingName, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & KSysGuard::Process::Command) {
            totalUpdated++;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingCommand, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & KSysGuard::Process::Login) {
            totalUpdated++;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingUser, process);
            emit q->dataChanged(index, index);
        }
        if(process->changes & KSysGuard::Process::IO) {
            totalUpdated++;
            QModelIndex index = q->createIndex(row, ProcessModel::HeadingIoRead, process);
            emit q->dataChanged(index, index);
            index = q->createIndex(row, ProcessModel::HeadingIoWrite, process);
            emit q->dataChanged(index, index);
        }
    }
}

void ProcessModelPrivate::beginInsertRow( KSysGuard::Process *process)
{
    Q_ASSERT(process);
    Q_ASSERT(!mRemovingRow);
    Q_ASSERT(!mInsertingRow);
    Q_ASSERT(!mMovingRow);
    mInsertingRow = true;

#ifdef Q_WS_X11
    process->hasManagedGuiWindow = mPidToWindowInfo.contains(process->pid);
#endif
    if(mSimple) {
        int row = mProcesses->processCount();
        q->beginInsertRows( QModelIndex(), row, row );
        return;
    }

    //Deal with the case that we are showing it as a tree
    int row = process->parent->children.count();
    QModelIndex parentModelIndex = q->getQModelIndex(process->parent, 0);

    //Only here can we actually change the model.  First notify the view/proxy models then modify
    q->beginInsertRows(parentModelIndex, row, row);
}
void ProcessModelPrivate::endInsertRow() {
    Q_ASSERT(!mRemovingRow);
    Q_ASSERT(mInsertingRow);
    Q_ASSERT(!mMovingRow);
    mInsertingRow = false;

    q->endInsertRows();
}
void ProcessModelPrivate::beginRemoveRow( KSysGuard::Process *process )
{
    Q_ASSERT(process);
    Q_ASSERT(process->pid >= 0);
    Q_ASSERT(!mRemovingRow);
    Q_ASSERT(!mInsertingRow);
    Q_ASSERT(!mMovingRow);
    mRemovingRow = true;

    if(mSimple) {
        return q->beginRemoveRows(QModelIndex(), process->index, process->index);
    } else  {
        int row = process->parent->children.indexOf(process);
        if(row == -1) {
            kDebug(1215) << "A serious problem occurred in remove row.";
            mRemovingRow = false;
            return;
        }

        return q->beginRemoveRows(q->getQModelIndex(process->parent,0), row, row);
    }
}
void ProcessModelPrivate::endRemoveRow()
{
    Q_ASSERT(!mInsertingRow);
    Q_ASSERT(!mMovingRow);
    if(!mRemovingRow)
        return;
    mRemovingRow = false;

    q->endRemoveRows();
}


void ProcessModelPrivate::beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent)
{
    Q_ASSERT(!mRemovingRow);
    Q_ASSERT(!mInsertingRow);
    Q_ASSERT(!mMovingRow);

    if(mSimple) return;  //We don't need to move processes when in simple mode
    mMovingRow = true;

    int current_row = process->parent->children.indexOf(process);
    Q_ASSERT(current_row != -1);
    int new_row = new_parent->children.count();
    QModelIndex sourceParent = q->getQModelIndex( process->parent, 0);
    QModelIndex destinationParent = q->getQModelIndex( new_parent, 0 );
    mMovingRow = q->beginMoveRows(sourceParent, current_row, current_row, destinationParent, new_row);
    Q_ASSERT(mMovingRow);
}
void ProcessModelPrivate::endMoveRow()
{
    Q_ASSERT(!mInsertingRow);
    Q_ASSERT(!mRemovingRow);
    if(!mMovingRow)
        return;
    mMovingRow = false;

    q->endMoveRows();
}


QModelIndex ProcessModel::getQModelIndex( KSysGuard::Process *process, int column) const
{
    Q_ASSERT(process);
    int pid = process->pid;
    if (pid == -1) return QModelIndex(); //pid -1 is our fake process meaning the very root (never drawn).  To represent that, we return QModelIndex() which also means the top element
    int row = 0;
    if(d->mSimple) {
        row = process->index;
    } else {
        row = process->parent->children.indexOf(process);
    }
    Q_ASSERT(row != -1);
    return createIndex(row, column, process);
}

QModelIndex ProcessModel::parent ( const QModelIndex & index ) const
{
    if(!index.isValid()) return QModelIndex();
    KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
    Q_ASSERT(process);

    if(d->mSimple)
        return QModelIndex();
    else
        return getQModelIndex(process->parent,0);
}

QVariant ProcessModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
    if(orientation != Qt::Horizontal)
        return QVariant();
    if(section < 0 || section >= d->mHeadings.count())
        return QVariant(); //is this needed?
    switch( role ) {
      case Qt::TextAlignmentRole:
      {
        switch(section) {
            case HeadingPid:
            case HeadingMemory:
            case HeadingXMemory:
            case HeadingSharedMemory:
            case HeadingIoRead:
            case HeadingIoWrite:
            case HeadingVmSize:
            case HeadingNiceness:
    //            return QVariant(Qt::AlignRight);
            case HeadingUser:
            case HeadingCPUUsage:
                return QVariant(Qt::AlignCenter);

        }
        return QVariant();
      }
      case Qt::ToolTipRole:
      {
          if(!d->mShowingTooltips)
            return QVariant();
          switch(section) {
            case HeadingName:
                return i18n("The process name.");
            case HeadingUser:
                return i18n("The user who owns this process.");
            case HeadingTty:
                return i18n("The controlling terminal on which this process is running.");
            case HeadingNiceness:
                return i18n("The priority with which this process is being run. For the normal scheduler, this ranges from 19 (very nice, least priority) to -19 (top priority).");
            case HeadingCPUUsage:
                if(d->mNumProcessorCores == 1)
                    return i18n("The current CPU usage of the process.");
                else
                    // i18n: %1 is always greater than 1, so do not worry about
                    // nonsensical verbosity of the singular part.
                    if(d->mNormalizeCPUUsage)
                        return i18np("The current total CPU usage of the process, divided by the %1 processor core in the machine.", "The current total CPU usage of the process, divided by the %1 processor cores in the machine.", d->mNumProcessorCores);
                    else
                        return i18n("The current total CPU usage of the process.");
            case HeadingCPUTime:
                return i18n("<qt>The total user and system time that this process has been running for, displayed as minutes:seconds.");
            case HeadingVmSize:
                return i18n("<qt>This is the amount of virtual memory space that the process is using, included shared libraries, graphics memory, files on disk, and so on. This number is almost meaningless.</qt>");
            case HeadingMemory:
                return i18n("<qt>This is the amount of real physical memory that this process is using by itself, and approximates the Private memory usage of the process.<br>It does not include any swapped out memory, nor the code size of its shared libraries.<br>This is often the most useful figure to judge the memory use of a program.  See What's This for more information.</qt>");
            case HeadingSharedMemory:
                return i18n("<qt>This is approximately the amount of real physical memory that this process's shared libraries are using.<br>This memory is shared among all processes that use this library.</qt>");
            case HeadingCommand:
                return i18n("<qt>The command with which this process was launched.</qt>");
            case HeadingXMemory:
                return i18n("<qt>The amount of pixmap memory that this process is using.</qt>");
            case HeadingXTitle:
                return i18n("<qt>The title of any windows that this process is showing.</qt>");
            case HeadingPid:
                return i18n("The unique Process ID that identifies this process.");
            case HeadingIoRead:
                return i18n("The number of bytes read.  See What's This for more information.");
            case HeadingIoWrite:
                return i18n("The number of bytes written.  See What's This for more information.");
            default:
                return QVariant();
          }
      }
      case Qt::WhatsThisRole:
      {
          switch(section) {
            case HeadingName:
                return i18n("<qt><i>Technical information: </i>The kernel process name is a maximum of 8 characters long, so the full command is examined.  If the first word in the full command line starts with the process name, the first word of the command line is shown, otherwise the process name is used.");
            case HeadingUser:
                return i18n("<qt>The user who owns this process.  If the effective, setuid etc user is different, the user who owns the process will be shown, followed by the effective user.  The ToolTip contains the full information.  <p>"
                        "<table>"
                          "<tr><td>Login Name/Group</td><td>The username of the Real User/Group who created this process</td></tr>"
                          "<tr><td>Effective User/Group</td><td>The process is running with privileges of the Effective User/Group.  This is shown if different from the real user.</td></tr>"
                          "<tr><td>Setuid User/Group</td><td>The saved username of the binary.  The process can escalate its Effective User/Group to the Setuid User/Group.</td></tr>"
#ifdef Q_OS_LINUX
                          "<tr><td>File System User/Group</td><td>Accesses to the filesystem are checked with the File System User/Group.  This is a Linux specific call. See setfsuid(2) for more information.</td></tr>"
#endif
                        "</table>");
            case HeadingVmSize:
                return i18n("<qt>This is the size of allocated address space - not memory, but address space. This value in practice means next to nothing. When a process requests a large memory block from the system but uses only a small part of it, the real usage will be low, VIRT will be high. <p><i>Technical information: </i>This is VmSize in proc/*/status and VIRT in top.");
            case HeadingMemory:
                return i18n("<qt><i>Technical information: </i>This is an approximation of the Private memory usage, calculated as VmRSS - Shared, from /proc/*/statm.  This tends to underestimate the true Private memory usage of a process (by not including i/o backed memory pages), but is the best estimation that is fast to determine.  This is sometimes known as URSS (Unique Resident Set Size). For an individual process, see \"Detailed  Memory Information\" for a more accurate, but slower, calculation of the true Private memory usage.");
            case HeadingCPUUsage:
                return i18n("The CPU usage of a process and all of its threads.");
            case HeadingCPUTime:
                return i18n("<qt>The total system and user time that a process and all of its threads have been running on the CPU for. This can be greater than the wall clock time if the process has been across multiple CPU cores.");
            case HeadingSharedMemory:
                return i18n("<qt><i>Technical information: </i>This is an approximation of the Shared memory, called SHR in top.  It is the number of pages that are backed by a file (see kernel Documentation/filesystems/proc.txt).  For an individual process, see \"Detailed Memory Information\" for a more accurate, but slower, calculation of the true Shared memory usage.");
            case HeadingCommand:
                return i18n("<qt><i>Technical information: </i>This is from /proc/*/cmdline");
            case HeadingXMemory:
                return i18n("<qt><i>Technical information: </i>This is the amount of memory used by the Xorg process for images for this process.  This is memory used in addition to Memory and Shared Memory.<br><i>Technical information: </i>This only counts the pixmap memory, and does not include resource memory used by fonts, cursors, glyphsets etc.  See the <code>xrestop</code> program for a more detailed breakdown.");
            case HeadingXTitle:
                return i18n("<qt><i>Technical information: </i>For each X11 window, the X11 property _NET_WM_PID is used to map the window to a PID.  If a process' windows are not shown, then that application incorrectly is not setting _NET_WM_PID.");
            case HeadingPid:
                return i18n("<qt><i>Technical information: </i>This is the Process ID.  A multi-threaded application is treated a single process, with all threads sharing the same PID.  The CPU usage etc will be the total, accumulated, CPU usage of all the threads.");
            case HeadingIoRead:
            case HeadingIoWrite:
                return i18n("<qt>This column shows the IO statistics for each process. The tooltip provides the following information:<br>"
                        "<table>"
                        "<tr><td>Characters Read</td><td>The number of bytes which this task has caused to be read from storage. This is simply the sum of bytes which this process passed to read() and pread(). It includes things like tty IO and it is unaffected by whether or not actual physical disk IO was required (the read might have been satisfied from pagecache).</td></tr>"
                        "<tr><td>Characters Written</td><td>The number of bytes which this task has caused, or shall cause to be written to disk. Similar caveats apply here as with Characters Read.</td></tr>"
                        "<tr><td>Read Syscalls</td><td>The number of read I/O operations, i.e. syscalls like read() and pread().</td></tr>"
                        "<tr><td>Write Syscalls</td><td>The number of write I/O operations, i.e. syscalls like write() and pwrite().</td></tr>"
                        "<tr><td>Actual Bytes Read</td><td>The number of bytes which this process really did cause to be fetched from the storage layer. Done at the submit_bio() level, so it is accurate for block-backed filesystems. This may not give sensible values for NFS and CIFS filesystems.</td></tr>"
                        "<tr><td>Actual Bytes Written</td><td>Attempt to count the number of bytes which this process caused to be sent to the storage layer. This is done at page-dirtying time.</td>"
                        "</table><p>"
                        "The number in brackets shows the rate at which each value is changing, determined from taking the difference between the previous value and the new value, and dividing by the update interval.<p>"
                        "<i>Technical information: </i>This data is collected from /proc/*/io and is documented further in Documentation/accounting and Documentation/filesystems/proc.txt in the kernel source.");
            default:
                return QVariant();
          }
      }
      case Qt::DisplayRole:
        return d->mHeadings[section];
      default:
        return QVariant();
    }
}
void ProcessModel::setSimpleMode(bool simple)
{
    if(d->mSimple == simple) return;

    emit layoutAboutToBeChanged ();

    d->mSimple = simple;

    int flatrow;
    int treerow;
    QList<QModelIndex> flatIndexes;
    QList<QModelIndex> treeIndexes;
    foreach( KSysGuard::Process *process, d->mProcesses->getAllProcesses()) {
        flatrow = process->index;
        treerow = process->parent->children.indexOf(process);
        flatIndexes.clear();
        treeIndexes.clear();

        for(int i=0; i < columnCount(); i++) {
            flatIndexes << createIndex(flatrow, i, process);
            treeIndexes << createIndex(treerow, i, process);
        }
        if(d->mSimple) //change from tree mode to flat mode
            changePersistentIndexList(treeIndexes, flatIndexes);
        else // change from flat mode to tree mode
            changePersistentIndexList(flatIndexes, treeIndexes);
    }

    emit layoutChanged();
}

bool ProcessModel::canUserLogin(long uid ) const
{
    if(uid == 65534) {
        //nobody user
        return false;
    }

    if(!d->mIsLocalhost) return true; //We only deal with localhost.  Just always return true for non localhost

    int canLogin = d->mUidCanLogin.value(uid, -1); //Returns 0 if we cannot login, 1 if we can, and the default is -1 meaning we don't know
    if(canLogin != -1) return canLogin; //We know whether they can log in

    //We got the default, -1, so we don't know.  Look it up

    KUser user(uid);
    if(!user.isValid()) {
        //for some reason the user isn't recognised.  This might happen under certain security situations.
        //Just return true to be safe
        d->mUidCanLogin[uid] = 1;
        return true;
    }
    QString shell = user.shell();
    if(shell == "/bin/false" )  //FIXME - add in any other shells it could be for false
    {
        d->mUidCanLogin[uid] = 0;
        return false;
    }
    d->mUidCanLogin[uid] = 1;
    return true;
}

QString ProcessModelPrivate::getTooltipForUser(const KSysGuard::Process *ps) const
{
    QString userTooltip = "<qt><p style='white-space:pre'>";
    if(!mIsLocalhost) {
        userTooltip += i18n("Login Name: %1<br/>", getUsernameForUser(ps->uid, true));
    } else {
        KUser user(ps->uid);
        if(!user.isValid())
            userTooltip += i18n("This user is not recognized for some reason.");
        else {
            if(!user.property(KUser::FullName).isValid())
                userTooltip += i18n("<b>%1</b><br/>", user.property(KUser::FullName).toString());
            userTooltip += i18n("Login Name: %1 (uid: %2)<br/>", user.loginName(), QString::number(ps->uid));
            if(!user.property(KUser::RoomNumber).isValid())
                userTooltip += i18n("  Room Number: %1<br/>", user.property(KUser::RoomNumber).toString());
            if(!user.property(KUser::WorkPhone).isValid())
                userTooltip += i18n("  Work Phone: %1<br/>", user.property(KUser::WorkPhone).toString());
        }
    }
    if( (ps->uid != ps->euid && ps->euid != -1) ||
                   (ps->uid != ps->suid && ps->suid != -1) ||
                   (ps->uid != ps->fsuid && ps->fsuid != -1)) {
        if(ps->euid != -1)
            userTooltip += i18n("Effective User: %1<br/>", getUsernameForUser(ps->euid, true));
        if(ps->suid != -1)
            userTooltip += i18n("Setuid User: %1<br/>", getUsernameForUser(ps->suid, true));
        if(ps->fsuid != -1)
            userTooltip += i18n("File System User: %1<br/>", getUsernameForUser(ps->fsuid, true));
        userTooltip += "<br/>";
    }
    if(ps->gid != -1) {
        userTooltip += i18n("Group: %1", getGroupnameForGroup(ps->gid));
        if( (ps->gid != ps->egid && ps->egid != -1) ||
                       (ps->gid != ps->sgid && ps->sgid != -1) ||
                       (ps->gid != ps->fsgid && ps->fsgid != -1)) {
            if(ps->egid != -1)
                userTooltip += i18n("<br/>Effective Group: %1", getGroupnameForGroup(ps->egid));
            if(ps->sgid != -1)
                userTooltip += i18n("<br/>Setuid Group: %1", getGroupnameForGroup(ps->sgid));
            if(ps->fsgid != -1)
                userTooltip += i18n("<br/>File System Group: %1", getGroupnameForGroup(ps->fsgid));
        }
    }
    return userTooltip;
}

QString ProcessModel::getStringForProcess(KSysGuard::Process *process) const {
    return i18nc("Short description of a process. PID, name, user", "<numid>%1</numid>: %2, owned by user %3", (long)(process->pid), process->name, d->getUsernameForUser(process->uid, false));
}

QString ProcessModelPrivate::getGroupnameForGroup(long gid) const {
    if(mIsLocalhost) {
        QString groupname = KUserGroup(gid).name();
        if(!groupname.isEmpty())
            return i18n("%1 (gid: <numid>%2</numid>)", groupname, gid);
    }
    return QString::number(gid);
}

QString ProcessModelPrivate::getUsernameForUser(long uid, bool withuid) const {
    QString &username = mUserUsername[uid];
    if(username.isNull()) {
        if(!mIsLocalhost) {
            username = ""; //empty, but not null
        } else {
            KUser user(uid);
            if(!user.isValid())
                username = "";
            else
                username = user.loginName();
        }
    }
    if(username.isEmpty())
        return QString::number(uid);
    if(withuid)
        return i18n("%1 (uid: %2)", username, (long int)uid);
    return username;
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
    //This function must be super duper ultra fast because it's called thousands of times every few second :(
    //I think it should be optomised for role first, hence the switch statement (fastest possible case)

    if (!index.isValid()) {
        return QVariant();
    }
    if (index.column() >= d->mHeadings.count()) {
        return QVariant();
    }

    switch (role){
    case Qt::DisplayRole: {
        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
        switch(index.column()) {
        case HeadingName:
            if(d->mShowCommandLineOptions)
                return process->name;
            else
                return process->name.section(' ', 0,0);
        case HeadingPid:
            return (qlonglong)process->pid;
        case HeadingUser:
            if(!process->login.isEmpty()) return process->login;
            if(process->uid == process->euid)
                return d->getUsernameForUser(process->uid, false);
            else
                return QString(d->getUsernameForUser(process->uid, false) + ", " + d->getUsernameForUser(process->euid, false));
        case HeadingNiceness:
            switch(process->scheduler) {
              case KSysGuard::Process::Other:
                  return process->niceLevel;
              case KSysGuard::Process::SchedulerIdle:
                  return i18nc("scheduler", "Idle"); //neither static nor dynamic priority matter
              case KSysGuard::Process::Batch:
                  return i18nc("scheduler", "(Batch) %1", process->niceLevel); //only dynamic priority matters
              case KSysGuard::Process::RoundRobin:
                  return i18nc("Round robin scheduler", "RR %1", process->niceLevel);
              case KSysGuard::Process::Fifo:
                  if(process->niceLevel == 99)
                      return i18nc("Real Time scheduler", "RT");
                  else
                      return i18nc("First in first out scheduler", "FIFO %1", process->niceLevel);
              case KSysGuard::Process::Interactive:
                  return i18nc("scheduler", "(IA) %1", process->niceLevel);
            }
        case HeadingTty:
            return process->tty;
        case HeadingCPUUsage:
            {
                double total;
                if(d->mShowChildTotals && !d->mSimple) total = process->totalUserUsage + process->totalSysUsage;
                else total = process->userUsage + process->sysUsage;
                if(d->mNormalizeCPUUsage)
                    total = total / d->mNumProcessorCores;

                if(total < 1 && process->status != KSysGuard::Process::Sleeping && process->status != KSysGuard::Process::Running && process->status != KSysGuard::Process::Ended)
                    return process->translatedStatus();  //tell the user when the process is a zombie or stopped
                if(total < 0.5)
                    return "";

                return QString(QString::number((int)(total+0.5)) + '%');
            }
        case HeadingCPUTime: {
            qlonglong seconds = (process->userTime + process->sysTime)/100;
            return QString("%1:%2").arg(seconds/60).arg((int)seconds%60, 2, 10, QLatin1Char('0'));
        }
        case HeadingMemory:
            if(process->vmURSS == -1) {
                //If we don't have the URSS (the memory used by only the process, not the shared libraries)
                //then return the RSS (physical memory used by the process + shared library) as the next best thing
                return formatMemoryInfo(process->vmRSS, d->mUnits, true);
            } else {
                return formatMemoryInfo(process->vmURSS, d->mUnits, true);
            }
        case HeadingVmSize:
            return formatMemoryInfo(process->vmSize, d->mUnits, true);
        case HeadingSharedMemory:
            if(process->vmRSS - process->vmURSS <= 0 || process->vmURSS == -1) return QVariant(QVariant::String);
            return formatMemoryInfo(process->vmRSS - process->vmURSS, d->mUnits);
        case HeadingCommand:
            {
		return process->command.replace('\n',' ');
// It would be nice to embolden the process name in command, but this requires that the itemdelegate to support html text
//                QString command = process->command;
//                command.replace(process->name, "<b>" + process->name + "</b>");
//                return "<qt>" + command;
            }
        case HeadingIoRead:
            {
                switch(d->mIoInformation) {
                    case ProcessModel::Bytes:  //divide by 1024 to convert to kB
                        return formatMemoryInfo(process->ioCharactersRead/1024, d->mIoUnits, true);
                    case ProcessModel::Syscalls:
                        if( process->ioReadSyscalls )
                            return QString::number(process->ioReadSyscalls);
                        break;
                    case ProcessModel::ActualBytes:
                        return formatMemoryInfo(process->ioCharactersActuallyRead/1024, d->mIoUnits, true);
                    case ProcessModel::BytesRate:
                        if( process->ioCharactersReadRate/1024 )
                            return i18n("%1/s", formatMemoryInfo(process->ioCharactersReadRate/1024, d->mIoUnits, true));
                        break;
                    case ProcessModel::SyscallsRate:
                        if( process->ioReadSyscallsRate )
                            return QString::number(process->ioReadSyscallsRate);
                        break;
                    case ProcessModel::ActualBytesRate:
                        if( process->ioCharactersActuallyReadRate/1024 )
                            return i18n("%1/s", formatMemoryInfo(process->ioCharactersActuallyReadRate/1024, d->mIoUnits, true));
                        break;
                }
                return QVariant();
            }
        case HeadingIoWrite:
            {
                switch(d->mIoInformation) {
                    case ProcessModel::Bytes:
                        return formatMemoryInfo(process->ioCharactersWritten/1024, d->mIoUnits, true);
                    case ProcessModel::Syscalls:
                        if( process->ioWriteSyscalls )
                            return QString::number(process->ioWriteSyscalls);
                        break;
                    case ProcessModel::ActualBytes:
                        return formatMemoryInfo(process->ioCharactersActuallyWritten/1024, d->mIoUnits, true);
                    case ProcessModel::BytesRate:
                        if(process->ioCharactersWrittenRate/1024)
                            return i18n("%1/s", formatMemoryInfo(process->ioCharactersWrittenRate/1024, d->mIoUnits, true));
                        break;
                    case ProcessModel::SyscallsRate:
                        if( process->ioWriteSyscallsRate )
                            return QString::number(process->ioWriteSyscallsRate);
                        break;
                    case ProcessModel::ActualBytesRate:
                        if(process->ioCharactersActuallyWrittenRate/1024)
                            return i18n("%1/s", formatMemoryInfo(process->ioCharactersActuallyWrittenRate/1024, d->mIoUnits, true));
                        break;
                }
                return QVariant();
            }
#ifdef Q_WS_X11
        case HeadingXMemory:
            return formatMemoryInfo(process->pixmapBytes/1024, d->mUnits, true);
        case HeadingXTitle:
            {
                if(!process->hasManagedGuiWindow)
                    return QVariant(QVariant::String);

                WindowInfo *w = d->mPidToWindowInfo.value(process->pid, NULL);
                if(!w)
                    return QVariant(QVariant::String);
                else
                    return w->name;
            }
#endif
        default:
            return QVariant();
        }
        break;
    }
    case Qt::ToolTipRole: {
        if(!d->mShowingTooltips)
            return QVariant();
        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
        QString tracer;
        if(process->tracerpid >= 0) {
            KSysGuard::Process *process_tracer = d->mProcesses->getProcess(process->tracerpid);
            if(process_tracer) //it is possible for this to be not the case in certain race conditions
                tracer = i18nc("tooltip. name,pid ","This process is being debugged by %1 (<numid>%2</numid>)", process_tracer->name, (long int)process->tracerpid);
        }
        switch(index.column()) {
        case HeadingName: {
            QString tooltip = "<qt><p style='white-space:pre'>";
            /*   It would be nice to be able to show the icon in the tooltip, but Qt4 won't let us put
             *   a picture in a tooltip :(

            QIcon icon;
            if(mPidToWindowInfo.contains(process->pid)) {
                WId wid;
                wid = mPidToWindowInfo[process->pid].wid;
                icon = KWindowSystem::icon(wid);
            }
            if(icon.isValid()) {
                tooltip = i18n("<qt><table><tr><td>%1", icon);
            }
            */
            if(process->parent_pid == -1) {
                //Give a quick explanation of init and kthreadd
                if(process->name == "init") {
                    tooltip += i18n("<b>Init</b> is the parent of all other processes and cannot be killed.<br/>");
                } else if(process->name == "kthreadd") {
                    tooltip += i18n("<b>KThreadd</b> manages kernel threads. The children processes run in the kernel, controlling hard disk access, etc.<br/>");
                }
                tooltip    += i18nc("name column tooltip. first item is the name","<b>%1</b><br />Process ID: <numid>%2</numid>", process->name, (long int)process->pid);
            }
            else {
                KSysGuard::Process *parent_process = d->mProcesses->getProcess(process->parent_pid);
                if(parent_process) { //In race conditions, it's possible for this process to not exist
                    tooltip    = i18nc("name column tooltip. first item is the name","<b>%1</b><br />Process ID: <numid>%2</numid><br />Parent: %3<br />Parent's ID: <numid>%4</numid>", process->name, (long int)process->pid, parent_process->name, (long int)process->parent_pid);
                } else {
                    tooltip    = i18nc("name column tooltip. first item is the name","<b>%1</b><br />Process ID: <numid>%2</numid><br />Parent's ID: <numid>%3</numid>", process->name, (long int)process->pid, (long int)process->parent_pid);
                }
            }
            if(process->numThreads >= 1)
                tooltip += i18n("<br/>Number of threads: %1", process->numThreads);
            if(!process->command.isEmpty()) {
                tooltip += i18n("<br/>Command: %1", process->command);
            }
            if(!process->tty.isEmpty())
                tooltip += i18n( "<br />Running on: %1", QString(process->tty));
            if(!tracer.isEmpty())
                return QString::fromLatin1("%1<br />%2").arg(tooltip).arg(tracer);
            return tooltip;
        }

        case HeadingCommand: {
            QString tooltip =
                i18n("<qt>This process was run with the following command:<br />%1", process->command);
            if(!process->tty.isEmpty())
                tooltip += i18n( "<br /><br />Running on: %1", QString(process->tty));
            if(tracer.isEmpty()) return tooltip;
            return QString::fromLatin1("%1<br />%2").arg(tooltip).arg(tracer);
        }
        case HeadingUser: {
            if(!tracer.isEmpty())
                return QString(d->getTooltipForUser(process) + "<br />" + tracer);
            return d->getTooltipForUser(process);
        }
        case HeadingNiceness: {
            QString tooltip = "<qt><p style='white-space:pre'>";
            switch(process->scheduler) {
              case KSysGuard::Process::Other:
              case KSysGuard::Process::Batch:
              case KSysGuard::Process::Interactive:
                  tooltip += i18n("Nice level: %1 (%2)", process->niceLevel, process->niceLevelAsString() );
                  break;
              case KSysGuard::Process::RoundRobin:
              case KSysGuard::Process::Fifo:
                  tooltip += i18n("This is a real time process.<br>Scheduler priority: %1", process->niceLevel);
                  break;
              case KSysGuard::Process::SchedulerIdle:
                  break; //has neither dynamic (niceness) or static (scheduler priority) priotiy
            }
            if(process->scheduler != KSysGuard::Process::Other)
                tooltip += i18n("<br/>Scheduler: %1", process->schedulerAsString());

            if(process->ioPriorityClass != KSysGuard::Process::None) {
                if((process->ioPriorityClass == KSysGuard::Process::RealTime || process->ioPriorityClass == KSysGuard::Process::BestEffort) && process->ioniceLevel != -1)
                    tooltip += i18n("<br/>I/O Nice level: %1 (%2)", process->ioniceLevel, process->ioniceLevelAsString() );
                tooltip += i18n("<br/>I/O Class: %1", process->ioPriorityClassAsString() );
            }
                if(tracer.isEmpty()) return tooltip;
            return QString(tooltip + "<br />" + tracer);
        }
        case HeadingCPUUsage:
        case HeadingCPUTime: {
            int divideby = (d->mNormalizeCPUUsage?d->mNumProcessorCores:1);
            QString tooltip = ki18n("<qt><p style='white-space:pre'>"
                        "Process status: %1 %2<br />"
                        "User CPU usage: %3%<br />"
                        "System CPU usage: %4%")  /* Please do not add </qt> here - the tooltip is appended to */
                        .subs(process->translatedStatus())
                        .subs(d->getStatusDescription(process->status))
                        .subs((float)(process->userUsage) / divideby)
                        .subs((float)(process->sysUsage) / divideby)
                        .toString();

            if(process->numThreads >= 1)
                tooltip += i18n("<br/>Number of threads: %1", process->numThreads);
            if(process->numChildren > 0) {
                tooltip += ki18n("<br />Number of children: %1<br />Total User CPU usage: %2%<br />"
                        "Total System CPU usage: %3%<br />Total CPU usage: %4%")
                        .subs(process->numChildren)
                        .subs((float)(process->totalUserUsage)/ divideby)
                        .subs((float)(process->totalSysUsage) / divideby)
                        .subs((float)(process->totalUserUsage + process->totalSysUsage) / divideby)
                        .toString();
            }
            if(process->userTime > 0)
                tooltip += ki18n("<br /><br />CPU time spent running as user: %1 seconds")
                        .subs(process->userTime / 100.0, 0, 'f', 1)
                        .toString();
            if(process->sysTime > 0)
                tooltip += ki18n("<br />CPU time spent running in kernel: %1 seconds")
                        .subs(process->sysTime / 100.0, 0, 'f', 1)
                        .toString();
            if(process->niceLevel != 0)
                tooltip += i18n("<br />Nice level: %1 (%2)", process->niceLevel, process->niceLevelAsString() );
            if(process->ioPriorityClass != KSysGuard::Process::None) {
                if((process->ioPriorityClass == KSysGuard::Process::RealTime || process->ioPriorityClass == KSysGuard::Process::BestEffort) && process->ioniceLevel != -1)
                    tooltip += i18n("<br/>I/O Nice level: %1 (%2)", process->ioniceLevel, process->ioniceLevelAsString() );
                tooltip += i18n("<br/>I/O Class: %1", process->ioPriorityClassAsString() );
            }

            if(!tracer.isEmpty())
                return QString(tooltip + "<br />" + tracer);
            return tooltip;
        }
        case HeadingVmSize: {
            return QVariant();
        }
        case HeadingMemory: {
            QString tooltip = "<qt><p style='white-space:pre'>";
            if(process->vmURSS != -1) {
                //We don't have information about the URSS, so just fallback to RSS
                if(d->mMemTotal > 0)
                    tooltip += i18n("Memory usage: %1 out of %2  (%3 %)<br />", KGlobal::locale()->formatByteSize(process->vmURSS * 1024), KGlobal::locale()->formatByteSize(d->mMemTotal*1024), process->vmURSS*100/d->mMemTotal);
                else
                    tooltip += i18n("Memory usage: %1<br />", KGlobal::locale()->formatByteSize(process->vmURSS * 1024));
            }
            if(d->mMemTotal > 0)
                tooltip += i18n("RSS Memory usage: %1 out of %2  (%3 %)", KGlobal::locale()->formatByteSize(process->vmRSS * 1024), KGlobal::locale()->formatByteSize(d->mMemTotal*1024), process->vmRSS*100/d->mMemTotal);
            else
                tooltip += i18n("RSS Memory usage: %1", KGlobal::locale()->formatByteSize(process->vmRSS * 1024));
            return tooltip;
        }
        case HeadingSharedMemory: {
            QString tooltip = "<qt><p style='white-space:pre'>";
            if(process->vmURSS == -1) {
                tooltip += i18n("Your system does not seem to have this information available to be read.");
                return tooltip;
            }
            if(d->mMemTotal >0)
                tooltip += i18n("Shared library memory usage: %1 out of %2  (%3 %)", KGlobal::locale()->formatByteSize((process->vmRSS - process->vmURSS) * 1024), KGlobal::locale()->formatByteSize(d->mMemTotal*1024), (process->vmRSS-process->vmURSS)*100/d->mMemTotal);
            else
                tooltip += i18n("Shared library memory usage: %1", KGlobal::locale()->formatByteSize((process->vmRSS - process->vmURSS) * 1024));

            return tooltip;
        }
        case HeadingIoWrite:
        case HeadingIoRead: {
            QString tooltip = "<qt><p style='white-space:pre'>";
            //FIXME - use the formatByteRate functions when added
            tooltip += ki18n("Characters read: %1 (%2 KiB/s)<br>Characters written: %3 (%4 KiB/s)<br>Read syscalls: %5 (%6 s⁻¹)<br>Write syscalls: %7 (%8 s⁻¹)<br>Actual bytes read: %9 (%10 KiB/s)<br>Actual bytes written: %11 (%12 KiB/s)")
                .subs(KGlobal::locale()->formatByteSize(process->ioCharactersRead))
                .subs(QString::number(process->ioCharactersReadRate/1024))
                .subs(KGlobal::locale()->formatByteSize(process->ioCharactersWritten))
                .subs(QString::number(process->ioCharactersWrittenRate/1024))
                .subs(QString::number(process->ioReadSyscalls))
                .subs(QString::number(process->ioReadSyscallsRate))
                .subs(QString::number(process->ioWriteSyscalls))
                .subs(QString::number(process->ioWriteSyscallsRate))
                .subs(KGlobal::locale()->formatByteSize(process->ioCharactersActuallyRead))
                .subs(QString::number(process->ioCharactersActuallyReadRate/1024 ))
                .subs(KGlobal::locale()->formatByteSize(process->ioCharactersActuallyWritten))
                .subs(QString::number(process->ioCharactersActuallyWrittenRate/1024))
                .toString();
            return tooltip;
        }
        case HeadingXTitle: {
            QString tooltip;
            QList<WindowInfo *> values = d->mPidToWindowInfo.values(process->pid);
            if(values.isEmpty()) return QVariant(QVariant::String);
            for(int i = 0; i < values.size(); i++) {
                if(!values.at(i)->name.isEmpty())
                    tooltip += "<li>" + values.at(i)->name + "</li>";
            }
            if(!tooltip.isEmpty())
                return QString("<qt><p style='white-space:pre'><ul>" + tooltip + "</ul>");
            return QVariant(QVariant::String);
        }

        default:
            return QVariant(QVariant::String);
        }
    }
    case Qt::TextAlignmentRole:
        switch(index.column() ) {
            case HeadingUser:
            case HeadingCPUUsage:
                return QVariant(Qt::AlignCenter);
            case HeadingNiceness:
            case HeadingCPUTime:
            case HeadingPid:
            case HeadingMemory:
            case HeadingXMemory:
            case HeadingSharedMemory:
            case HeadingVmSize:
            case HeadingIoWrite:
            case HeadingIoRead:
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);
            default:
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    case UidRole: {
        if(index.column() != 0) return QVariant();  //If we query with this role, then we want the raw UID for this.
        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
        return process->uid;
    }
    case PlainValueRole:  //Used to return a plain value.  For copying to a clipboard etc
    {
        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
        switch(index.column()) {
        case HeadingName:
            return process->name;
        case HeadingPid:
            return (qlonglong)process->pid;
        case HeadingUser:
            if(!process->login.isEmpty()) return process->login;
            if(process->uid == process->euid)
                return d->getUsernameForUser(process->uid, false);
            else
                return QString(d->getUsernameForUser(process->uid, false) + ", " + d->getUsernameForUser(process->euid, false));
        case HeadingNiceness:
            return process->niceLevel;
        case HeadingTty:
            return process->tty;
        case HeadingCPUUsage:
            {
                double total;
                if(d->mShowChildTotals && !d->mSimple) total = process->totalUserUsage + process->totalSysUsage;
                else total = process->userUsage + process->sysUsage;

                if(d->mNormalizeCPUUsage)
                    return total / d->mNumProcessorCores;
                else
                    return total;
            }
        case HeadingCPUTime:
            return (qlonglong)(process->userTime + process->sysTime);
        case HeadingMemory:
            if(process->vmRSS == 0) return QVariant(QVariant::String);
            if(process->vmURSS == -1) {
                return (qlonglong)process->vmRSS;
            } else {
                return (qlonglong)process->vmURSS;
            }
        case HeadingVmSize:
            return (qlonglong)process->vmSize;
        case HeadingSharedMemory:
            if(process->vmRSS - process->vmURSS < 0 || process->vmURSS == -1) return QVariant(QVariant::String);
            return (qlonglong)(process->vmRSS - process->vmURSS);
        case HeadingCommand:
            return process->command;
        case HeadingIoRead:
            switch(d->mIoInformation) {
                case ProcessModel::Bytes:
                    return process->ioCharactersRead;
                case ProcessModel::Syscalls:
                    return process->ioReadSyscalls;
                case ProcessModel::ActualBytes:
                    return process->ioCharactersActuallyRead;
                case ProcessModel::BytesRate:
                    return (qlonglong)process->ioCharactersReadRate;
                case ProcessModel::SyscallsRate:
                    return (qlonglong)process->ioReadSyscallsRate;
                case ProcessModel::ActualBytesRate:
                    return (qlonglong)process->ioCharactersActuallyReadRate;

            }
        case HeadingIoWrite:
            switch(d->mIoInformation) {
                case ProcessModel::Bytes:
                    return process->ioCharactersWritten;
                case ProcessModel::Syscalls:
                    return process->ioWriteSyscalls;
                case ProcessModel::ActualBytes:
                    return process->ioCharactersActuallyWritten;
                case ProcessModel::BytesRate:
                    return (qlonglong)process->ioCharactersWrittenRate;
                case ProcessModel::SyscallsRate:
                    return (qlonglong)process->ioWriteSyscallsRate;
                case ProcessModel::ActualBytesRate:
                    return (qlonglong)process->ioCharactersActuallyWrittenRate;

            }
        case HeadingXMemory:
            return (qulonglong)process->pixmapBytes;
#ifdef Q_WS_X11
        case HeadingXTitle:
            {
                WindowInfo *w = d->mPidToWindowInfo.value(process->pid, NULL);
                if(!w)
                    return QString();
                return w->name;
            }
#endif
        default:
            return QVariant();
        }
        break;
    }
#ifdef Q_WS_X11
        case WindowIdRole: {
        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
        WindowInfo *w = d->mPidToWindowInfo.value(process->pid, NULL);
        if(!w)
            return (int)0;
        else
            return (int)w->wid;
    }
#endif
    case PercentageRole: {
        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
        Q_CHECK_PTR(process);
        switch(index.column()) {
            case HeadingCPUUsage: {
                float cpu;
                if(d->mSimple || !d->mShowChildTotals)
                    cpu = process->userUsage + process->sysUsage;
                else
                    cpu = process->totalUserUsage + process->totalSysUsage;
                cpu = cpu / 100.0;
                if(!d->mNormalizeCPUUsage)
                    return cpu;
                return cpu / d->mNumProcessorCores;
            }
            case HeadingMemory:
                if(d->mMemTotal <= 0)
                    return -1;
                if(process->vmURSS != -1)
                    return float(process->vmURSS) / d->mMemTotal;
                else
                    return float(process->vmRSS) / d->mMemTotal;
            case HeadingSharedMemory:
                if(process->vmURSS == -1 || d->mMemTotal <= 0)
                    return -1;
                return float(process->vmRSS - process->vmURSS)/d->mMemTotal;
            default:
                return -1;
        }
    }
    case Qt::DecorationRole: {
        if(index.column() == HeadingName) {
#ifdef Q_WS_X11
            KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
            if(!process->hasManagedGuiWindow) {
                if(d->mSimple) //When not in tree mode, we need to pad the name column where we do not have an icon
                    return QIcon(d->mBlankPixmap);
                else  //When in tree mode, the padding looks bad, so do not pad in this case
                    return QVariant();
            }
            WindowInfo *w = d->mPidToWindowInfo.value(process->pid, NULL);
            if(w && !w->icon.isNull())
                return w->icon;
            return QIcon(d->mBlankPixmap);
#else
            return QVariant();
#endif

        } else if (index.column() == HeadingCPUUsage) {
            KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
            if(process->status == KSysGuard::Process::Stopped || process->status == KSysGuard::Process::Zombie) {
            //        QPixmap pix = KIconLoader::global()->loadIcon("button_cancel", KIconLoader::Small,
        //                    KIconLoader::SizeSmall, KIconLoader::DefaultState, QStringList(),
        //                0L, true);

            }
        }
        return QVariant();
    }
    case Qt::BackgroundRole: {
        if (index.column() != HeadingUser) {
            if (!d->mHaveTimer) //If there is no timer, then no processes are being killed, so no point looking for one
                return QVariant();
            KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
            if (!process->timeKillWasSent.isNull()) {
                int elapsed = process->timeKillWasSent.elapsed();
                if (elapsed < MILLISECONDS_TO_SHOW_RED_FOR_KILLED_PROCESS) {//Only show red for about 7 seconds
                    int transparency = 255 - elapsed*250/MILLISECONDS_TO_SHOW_RED_FOR_KILLED_PROCESS;

                    KColorScheme scheme(QPalette::Active, KColorScheme::Selection);
                    QBrush brush = scheme.background(KColorScheme::NegativeBackground);
                    QColor color = brush.color();
                    color.setAlpha(transparency);
                    brush.setColor(color);
                    return brush;
                }
            }
            return QVariant();
        }
        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
        if(process->status == KSysGuard::Process::Ended) {
            return QColor(Qt::lightGray);
        }
        if(process->tracerpid >= 0) {
            //It's being debugged, so probably important.  Let's mark it as such
            return QColor(Qt::yellow);
        }
        if(d->mIsLocalhost && process->uid == getuid()) { //own user
            return QColor(0, 208, 214, 50);
        }
        if(process->uid < 100 || !canUserLogin(process->uid))
            return QColor(218, 220,215, 50); //no color for system tasks
        //other users
        return QColor(2, 154, 54, 50);
    }
    case Qt::FontRole: {
        if(index.column() == HeadingCPUUsage) {
            KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
            if(process->userUsage == 0) {
                QFont font;
                font.setItalic(true);
                return font;
            }
        }
        return QVariant();
    }
    default: //This is a very very common case, so the route to this must be very minimal
        return QVariant();
    }

    return QVariant(); //never get here, but make compiler happy
}

bool ProcessModel::hasGUIWindow(qlonglong pid) const
{
#ifdef Q_WS_X11
    return d->mPidToWindowInfo.contains(pid);
#else
    return false;
#endif
}

bool ProcessModel::isLocalhost() const
{
    return d->mIsLocalhost;
}


void ProcessModel::setupHeader() {
    //These must be in the same order that they are in in the header file
    QStringList headings;
    headings << i18nc("process heading", "Name");
    headings << i18nc("process heading", "Username");
    headings << i18nc("process heading", "PID");
    headings << i18nc("process heading", "TTY");
    headings << i18nc("process heading", "Niceness");
    // xgettext: no-c-format
    headings << i18nc("process heading", "CPU %");
    headings << i18nc("process heading", "CPU Time");
    headings << i18nc("process heading", "IO Read");
    headings << i18nc("process heading", "IO Write");
    headings << i18nc("process heading", "Virtual Size");
    headings << i18nc("process heading", "Memory");
    headings << i18nc("process heading", "Shared Mem");
    headings << i18nc("process heading", "Command");
#ifdef Q_WS_X11
    headings << i18nc("process heading", "X11 Memory");
    headings << i18nc("process heading", "Window Title");
#endif

    if(d->mHeadings.isEmpty()) { // If it's empty, this is the first time this has been called, so insert the headings
        beginInsertColumns(QModelIndex(), 0, headings.count()-1);
        {
            d->mHeadings = headings;
        }
        endInsertColumns();
    } else {
        // This was called to retranslate the headings.  Just use the new translations and call headerDataChanged
        Q_ASSERT(d->mHeadings.count() == headings.count());
        d->mHeadings = headings;
        headerDataChanged(Qt::Horizontal, 0 , headings.count()-1);

    }
}

void ProcessModel::retranslateUi()
{
    setupHeader();
}

KSysGuard::Process *ProcessModel::getProcess(qlonglong pid) {
    return d->mProcesses->getProcess(pid);
}

bool ProcessModel::showTotals() const {
    return d->mShowChildTotals;
}

void ProcessModel::setShowTotals(bool showTotals)  //slot
{
    if(showTotals == d->mShowChildTotals) return;
    d->mShowChildTotals = showTotals;

    QModelIndex index;
    foreach( KSysGuard::Process *process, d->mProcesses->getAllProcesses()) {
        if(process->numChildren > 0) {
            int row;
            if(d->mSimple)
                row = process->index;
            else
                row = process->parent->children.indexOf(process);
            index = createIndex(row, HeadingCPUUsage, process);
            emit dataChanged(index, index);
        }
    }
}

qlonglong ProcessModel::totalMemory() const
{
    return d->mMemTotal;
}
void ProcessModel::setUnits(Units units)
{
    if(d->mUnits == units)
        return;
    d->mUnits = units;

    QModelIndex index;
    foreach( KSysGuard::Process *process, d->mProcesses->getAllProcesses()) {
        int row;
        if(d->mSimple)
            row = process->index;
        else
            row = process->parent->children.indexOf(process);
        index = createIndex(row, HeadingMemory, process);
        emit dataChanged(index, index);
        index = createIndex(row, HeadingXMemory, process);
        emit dataChanged(index, index);
        index = createIndex(row, HeadingSharedMemory, process);
        emit dataChanged(index, index);
        index = createIndex(row, HeadingVmSize, process);
        emit dataChanged(index, index);
    }
}
ProcessModel::Units ProcessModel::units() const
{
    return (Units) d->mUnits;
}
void ProcessModel::setIoUnits(Units units)
{
    if(d->mIoUnits == units)
        return;
    d->mIoUnits = units;

    QModelIndex index;
    foreach( KSysGuard::Process *process, d->mProcesses->getAllProcesses()) {
        int row;
        if(d->mSimple)
            row = process->index;
        else
            row = process->parent->children.indexOf(process);
        index = createIndex(row, HeadingIoRead, process);
        emit dataChanged(index, index);
        index = createIndex(row, HeadingIoWrite, process);
        emit dataChanged(index, index);
    }

}
ProcessModel::Units ProcessModel::ioUnits() const
{
    return (Units) d->mIoUnits;
}
void ProcessModel::setIoInformation( ProcessModel::IoInformation ioInformation )
{
    d->mIoInformation = ioInformation;
}
ProcessModel::IoInformation ProcessModel::ioInformation() const
{
    return d->mIoInformation;
}
QString ProcessModel::formatMemoryInfo(qlonglong amountInKB, Units units, bool returnEmptyIfValueIsZero) const
{
    //We cache the result of i18n for speed reasons.  We call this function
    //hundreds of times, every second or so
    if(returnEmptyIfValueIsZero && amountInKB == 0)
        return QString();
    static QString percentageString = i18n("%1%", QString::fromLatin1("%1"));
    if (units == UnitsPercentage) {
        if(d->mMemTotal == 0)
            return ""; //memory total not determined yet.  Shouldn't happen, but don't crash if it does
        float percentage = amountInKB*100.0/d->mMemTotal;
        if(percentage < 0.1) percentage = 0.1;
        return percentageString.arg(percentage, 0, 'f', 1);
    } else
        return formatByteSize(amountInKB, units);
}

QString ProcessModel::hostName() const {
    return d->mHostName;
}
QStringList ProcessModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    types << "text/csv";
    types << "text/html";
    return types;
}
QMimeData *ProcessModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QString textCsv;
    QString textCsvHeaders;
    QString textPlain;
    QString textPlainHeaders;
    QString textHtml;
    QString textHtmlHeaders;
    QString display;
    int firstColumn = -1;
    bool firstrow = true;
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            if(firstColumn == -1)
                firstColumn = index.column();
            else if(firstColumn != index.column())
                continue;
            else {
                textCsv += '\n';
                textPlain += '\n';
                textHtml += "</tr><tr>";
                firstrow = false;
            }
            for(int i = 0; i < d->mHeadings.size(); i++) {
                if(firstrow) {
                    QString heading = d->mHeadings[i];
                    textHtmlHeaders += "<th>" + heading + "</th>";
                    if(i) {
                        textCsvHeaders += ',';
                        textPlainHeaders += ", ";
                    }
                    textPlainHeaders += heading;
                    heading.replace('"', "\"\"");
                    textCsvHeaders += '"' + heading + '"';
                }
                QModelIndex index2 = createIndex(index.row(), i, reinterpret_cast< KSysGuard::Process * > (index.internalPointer()));
                QString display = data(index2, PlainValueRole).toString();
                if(i) {
                    textCsv += ',';
                    textPlain += ", ";
                }
                textHtml += "<td>" + Qt::escape(display) + "</td>";
                textPlain += display;
                display.replace('"',"\"\"");
                textCsv += '"' + display + '"';
            }
        }
    }
    textHtml = "<html><table><tr>" + textHtmlHeaders + "</tr><tr>" + textHtml + "</tr></table>";
    textCsv = textCsvHeaders + '\n' + textCsv;
    textPlain = textPlainHeaders + '\n' + textPlain;

    mimeData->setText(textPlain);
    mimeData->setHtml(textHtml);
    mimeData->setData("text/csv", textCsv.toUtf8());
    return mimeData;

}
Qt::ItemFlags ProcessModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags; //Would this ever happen?

    KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
    if(process->status == KSysGuard::Process::Ended)
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
    else
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

}

bool ProcessModel::isShowCommandLineOptions() const
{
    return d->mShowCommandLineOptions;
}
void ProcessModel::setShowCommandLineOptions(bool showCommandLineOptions)
{
    d->mShowCommandLineOptions = showCommandLineOptions;
}
bool ProcessModel::isShowingTooltips() const
{
    return d->mShowingTooltips;
}
void ProcessModel::setShowingTooltips(bool showTooltips)
{
    d->mShowingTooltips = showTooltips;
}
bool ProcessModel::isNormalizedCPUUsage() const
{
    return d->mNormalizeCPUUsage;
}
void ProcessModel::setNormalizedCPUUsage(bool normalizeCPUUsage)
{
    d->mNormalizeCPUUsage = normalizeCPUUsage;
}

void ProcessModelPrivate::timerEvent( QTimerEvent * event )
{
    Q_UNUSED(event);
    foreach (qlonglong pid, mPidsToUpdate) {
        KSysGuard::Process *process = mProcesses->getProcess(pid);
        if (process && !process->timeKillWasSent.isNull() && process->timeKillWasSent.elapsed() < MILLISECONDS_TO_SHOW_RED_FOR_KILLED_PROCESS) {
            int row;
            if(mSimple)
                row = process->index;
            else
                row = process->parent->children.indexOf(process);

            QModelIndex index1 = q->createIndex(row, 0, process);
            QModelIndex index2 = q->createIndex(row, mHeadings.count()-1, process);
            emit q->dataChanged(index1, index2);
        } else {
            mPidsToUpdate.removeAll(pid);
        }
    }

    if (mPidsToUpdate.isEmpty()) {
        mHaveTimer = false;
        killTimer(mTimerId);
        mTimerId = -1;
    }
}

