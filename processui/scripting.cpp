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
#include <QTextStream>
#include <QWebView>
#include <QWebFrame>

#include "processes.h"
#include "ksysguardprocesslist.h"
#include <KAction>
#include <KDialog>
#include <KMessageBox>
#include <KDesktopFile>
#include <KStandardDirs>
#include <KStandardAction>
#include <QVBoxLayout>

class ScriptingHtmlDialog : public KDialog {
    public:
        ScriptingHtmlDialog(QWidget *parent) : KDialog(parent) {
            setButtons( KDialog::Close );
            setButtonGuiItem( KDialog::Close, KStandardGuiItem::close() );
            setDefaultButton( KDialog::Close );
            setEscapeButton( KDialog::Close );
            setMainWidget(&m_webView);
            (void)minimumSizeHint(); //Force the dialog to be laid out now
            layout()->setContentsMargins(0,0,0,0);
            m_webView.settings()->setOfflineStoragePath(QString());
            m_webView.settings()->setOfflineWebApplicationCachePath(QString());
            m_webView.settings()->setObjectCacheCapacities(0,0,0);
            m_webView.settings()->setAttribute(QWebSettings::PluginsEnabled, false);
            m_webView.settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
            m_webView.page()->setNetworkAccessManager(NULL); //Disable talking to remote servers
            m_webView.page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
            m_webView.page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
        }
        QWebView *webView() {
            return &m_webView;
        }
    protected:
        QWebView m_webView;
};

ProcessObject::ProcessObject(ProcessModel *model, int pid) {
    mModel = model;
    mPid = pid;
}

bool ProcessObject::fileExists(const QString &filename)
{
    QFileInfo fileInfo(filename);
    return fileInfo.exists();
}
QString ProcessObject::readFile(const QString &filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return QString();
    QTextStream stream(&file);
    QString contents = stream.readAll();
    file.close();
    return contents;
}

Scripting::Scripting(KSysGuardProcessList * parent) : QWidget(parent), mProcessList(parent) {
    mScriptingHtmlDialog = NULL;
    loadContextMenu();
}
void Scripting::runScript(const QString &path, const QString &name) {
    //Record the script name and path for use in the script helper functions
    mScriptPath = path;
    mScriptName = name;

    QUrl fileName = QUrl::fromLocalFile(path + "index.html");
    if(!mScriptingHtmlDialog) {
        mScriptingHtmlDialog = new ScriptingHtmlDialog(this);
        connect(mScriptingHtmlDialog, SIGNAL(closeClicked()), SLOT(stopAllScripts()));

        KAction *refreshAction = new KAction("refresh", mScriptingHtmlDialog);
        refreshAction->setShortcut(QKeySequence::Refresh);
        connect(refreshAction, SIGNAL(triggered()), SLOT(refreshScript()));
        mScriptingHtmlDialog->addAction(refreshAction);

        KAction *zoomInAction = KStandardAction::zoomIn(this, SLOT(zoomIn()), mScriptingHtmlDialog);
        mScriptingHtmlDialog->addAction(zoomInAction);

        KAction *zoomOutAction = KStandardAction::zoomOut(this, SLOT(zoomOut()), mScriptingHtmlDialog);
        mScriptingHtmlDialog->addAction(zoomOutAction);
    }

    //Make the process information available to the script
    mScriptingHtmlDialog->webView()->load(fileName);
    mScriptingHtmlDialog->show();
    connect(mScriptingHtmlDialog->webView()->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(setupJavascriptObjects()));
    setupJavascriptObjects();

//    connect(mProcessList, SIGNAL(updated()), SLOT(refreshScript()));
}
void Scripting::zoomIn() {
    QWebView *webView = mScriptingHtmlDialog->webView();
    webView->setZoomFactor( webView->zoomFactor() * 1.1 );
}
void Scripting::zoomOut() {
    QWebView *webView = mScriptingHtmlDialog->webView();
    if(webView->zoomFactor() > 0.1) //Prevent it getting too small
        webView->setZoomFactor( webView->zoomFactor() / 1.1 );
}

void Scripting::refreshScript() {
    //Call any refresh function, if it exists
    mProcessList->processModel()->update(0, KSysGuard::Processes::XMemory);
    if(mScriptingHtmlDialog && mScriptingHtmlDialog->webView() && mScriptingHtmlDialog->webView()->page() && mScriptingHtmlDialog->webView()->page()->mainFrame()) {
        mScriptingHtmlDialog->webView()->page()->mainFrame()->evaluateJavaScript("refresh();");
    }
}
void Scripting::setupJavascriptObjects() {
    mProcessList->processModel()->update(0, KSysGuard::Processes::XMemory);
    mProcessObject = new ProcessObject(mProcessList->processModel(), mPid);
    mScriptingHtmlDialog->webView()->page()->mainFrame()->addToJavaScriptWindowObject("process", mProcessObject, QScriptEngine::ScriptOwnership);
}
void Scripting::stopAllScripts()
{
    if (mScriptingHtmlDialog)
        mScriptingHtmlDialog->deleteLater();
    mScriptingHtmlDialog = NULL;
    mProcessObject = NULL;
    mScriptPath.clear();
    mScriptName.clear();
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
            action->setProperty("scriptPath", QString(scriptPath + QLatin1Char('/')));
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
    mPid = selectedProcesses[0]->pid;

    runScript(path, action->text());
}
