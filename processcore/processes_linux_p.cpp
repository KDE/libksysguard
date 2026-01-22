/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process.h"

#include "memoryinfo_p.h"
#include "processes_local_p.h"

#include <klocalizedstring.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QQueue>
#include <QSet>
#include <QTextStream>
#include <QThreadPool>

// for sysconf
#include <unistd.h>
// for kill and setNice
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
// for ionice
#include <asm/unistd.h>
#include <sys/ptrace.h>
// for getsched
#include <sched.h>

#include <thread>

#define PROCESS_BUFFER_SIZE 1000

/* For ionice */
extern int sys_ioprio_set(int, int, int);
extern int sys_ioprio_get(int, int);

#define HAVE_IONICE
/* Check if this system has ionice */
#if !defined(SYS_ioprio_get) || !defined(SYS_ioprio_set)
/* All new kernels have SYS_ioprio_get and _set defined, but for the few that do not, here are the definitions */
#if defined(__i386__)
#define __NR_ioprio_set 289
#define __NR_ioprio_get 290
#elif defined(__ppc__) || defined(__powerpc__)
#define __NR_ioprio_set 273
#define __NR_ioprio_get 274
#elif defined(__x86_64__)
#define __NR_ioprio_set 251
#define __NR_ioprio_get 252
#elif defined(__ia64__)
#define __NR_ioprio_set 1274
#define __NR_ioprio_get 1275
#else
#ifdef __GNUC__
#warning "This architecture does not support IONICE.  Disabling ionice feature."
#endif
#undef HAVE_IONICE
#endif
/* Map these to SYS_ioprio_get */
#define SYS_ioprio_get __NR_ioprio_get
#define SYS_ioprio_set __NR_ioprio_set

#endif /* !SYS_ioprio_get */

/* Set up ionice functions */
#ifdef HAVE_IONICE
#include <linux/ioprio.h>

/* Expose the kernel calls to userspace via syscall
 * See man ioprio_set  and man ioprio_get   for information on these functions */
static int ioprio_set(int which, int who, int ioprio)
{
    return syscall(SYS_ioprio_set, which, who, ioprio);
}

static int ioprio_get(int which, int who)
{
    return syscall(SYS_ioprio_get, which, who);
}
#endif

// The rate at which the thread reading /proc/PID/smaps_rollup reads batches.
constexpr auto SmapsThreadDelay = std::chrono::milliseconds(100);
// The number of PIDs to read in a single batch.
constexpr auto SmapsThreadBatchSize = 5;
// The minimum time between updates of precise memory data.
constexpr auto PreciseUpdateInterval = std::chrono::seconds(30);

using namespace Qt::StringLiterals;

namespace KSysGuard
{
class ProcessesLocal::Private
{
public:
    Private()
    {
        mBuffer.fill('\0', PROCESS_BUFFER_SIZE);
        mProcDir = opendir("/proc");
    }
    ~Private();
    inline bool readProcStatus(const QString &dir, Process *process);
    inline bool readProcStat(const QString &dir, Process *process);
    inline bool readProcStatm(const QString &dir, Process *process);
    inline bool readProcCmdline(const QString &dir, Process *process);
    inline bool readProcCGroup(const QString &dir, Process *process);
    inline bool getNiceness(long pid, Process *process);
    inline bool getIOStatistics(const QString &dir, Process *process);

    static void smapsThreadFunction(std::stop_token stopToken, ProcessesLocal *processes);

    QFile mFile;
    QByteArray mBuffer;
    DIR *mProcDir;

