/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2009 John Tapsell <john.tapsell@kde.org>

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

#ifndef KSYSGUARDSCRIPTING_H
#define KSYSGUARDSCRIPTING_H

#include <QList>
#include <QString>
#include <QWidget>
#include <processcore/processes.h>
#include "ProcessModel.h"
#include "../config-ksysguard.h"

class QAction;
class ScriptingHtmlDialog; //Defined in scripting.cpp file
class KSysGuardProcessList;
class ProcessObject;

class Scripting : public QWidget {
  Q_OBJECT
  public:
    /** Create a scripting object */
    Scripting(KSysGuardProcessList *parent);
    /** Run the script in the given path */
    void runScript(const QString &path, const QString &name);
    /** Read all the script .desktop files and create an action for each one */
    void loadContextMenu();
    /** List of context menu actions that are created by loadContextMenu() */
    QList<QAction *> actions() { return mActions; }
    /** Create a ScriptingHtmlDialog, if one does not already exist, and display the given html */
    void displayHtml(const QString &html);

  public Q_SLOTS:
    /** Stop all scripts and delete the script engine */
    void stopAllScripts();
  private Q_SLOTS:
    /** Run the script associated with the QAction that called this slot */
    void runScriptSlot();
#if HAVE_QTWEBKITWIDGETS
    void setupJavascriptObjects();
    void refreshScript();
    void zoomIn();
    void zoomOut();
#endif
  private:
    /** This is created on the fly as needed, and deleted when no longer used */
    ScriptingHtmlDialog *mScriptingHtmlDialog;
    /** The parent process list to script for */
    KSysGuardProcessList * const mProcessList;
    /** List of context menu actions that are created by loadContextMenu() */
    QList<QAction *> mActions;
    QString mScriptPath;
    QString mScriptName;
    ProcessObject *mProcessObject;

    qlonglong mPid;
};

#define PROPERTY(Type,Name) Type Name() const { KSysGuard::Process *process = mModel->getProcess(mPid); if(process) return process->Name(); else return Type();}

