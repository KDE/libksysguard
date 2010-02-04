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

#include "scripting.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>
#include <QTextStream>
#include <QUiLoader>
#include <QWebView>

#include <QDebug>

#include "processes.h"
#include "ksysguardprocesslist.h"
#include <KAction>
#include <KDialog>
#include <KMessageBox>
#include <KDesktopFile>
#include <KStandardDirs>
#include <QVBoxLayout>

class ScriptingHtmlDialog : public KDialog {
    public:
        ScriptingHtmlDialog(QWidget *parent) : KDialog(parent) {
            setButtons( KDialog::Close );
            setButtonGuiItem( KDialog::Close, KStandardGuiItem::close() );
            setDefaultButton( KDialog::Close );
            setEscapeButton( KDialog::Close );
            showButtonSeparator( false );
            setMainWidget(&m_webView);
            (void)minimumSizeHint(); //Force the dialog to be laid out now
            layout()->setContentsMargins(0,0,0,0);
            m_webView.settings()->setOfflineStoragePath(QString());
            m_webView.settings()->setOfflineWebApplicationCachePath(QString());
            m_webView.settings()->setObjectCacheCapacities(0,0,0);
        }
        QWebView *webView() {
            return &m_webView;
        }
    protected:
        QWebView m_webView;
};

QScriptValue setHtml(QScriptContext *context, QScriptEngine *engine)
{
    Scripting *scriptingParent = static_cast<Scripting *>(qVariantValue<QObject*>(engine->property("scriptingParent")));
    if(context->argumentCount() != 1) {
        return context->throwError(QScriptContext::SyntaxError, i18np("Script error: There needs to be exactly one argument to setHtml(), but there was %1.",
                    "Script error: There needs to be exactly one argument to setHtml(), but there were %1.",
                    context->argumentCount()));
    }
    if(!context->argument(0).isString()) {
        return context->throwError(QScriptContext::TypeError, i18n("Script error: Argument to setHtml() was not a string"));
    }
    QString html = context->argument(0).toString();

    scriptingParent->displayHtml(html);
    return QScriptValue();
}

void Scripting::displayHtml(const QString &html) {

    if(!mScriptingHtmlDialog) {
        mScriptingHtmlDialog = new ScriptingHtmlDialog(this);
        connect(mScriptingHtmlDialog, SIGNAL(finished(int)), SLOT(deleteScriptingHtmlDialog()));
    }

    mScriptingHtmlDialog->webView()->setHtml(html, QUrl::fromLocalFile(mScriptPath + '/'));
    mScriptingHtmlDialog->show();
}

void Scripting::deleteScriptingHtmlDialog() {
    delete mScriptingHtmlDialog;
    mScriptingHtmlDialog = NULL;
}

