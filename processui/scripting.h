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
#include <QScriptValue>
#include <QString>
#include <QWidget>
#include "processes.h"

class QScriptContext;
class QScriptEngine;
class QAction;
class ScriptingHtmlDialog; //Defined in scripting.cpp file
class KSysGuardProcessList;

class Scripting : public QWidget {
  Q_OBJECT
  public:
    /** Create a scripting object */
    Scripting(KSysGuardProcessList *parent);
    /** Run the script in the given path */
    void runScript(KSysGuard::Process *process, const QString &path, const QString &name);
    /** Read all the script .desktop files and create an action for each one */
    void loadContextMenu();
    /** List of context menu actions that are created by loadContextMenu() */
    QList<QAction *> actions() { return mActions; }
    /** Create a ScriptingHtmlDialog, if one does not already exist, and display the given html */
    void displayHtml(const QString &html);
    /** Stop all scripts and delete the script engine */
    void stopAllScripts();

  public Q_SLOTS:
    /** Delete scripting html dialog if it exists */
    void deleteScriptingHtmlDialog();
  private Q_SLOTS:
    /** Run the script associated with the QAction that called this slot */
    void runScriptSlot();
  private:
    /** This is created on the fly as needed, and deleted when no longer used */
    ScriptingHtmlDialog *mScriptingHtmlDialog;
    /** This is created on the fly as needed */
    QScriptEngine *mScriptEngine;
    /** The parent process list to script for */
    KSysGuardProcessList * const mProcessList;
    /** List of context menu actions that are created by loadContextMenu() */
    QList<QAction *> mActions;
    QString mScriptPath;
    QString mScriptName;


};

#endif