class ProcessObject : public QObject {
    Q_OBJECT
    public:
       Q_PROPERTY(qlonglong pid READ pid WRITE setPid)                 /* Add functionality to 'set' the pid to change which process to read from */
       Q_PROPERTY(qlonglong ppid READ parentPid)                       /* Map 'ppid' to 'parentPid' to give it a nicer scripting name */
       Q_PROPERTY(QString name READ name)                              /* Defined below to return the first word of the name */
       Q_PROPERTY(QString fullname READ fullname)                      /* Defined below to return 'name' */
       Q_PROPERTY(qlonglong rss READ vmRSS)                            /* Map 'rss' to 'vmRSS' just to give it a nicer scripting name */
       Q_PROPERTY(qlonglong urss READ vmURSS)                          /* Map 'urss' to 'vmURSS' just to give it a nicer scripting name */
       Q_PROPERTY(int numThreads READ numThreads)                      PROPERTY(int, numThreads)
       Q_PROPERTY(qlonglong fsgid READ fsgid)                          PROPERTY(qlonglong, fsgid)
       Q_PROPERTY(qlonglong parentPid READ parentPid)                  PROPERTY(qlonglong, parentPid)
       Q_PROPERTY(QString login READ login)                            PROPERTY(QString, login)
       Q_PROPERTY(qlonglong uid READ uid)                              PROPERTY(qlonglong, uid)
       Q_PROPERTY(qlonglong euid READ euid)                            PROPERTY(qlonglong, euid)
       Q_PROPERTY(qlonglong suid READ suid)                            PROPERTY(qlonglong, suid)
       Q_PROPERTY(qlonglong fsuid READ fsuid)                          PROPERTY(qlonglong, fsuid)
       Q_PROPERTY(qlonglong gid READ gid)                              PROPERTY(qlonglong, gid)
       Q_PROPERTY(qlonglong egid READ egid)                            PROPERTY(qlonglong, egid)
       Q_PROPERTY(qlonglong sgid READ sgid)                            PROPERTY(qlonglong, sgid)
       Q_PROPERTY(qlonglong tracerpid READ tracerpid)                  PROPERTY(qlonglong, tracerpid)
       Q_PROPERTY(QByteArray tty READ tty)                             PROPERTY(QByteArray, tty)
       Q_PROPERTY(qlonglong userTime READ userTime)                    PROPERTY(qlonglong, userTime)
       Q_PROPERTY(qlonglong sysTime READ sysTime)                      PROPERTY(qlonglong, sysTime)
       Q_PROPERTY(int userUsage READ userUsage)                        PROPERTY(int, userUsage)
       Q_PROPERTY(int sysUsage READ sysUsage)                          PROPERTY(int, sysUsage)
       Q_PROPERTY(int totalUserUsage READ totalUserUsage)              PROPERTY(int, totalUserUsage)
       Q_PROPERTY(int totalSysUsage READ totalSysUsage)                PROPERTY(int, totalSysUsage)
       Q_PROPERTY(int numChildren READ numChildren)                    PROPERTY(int, numChildren)
       Q_PROPERTY(int niceLevel READ niceLevel)                        PROPERTY(int, niceLevel)
       Q_PROPERTY(int scheduler READ scheduler)                        PROPERTY(int, scheduler)
       Q_PROPERTY(int ioPriorityClass READ ioPriorityClass)            PROPERTY(int, ioPriorityClass)
       Q_PROPERTY(int ioniceLevel READ ioniceLevel)                    PROPERTY(int, ioniceLevel)
       Q_PROPERTY(qlonglong vmSize READ vmSize)                        PROPERTY(qlonglong, vmSize)
       Q_PROPERTY(qlonglong vmRSS READ vmRSS)                          PROPERTY(qlonglong, vmRSS)
       Q_PROPERTY(qlonglong vmURSS READ vmURSS)                        PROPERTY(qlonglong, vmURSS)
       Q_PROPERTY(qlonglong pixmapBytes READ pixmapBytes)              PROPERTY(qlonglong, pixmapBytes)
       Q_PROPERTY(bool hasManagedGuiWindow READ hasManagedGuiWindow)   PROPERTY(bool, hasManagedGuiWindow)
       Q_PROPERTY(QString command READ command)                        PROPERTY(QString, command)
       Q_PROPERTY(qlonglong status READ status)                        PROPERTY(qlonglong, status)
       Q_PROPERTY(qlonglong ioCharactersRead READ ioCharactersRead)    PROPERTY(qlonglong, ioCharactersRead)
       Q_PROPERTY(qlonglong ioCharactersWritten READ ioCharactersWritten)                  PROPERTY(qlonglong, ioCharactersWritten)
       Q_PROPERTY(qlonglong ioReadSyscalls READ ioReadSyscalls)                            PROPERTY(qlonglong, ioReadSyscalls)
       Q_PROPERTY(qlonglong ioWriteSyscalls READ ioWriteSyscalls)                          PROPERTY(qlonglong, ioWriteSyscalls)
       Q_PROPERTY(qlonglong ioCharactersActuallyRead READ ioCharactersActuallyRead)        PROPERTY(qlonglong, ioCharactersActuallyRead)
       Q_PROPERTY(qlonglong ioCharactersActuallyWritten READ ioCharactersActuallyWritten)  PROPERTY(qlonglong, ioCharactersActuallyWritten)
       Q_PROPERTY(qlonglong ioCharactersReadRate READ ioCharactersReadRate)                PROPERTY(qlonglong, ioCharactersReadRate)
       Q_PROPERTY(qlonglong ioCharactersWrittenRate READ ioCharactersWrittenRate)          PROPERTY(qlonglong, ioCharactersWrittenRate)
       Q_PROPERTY(qlonglong ioReadSyscallsRate READ ioReadSyscallsRate)                    PROPERTY(qlonglong, ioReadSyscallsRate)
       Q_PROPERTY(qlonglong ioWriteSyscallsRate READ ioWriteSyscallsRate)                  PROPERTY(qlonglong, ioWriteSyscallsRate)
       Q_PROPERTY(qlonglong ioCharactersActuallyReadRate READ ioCharactersActuallyReadRate)        PROPERTY(qlonglong, ioCharactersActuallyReadRate)
       Q_PROPERTY(qlonglong ioCharactersActuallyWrittenRate READ ioCharactersActuallyWrittenRate)  PROPERTY(qlonglong, ioCharactersActuallyWrittenRate)

        ProcessObject(ProcessModel * processModel, int pid);
        void update(KSysGuard::Process *process);

        int pid() const { return mPid; }
        void setPid(int pid) { mPid = pid; }
        QString name() const { KSysGuard::Process *process = mModel->getProcess(mPid); if(process) return process->name().section(' ', 0,0); else return QString(); }
        QString fullname() const { KSysGuard::Process *process = mModel->getProcess(mPid); if(process) return process->name(); else return QString(); }

        public Q_SLOTS:
        bool fileExists(const QString &filename);
        QString readFile(const QString &filename);
    private:
        int mPid;
        ProcessModel *mModel;
};

#endif