QScriptValue fileExists(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);
    /* We do lots of checks on the file to see whether we should allow this to be read
     * Maybe this is a bit too paranoid and too restrictive.  Some restrictions
     * may be lifted */
    if(context->argumentCount() !=1)  {
        return context->throwError(QScriptContext::SyntaxError, i18np("Script error: There needs to be exactly one argument to fileExists(), but there was %1.",
                    "Script error: There needs to be exactly one argument to fileExists(), but there were %1.",
                    context->argumentCount()));
    }
    if(!context->argument(0).isString())
        return context->throwError(QScriptContext::TypeError, i18n("Script error: Argument to fileExists() was not a string"));
    QString filename = context->argument(0).toString();
    QFileInfo fileInfo(filename);
    return QScriptValue(engine, fileInfo.exists());

}
QScriptValue readFile(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);
    /* We do lots of checks on the file to see whether we should allow this to be read
     * Maybe this is a bit too paranoid and too restrictive.  Some restrictions
     * may be lifted */
    if(context->argumentCount() !=1)
        return context->throwError(QScriptContext::SyntaxError, i18np("Script error: There needs to be exactly one argument to readFile(), but there was %1.",
                    "Script error: There needs to be exactly one argument to readFile(), but there were %1.",
                    context->argumentCount()));
    if(!context->argument(0).isString())
        return context->throwError(QScriptContext::TypeError, i18n("Script error: Argument to readFile() was not a string"));
    QString filename = context->argument(0).toString();
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        return QScriptValue();
    }
    QTextStream stream(&file);
    QString contents = stream.readAll();
    file.close();

    return contents;
}
Scripting::Scripting(KSysGuardProcessList * parent) : QWidget(parent), mProcessList(parent) {
    mScriptingHtmlDialog = NULL;
    mScriptEngine = NULL;
    loadContextMenu();
}
void Scripting::runScript(KSysGuard::Process *process, const QString &path, const QString &name) {
    if(!mScriptEngine) {
        mScriptEngine = new QScriptEngine(this);
        mScriptEngine->setProperty("scriptingParent", qVariantFromValue(static_cast<QObject *>(this)));
    }
    //Record the script name and path for use in the script helper functions
    mScriptPath = path;
    mScriptName = name;

    //Add the various functions to the script
    mScriptEngine->globalObject().setProperty("readFile", mScriptEngine->newFunction( readFile ) );
    mScriptEngine->globalObject().setProperty("fileExists", mScriptEngine->newFunction( fileExists ) );
    mScriptEngine->globalObject().setProperty("setHtml", mScriptEngine->newFunction( setHtml ) );

    //Make the process information available to the script
    QScriptValue p = mScriptEngine->newObject();
    p.setProperty("pid", (int)process->pid);
    p.setProperty("ppid", (int)process->parent_pid);
    p.setProperty("name", process->name.section(' ', 0,0));
    p.setProperty("fullname", process->name);
    p.setProperty("command", process->command);
    mScriptEngine->globalObject().setProperty("process", p);

    //Load ui resources first
    QDir dir(path, "*.ui");
    QStringList uiFiles = dir.entryList();
    QUiLoader *loader = NULL;
    foreach(QString uiFileName, uiFiles) { //We do uiFileName.replace so can't make this a const reference
        if(!loader)
            loader = new QUiLoader(this);
        QFile uiFile(path + uiFileName);
        uiFile.open(QIODevice::ReadOnly);
        QWidget *ui = loader->load(&uiFile, this);
        uiFile.close();

        //Call the ui, e.g.,  form.ui as variable name form_ui
        QScriptValue scriptUi = mScriptEngine->newQObject(ui, QScriptEngine::ScriptOwnership);
        mScriptEngine->globalObject().setProperty(uiFileName.replace('.', '_'), scriptUi);
    }
    delete loader;

    /* load main javascript file */
    QString fileName = path + "main.js";
    QFile scriptFile(fileName);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        KMessageBox::sorry(this, i18n("Could not read script '%1'.  Error %2", fileName, scriptFile.error()));
        return;
    }
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    mScriptEngine->evaluate(contents, fileName);
    if(mScriptEngine->hasUncaughtException()) {
        KMessageBox::sorry(this, i18n("<qt>Script failed: %1: %2<br>%3",
                    mScriptEngine->uncaughtExceptionLineNumber(),
                    mScriptEngine->uncaughtException().toString(),
                    mScriptEngine->uncaughtExceptionBacktrace().join("<br>")));
    }
}

void Scripting::stopAllScripts()
{
    deleteScriptingHtmlDialog();
    delete mScriptEngine;
    mScriptEngine = NULL;
    mScriptPath = QString();
    mScriptName = QString();
}
void Scripting::loadContextMenu() {
    //Clear any existing actions
    qDeleteAll(mActions);
    mActions.clear();

    QStringList scripts = KGlobal::dirs()->findAllResources("data", "ksysguard/scripts/*/*.desktop", KStandardDirs::NoDuplicates);
    foreach(const QString &script, scripts) {
        KDesktopFile desktopFile(script);
        if(!desktopFile.name().isEmpty() && !desktopFile.noDisplay()) {
            KAction *action = new KAction(desktopFile.readName(), this);
            action->setToolTip(desktopFile.readComment());
            action->setIcon(QIcon(desktopFile.readIcon()));
            QString scriptPath = script;
            scriptPath.truncate(scriptPath.lastIndexOf('/'));
            action->setProperty("scriptPath", scriptPath + '/');
            connect(action, SIGNAL(triggered(bool)), SLOT(runScriptSlot()));
            mProcessList->addAction(action);
            mActions << action;
        }
    }
}

void Scripting::runScriptSlot() {
    KAction *action = static_cast<KAction*>(sender());
    //All the files for the script should be in the scriptPath
    QString path = action->property("scriptPath").toString();

    QList<KSysGuard::Process *> selectedProcesses = mProcessList->selectedProcesses();
    if(selectedProcesses.isEmpty())
        return;

    runScript(selectedProcesses[0], path, action->text());

}