    std::unique_ptr<std::jthread> smapsThread = nullptr;
    QQueue<long> smapsQueue;
    std::mutex smapsQueueMutex;
};

ProcessesLocal::Private::~Private()
{
    if (smapsThread) {
        smapsThread->request_stop();
        smapsThread->join();
    }
    closedir(mProcDir);
}

void ProcessesLocal::Private::smapsThreadFunction(std::stop_token stopToken, ProcessesLocal *processes)
{
    while (!stopToken.stop_requested()) {
        std::this_thread::sleep_for(SmapsThreadDelay);

        std::vector<long> pids;
        {
            std::lock_guard<std::mutex> lock(processes->d->smapsQueueMutex);
            while (!processes->d->smapsQueue.isEmpty() && pids.size() < SmapsThreadBatchSize) {
                pids.push_back(processes->d->smapsQueue.takeFirst());
            }
        }

        if (pids.empty()) {
            continue;
        }

        for (auto pid : pids) {
            QFile file{"/proc/"_L1 + QString::number(pid) + "/smaps_rollup"_L1};
            if (!file.open(QIODevice::ReadOnly)) {
                continue;
            }

            MemoryFields fields;

            auto buffer = QByteArray{1024, '\0'};
            while (file.readLine(buffer.data(), buffer.size()) > 0) {
                auto parts = buffer.split(':');
                if (parts.size() >= 2) {
                    if (parts.at(0) == "Rss") {
                        fields.rss += std::stoll(parts.at(1).toStdString());
                    } else if (parts.at(0) == "Pss") {
                        fields.pss += std::stoll(parts.at(1).toStdString());
                    } else if (parts.at(0).startsWith("Shared")) {
                        fields.shared += std::stoll(parts.at(1).toStdString());
                    } else if (parts.at(0).startsWith("Private")) {
                        fields.priv += std::stoll(parts.at(1).toStdString());
                    } else if (parts.at(0) == "Swap") {
                        fields.swap += std::stoll(parts.at(1).toStdString());
                    }
                }
            }

            file.close();

            fields.lastUpdate = std::chrono::steady_clock::now();

            Q_EMIT processes->processUpdated(pid, {{Process::MemoryPrecise, QVariant::fromValue(fields)}});
        }
    }
}

ProcessesLocal::ProcessesLocal()
    : d(new Private())
{
}

bool ProcessesLocal::Private::readProcStatus(const QString &dir, Process *process)
{
    mFile.setFileName(dir + QStringLiteral("status"));
    if (!mFile.open(QIODevice::ReadOnly))
        return false; /* process has terminated in the meantime */

    bool uidFound = false;
    bool gidFound = false;
    bool tracerFound = false;
    bool numThreadsFound = false;
    bool noNewPrivilegesFound = false;

    constexpr int maxFieldCount = 7;

    int size;
    int found = 0; // count how many fields we found
    while ((size = mFile.readLine(mBuffer.data(), mBuffer.size())) > 0) { //-1 indicates an error
        switch (mBuffer[0]) {
        case 'N':
            if ((unsigned int)size > sizeof("Name:") && qstrncmp(mBuffer, "Name:", sizeof("Name:") - 1) == 0) {
                if (process->command().isEmpty()) {
                    process->setName(QString::fromLocal8Bit(mBuffer + sizeof("Name:") - 1, size - sizeof("Name:") + 1).trimmed());
                }
                if (++found == maxFieldCount) {
                    goto finish;
                }
            } else if ((unsigned int)size > sizeof("NoNewPrivs:") && qstrncmp(mBuffer, "NoNewPrivs:", sizeof("NoNewPrivs:") - 1) == 0) {
                process->setNoNewPrivileges(atol(mBuffer + sizeof("NoNewPrivs:") - 1));
                noNewPrivilegesFound = true;
                if (++found == maxFieldCount) {
                    goto finish;
                }
            }
            break;
        case 'U':
            if ((unsigned int)size > sizeof("Uid:") && qstrncmp(mBuffer, "Uid:", sizeof("Uid:") - 1) == 0) {
                qlonglong uid;
                qlonglong euid;
                qlonglong suid;
                qlonglong fsuid;
                sscanf(mBuffer + sizeof("Uid:") - 1, "%lld %lld %lld %lld", &uid, &euid, &suid, &fsuid);
                process->setUid(uid);
                process->setEuid(euid);
                process->setSuid(suid);
                process->setFsuid(fsuid);
                uidFound = true;
                if (++found == maxFieldCount) {
                    goto finish;
                }
            }
            break;
        case 'G':
            if ((unsigned int)size > sizeof("Gid:") && qstrncmp(mBuffer, "Gid:", sizeof("Gid:") - 1) == 0) {
                qlonglong gid, egid, sgid, fsgid;
                sscanf(mBuffer + sizeof("Gid:") - 1, "%lld %lld %lld %lld", &gid, &egid, &sgid, &fsgid);
                process->setGid(gid);
                process->setEgid(egid);
                process->setSgid(sgid);
                process->setFsgid(fsgid);
                gidFound = true;
                if (++found == maxFieldCount) {
                    goto finish;
                }
            }
            break;
        case 'T':
            if ((unsigned int)size > sizeof("TracerPid:") && qstrncmp(mBuffer, "TracerPid:", sizeof("TracerPid:") - 1) == 0) {
                process->setTracerpid(atol(mBuffer + sizeof("TracerPid:") - 1));
                if (process->tracerpid() == 0) {
                    process->setTracerpid(-1);
                }
                tracerFound = true;
                if (++found == maxFieldCount) {
                    goto finish;
                }
            } else if ((unsigned int)size > sizeof("Threads:") && qstrncmp(mBuffer, "Threads:", sizeof("Threads:") - 1) == 0) {
                process->setNumThreads(atol(mBuffer + sizeof("Threads:") - 1));
                numThreadsFound = true;
                if (++found == maxFieldCount) {
                    goto finish;
                }
            }
            break;
        case 'V':
            if (mBuffer.startsWith("VmSwap:")) {
                process->memoryInfo()->imprecise.swap = atol(mBuffer + sizeof("VmSwap:") - 1);

                if (++found == maxFieldCount) {
                    goto finish;
                }
            }
        default:
            break;
        }
    }

finish:
    if (!uidFound) {
        process->setUid(0);
        process->setEuid(0);
        process->setSuid(0);
        process->setFsuid(0);
    }
    if (!gidFound) {
        process->setGid(0);
        process->setEgid(0);
        process->setSgid(0);
        process->setFsgid(0);
    }
    if (!tracerFound) {
        process->setTracerpid(-1);
    }
    if (!numThreadsFound) {
        process->setNumThreads(0);
    }
    if (!noNewPrivilegesFound) {
        process->setNoNewPrivileges(0);
    }

    mFile.close();
    return true;
}

bool ProcessesLocal::Private::readProcCGroup(const QString &dir, Process *process)
{
    mFile.setFileName(dir + QStringLiteral("cgroup"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }

    while (mFile.readLine(mBuffer.data(), mBuffer.size()) > 0) { //-1 indicates an error
        if (mBuffer[0] == '0' && mBuffer[1] == ':' && mBuffer[2] == ':') {
            process->setCGroup(QString::fromLocal8Bit(&mBuffer[3]).trimmed());
            break;
        }
    }
    mFile.close();
    return true;
}

long ProcessesLocal::getParentPid(long pid)
{
    if (pid <= 0) {
        return -1;
    }
    d->mFile.setFileName(QStringLiteral("/proc/") + QString::number(pid) + QStringLiteral("/stat"));
    if (!d->mFile.open(QIODevice::ReadOnly)) {
        return -1; /* process has terminated in the meantime */
    }

    int size; // amount of data read in
    if ((size = d->mFile.readLine(d->mBuffer.data(), d->mBuffer.size())) <= 0) { //-1 indicates nothing read
        d->mFile.close();
        return -1;
    }

    d->mFile.close();
    char *word = d->mBuffer.data();
    // The command name is the second parameter, and this ends with a closing bracket.  So find the last
    // closing bracket and start from there
    word = strrchr(word, ')');
    if (!word) {
        return -1;
    }
    word++; // Nove to the space after the last ")"
    int current_word = 1;

    while (true) {
        if (word[0] == ' ') {
            if (++current_word == 3) {
                break;
            }
        } else if (word[0] == 0) {
            return -1; // end of data - serious problem
        }
        word++;
    }
    long ppid = atol(++word);
    if (ppid == 0) {
        return -1;
    }
    return ppid;
}

bool ProcessesLocal::Private::readProcStat(const QString &dir, Process *ps)
{
    QString filename = dir + QStringLiteral("stat");
    // As an optimization, if the last file read in was stat, then we already have this info in memory
    if (mFile.fileName() != filename) {
        mFile.setFileName(filename);
        if (!mFile.open(QIODevice::ReadOnly)) {
            return false; /* process has terminated in the meantime */
        }
        if (mFile.readLine(mBuffer.data(), mBuffer.size()) <= 0) { //-1 indicates nothing read
            mFile.close();
            return false;
        }
        mFile.close();
    }

    char *word = mBuffer.data();
    // The command name is the second parameter, and this ends with a closing bracket.  So find the last
    // closing bracket and start from there
    word = strrchr(word, ')');
    if (!word) {
        return false;
    }
    word++; // Nove to the space after the last ")"
    int current_word = 1; // We've skipped the process ID and now at the end of the command name
    char status = '\0';
    while (current_word < 23) {
        if (word[0] == ' ') {
            ++current_word;
            switch (current_word) {
            case 2: // status
                status = word[1]; // Look at the first letter of the status.
                // We analyze this after the while loop
                break;
            case 6: // ttyNo
            {
                int ttyNo = atoi(word + 1);
                int major = ttyNo >> 8;
                int minor = ttyNo & 0xff;
                switch (major) {
                case 136:
                    ps->setTty(QByteArray("pts/") + QByteArray::number(minor));
                    break;
                case 5:
                    ps->setTty(QByteArray("tty"));
                    break;
                case 4:
                    if (minor < 64) {
                        ps->setTty(QByteArray("tty") + QByteArray::number(minor));
                    } else {
                        ps->setTty(QByteArray("ttyS") + QByteArray::number(minor - 64));
                    }
                    break;
                default:
                    ps->setTty(QByteArray());
                }
            } break;
            case 13: // userTime
                ps->setUserTime(atoll(word + 1));
                break;
            case 14: // sysTime
                ps->setSysTime(atoll(word + 1));
                break;
            case 18: // niceLevel
                ps->setNiceLevel(atoi(word + 1)); /*Or should we use getPriority instead? */
                break;
            case 21: // startTime
                ps->setStartTime(atoll(word + 1));
                break;
            case 22: // vmSize
                // Does nothing, read from statm below.
                break;
            case 23: // vmRSS
                // Does nothing, read from statm below.
                break;
            default:
                break;
            }
        } else if (word[0] == 0) {
            return false; // end of data - serious problem
        }
        word++;
    }

    switch (status) {
    case 'R':
        ps->setStatus(Process::Running);
        break;
    case 'S':
        ps->setStatus(Process::Sleeping);
        break;
    case 'D':
        ps->setStatus(Process::DiskSleep);
        break;
    case 'Z':
        ps->setStatus(Process::Zombie);
        break;
    case 'T':
        ps->setStatus(Process::Stopped);
        break;
    case 'W':
        ps->setStatus(Process::Paging);
        break;
    default:
        ps->setStatus(Process::OtherStatus);
        break;
    }
    return true;
}

bool ProcessesLocal::Private::readProcStatm(const QString &dir, Process *process)
{
#ifdef _SC_PAGESIZE
    mFile.setFileName(dir + QStringLiteral("statm"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }

    qint64 size = 0;
    if (size = mFile.readLine(mBuffer.data(), mBuffer.size()); size <= 0) { //-1 indicates nothing read
        mFile.close();
        return false;
    }
    mFile.close();

    MemoryFields fields;
    auto parts = mBuffer.mid(0, size).split(' ');

    if (parts.size() != 7) {
        qWarning() << "Reading statm from" << dir + u"statm"_s << "failed, expected 7 fields, got" << parts.size();
        return false;
    }

    static const auto pageSize = sysconf(_SC_PAGESIZE) / 1024;

    // From kernel documentation:
    // Table 1-3: Contents of the statm files (as of 2.6.8-rc3)
    // ..............................................................................
    // Field    Content
    // size     total program size (pages)		(same as VmSize in status)
    // resident size of memory portions (pages)	(same as VmRSS in status)
    // shared   number of pages that are shared	(i.e. backed by a file, same
    //                                              as RssFile+RssShmem in status)
    // trs      number of pages that are 'code'	(not including libs; broken,
    //                                              includes data segment)
    // lrs      number of pages of library		(always 0 on 2.6)
    // drs      number of pages of data/stack	(including libs; broken,
    //                                              includes library text)
    // dt       number of dirty pages			(always 0 on 2.6)
    fields.rss = atol(parts.at(1)) * pageSize;
    fields.shared = atol(parts.at(2)) * pageSize;
    fields.priv = fields.rss - fields.shared;
    fields.swap = process->swap();
    fields.lastUpdate = std::chrono::steady_clock::now();

    process->memoryInfo()->imprecise = fields;
    process->memoryInfo()->vmSize = atol(parts.at(0)) * pageSize;
    process->addChange(Process::Memory);

    return true;
#else
    return false;
#endif
}

bool ProcessesLocal::Private::readProcCmdline(const QString &dir, Process *process)
{
    if (!process->command().isNull()) {
        return true; // only parse the cmdline once.  This function takes up 25% of the CPU time :-/
    }
    mFile.setFileName(dir + QStringLiteral("cmdline"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }

    QTextStream in(&mFile);
    process->setCommand(in.readAll());

    // cmdline separates parameters with the NULL character
    if (!process->command().isEmpty()) {
        // extract non-truncated name from cmdline
        int zeroIndex = process->command().indexOf(QLatin1Char('\0'));
        int processNameStart = process->command().lastIndexOf(QLatin1Char('/'), zeroIndex);
        if (processNameStart == -1) {
            processNameStart = 0;
        } else {
            processNameStart++;
        }
        QString nameFromCmdLine = process->command().mid(processNameStart, zeroIndex - processNameStart);
        if (nameFromCmdLine.startsWith(process->name())) {
            process->setName(nameFromCmdLine);
        }

        process->command().replace(QLatin1Char('\0'), QLatin1Char(' '));
    }

    mFile.close();
    return true;
}

bool ProcessesLocal::Private::getNiceness(long pid, Process *process)
{
    int sched = sched_getscheduler(pid);
    switch (sched) {
    case (SCHED_OTHER):
        process->setScheduler(KSysGuard::Process::Other);
        break;
    case (SCHED_RR):
        process->setScheduler(KSysGuard::Process::RoundRobin);
        break;
    case (SCHED_FIFO):
        process->setScheduler(KSysGuard::Process::Fifo);
        break;
#ifdef SCHED_IDLE
    case (SCHED_IDLE):
        process->setScheduler(KSysGuard::Process::SchedulerIdle);
        break;
#endif
#ifdef SCHED_BATCH
    case (SCHED_BATCH):
        process->setScheduler(KSysGuard::Process::Batch);
        break;
#endif
    default:
        process->setScheduler(KSysGuard::Process::Other);
    }
    if (sched == SCHED_FIFO || sched == SCHED_RR) {
        struct sched_param param;
        if (sched_getparam(pid, &param) == 0) {
            process->setNiceLevel(param.sched_priority);
        } else {
            process->setNiceLevel(0); // Error getting scheduler parameters.
        }
    }

#ifdef HAVE_IONICE
    int ioprio = ioprio_get(IOPRIO_WHO_PROCESS, pid); /* Returns from 0 to 7 for the iopriority, and -1 if there's an error */
    if (ioprio == -1) {
        process->setIoniceLevel(-1);
        process->setIoPriorityClass(KSysGuard::Process::None);
        return false; /* Error. Just give up. */
    }
    process->setIoniceLevel(ioprio & 0xff); /* Bottom few bits are the priority */
    process->setIoPriorityClass((KSysGuard::Process::IoPriorityClass)(ioprio >> IOPRIO_CLASS_SHIFT)); /* Top few bits are the class */
    return true;
#else
    return false; /* Do nothing, if we do not support this architecture */
#endif
}

bool ProcessesLocal::Private::getIOStatistics(const QString &dir, Process *process)
{
    mFile.setFileName(dir + QStringLiteral("io"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }

    while (mFile.readLine(mBuffer.data(), mBuffer.size()) > 0) {
        auto parts = mBuffer.split(':');
        if (parts.size() >= 2) {
            auto name = parts.at(0).trimmed();
            auto number = atoll(parts.at(1));
            if (name == "rchar") {
                process->setIoCharactersRead(number);
            } else if (name == "wchar") {
                process->setIoCharactersWritten(number);
            } else if (name == "syscr") {
                process->setIoReadSyscalls(number);
            } else if (name == "syscw") {
                process->setIoWriteSyscalls(number);
            } else if (name == "read_bytes") {
                process->setIoCharactersActuallyRead(number);
            } else if (name == "write_bytes") {
                process->setIoCharactersActuallyWritten(number);
            }
        }
    }

    mFile.close();
    return true;
}

bool ProcessesLocal::updateProcessInfo(long pid, Process *process)
{
    bool success = true;
    const QString dir = QLatin1String("/proc/") + QString::number(pid) + QLatin1Char('/');

    if (mUpdateFlags.testFlag(Processes::Smaps)) {
        if (!d->smapsThread) {
            d->smapsThread = std::make_unique<std::jthread>(Private::smapsThreadFunction, this);
        }

        if (std::chrono::steady_clock::now() - process->memoryInfo()->precise.lastUpdate >= PreciseUpdateInterval) {
            std::lock_guard<std::mutex> lock(d->smapsQueueMutex);
            if (!d->smapsQueue.contains(pid)) {
                d->smapsQueue.append(pid);
            }
        }
    }

    if (!d->readProcStat(dir, process)) {
        success = false;
    }
    if (!d->readProcStatus(dir, process)) {
        success = false;
    }
    if (!d->readProcStatm(dir, process)) {
        success = false;
    }
    if (!d->readProcCmdline(dir, process)) {
        success = false;
    }
    if (!d->readProcCGroup(dir, process)) {
        success = false;
    }
    if (!d->getNiceness(pid, process)) {
        success = false;
    }
    if (mUpdateFlags.testFlag(Processes::IOStatistics) && !d->getIOStatistics(dir, process)) {
        success = false;
    }

    return success;
}

QSet<long> ProcessesLocal::getAllPids()
{
    QSet<long> pids;
    if (d->mProcDir == nullptr) {
        return pids; // There's not much we can do without /proc
    }
    struct dirent *entry;
    rewinddir(d->mProcDir);
    while ((entry = readdir(d->mProcDir))) {
        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
            pids.insert(atol(entry->d_name));
        }
    }
    return pids;
}

Processes::Error ProcessesLocal::sendSignal(long pid, int sig)
{
    errno = 0;
    if (pid <= 0) {
        return Processes::InvalidPid;
    }
    if (kill((pid_t)pid, sig)) {
        switch (errno) {
        case ESRCH:
            return Processes::ProcessDoesNotExistOrZombie;
        case EINVAL:
            return Processes::InvalidParameter;
        case EPERM:
            return Processes::InsufficientPermissions;
        }
        // Kill failed
        return Processes::Unknown;
    }
    return Processes::NoError;
}

Processes::Error ProcessesLocal::setNiceness(long pid, int priority)
{
    errno = 0;
    if (pid <= 0) {
        return Processes::InvalidPid;
    }
    auto error = [] {
        switch (errno) {
        case ESRCH:
        case ENOENT:
            return Processes::ProcessDoesNotExistOrZombie;
        case EINVAL:
            return Processes::InvalidParameter;
        case EACCES:
        case EPERM:
            return Processes::InsufficientPermissions;
        default:
            return Processes::Unknown;
        }
    };
    auto threadList{QDir(QString::fromLatin1("/proc/%1/task").arg(pid)).entryList(QDir::NoDotAndDotDot | QDir::Dirs)};
    if (threadList.isEmpty()) {
        return error();
    }
    for (auto entry : threadList) {
        int threadId = entry.toInt();
        if (!threadId) {
            return Processes::InvalidParameter;
        }
        if (setpriority(PRIO_PROCESS, threadId, priority)) {
            return error();
        }
    }
    return Processes::NoError;
}

int ProcessesLocal::getNiceness(long pid)
{
    if (pid <= 0) {
        return 0;
    }
    errno = 0;
    const int nice = getpriority(PRIO_PROCESS, pid);
    if (errno != 0) {
        return 0;
    }
    return nice;
}

Processes::Error ProcessesLocal::setScheduler(long pid, int priorityClass, int priority)
{
    errno = 0;
    if (priorityClass == KSysGuard::Process::Other || priorityClass == KSysGuard::Process::Batch || priorityClass == KSysGuard::Process::SchedulerIdle) {
        priority = 0;
    }
    if (pid <= 0) {
        return Processes::InvalidPid;
    }
    struct sched_param params;
    params.sched_priority = priority;
    int policy;
    switch (priorityClass) {
    case (KSysGuard::Process::Other):
        policy = SCHED_OTHER;
        break;
    case (KSysGuard::Process::RoundRobin):
        policy = SCHED_RR;
        break;
    case (KSysGuard::Process::Fifo):
        policy = SCHED_FIFO;
        break;
#ifdef SCHED_IDLE
    case (KSysGuard::Process::SchedulerIdle):
        policy = SCHED_IDLE;
        break;
#endif
#ifdef SCHED_BATCH
    case (KSysGuard::Process::Batch):
        policy = SCHED_BATCH;
        break;
#endif
    default:
        return Processes::NotSupported;
    }

    auto error = [] {
        switch (errno) {
        case ESRCH:
        case ENOENT:
            return Processes::ProcessDoesNotExistOrZombie;
        case EINVAL:
            return Processes::InvalidParameter;
        case EPERM:
            return Processes::InsufficientPermissions;
        default:
            return Processes::Unknown;
        }
    };
    auto threadList{QDir(QString::fromLatin1("/proc/%1/task").arg(pid)).entryList(QDir::NoDotAndDotDot | QDir::Dirs)};
    if (threadList.isEmpty()) {
        return error();
    }
    for (auto entry : threadList) {
        int threadId = entry.toInt();
        if (!threadId) {
            return Processes::InvalidParameter;
        }
        if (sched_setscheduler(threadId, policy, &params) != 0) {
            return error();
        }
    }
    return Processes::NoError;
}

int ProcessesLocal::getSchedulerClass(long pid)
{
    if (pid <= 0) {
        return 0;
    }
    const int policy = sched_getscheduler(pid);
    switch (policy) {
    case (SCHED_OTHER): return KSysGuard::Process::Other;
    case (SCHED_RR): return KSysGuard::Process::RoundRobin;
    case (SCHED_FIFO): return KSysGuard::Process::Fifo;
#ifdef SCHED_IDLE
    case (SCHED_IDLE): return KSysGuard::Process::SchedulerIdle;
#endif
    case (SCHED_BATCH): return KSysGuard::Process::Batch;
    default: return 0;
    }
}

Processes::Error ProcessesLocal::setIoNiceness(long pid, int priorityClass, int priority)
{
    errno = 0;
    if (pid <= 0) {
        return Processes::InvalidPid;
    }
#ifdef HAVE_IONICE
    if (ioprio_set(IOPRIO_WHO_PROCESS, pid, IOPRIO_PRIO_VALUE(priorityClass, priority)) == -1) {
        // set io niceness failed
        switch (errno) {
        case ESRCH:
            return Processes::ProcessDoesNotExistOrZombie;
            break;
        case EINVAL:
            return Processes::InvalidParameter;
        case EPERM:
            return Processes::InsufficientPermissions;
        }
        return Processes::Unknown;
    }
    return Processes::NoError;
#else
    return Processes::NotSupported;
#endif
}

int ProcessesLocal::getIoNiceness(long pid)
{
    if (pid <= 0) {
        return 0;
    }
#ifdef HAVE_IONICE
    const int mask = ioprio_get(IOPRIO_WHO_PROCESS, pid);
    return IOPRIO_PRIO_DATA(mask);
#else
    return 0;
#endif
}

int ProcessesLocal::getIoPriorityClass(long pid)
{
    if (pid <= 0) {
        return 0;
    }
#ifdef HAVE_IONICE
    const int mask = ioprio_get(IOPRIO_WHO_PROCESS, pid);
    return IOPRIO_PRIO_CLASS(mask);
#else
    return 0;
#endif
}

bool ProcessesLocal::supportsIoNiceness()
{
#ifdef HAVE_IONICE
    return true;
#else
    return false;
#endif
}

long long ProcessesLocal::totalPhysicalMemory()
{
    // Try to get the memory via sysconf.  Note the cast to long long to try to avoid a long overflow
    // Should we use sysconf(_SC_PAGESIZE)  or getpagesize()  ?
#ifdef _SC_PHYS_PAGES
    return ((long long)sysconf(_SC_PHYS_PAGES)) * (sysconf(_SC_PAGESIZE) / 1024);
#else
    // This is backup code in case this is not defined.  It should never fail on a linux system.

    d->mFile.setFileName("/proc/meminfo");
    if (!d->mFile.open(QIODevice::ReadOnly)) {
        return 0;
    }

    int size;
    while ((size = d->mFile.readLine(d->mBuffer.data(), d->mBuffer.size())) > 0) { //-1 indicates an error
        switch (d->mBuffer[0]) {
        case 'M':
            if ((unsigned int)size > sizeof("MemTotal:") && qstrncmp(d->mBuffer, "MemTotal:", sizeof("MemTotal:") - 1) == 0) {
                d->mFile.close();
                return atoll(d->mBuffer + sizeof("MemTotal:") - 1);
            }
        }
    }
    return 0; // Not found.  Probably will never happen
#endif
}

long long ProcessesLocal::totalSwapMemory()
{
    d->mFile.setFileName("/proc/meminfo");
    if (!d->mFile.open(QIODevice::ReadOnly)) {
        return 0;
    }

    int size;
    while ((size = d->mFile.readLine(d->mBuffer.data(), d->mBuffer.size())) > 0) {
        if (d->mBuffer.startsWith("SwapTotal:")) {
            d->mFile.close();
            return std::atoll(d->mBuffer.mid(sizeof("SwapTotal:") + 1));
        }
    }

    return 0;
}

ProcessesLocal::~ProcessesLocal()
{
    delete d;
}
}
