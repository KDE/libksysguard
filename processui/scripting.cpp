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
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDialog>
#include <QUrl>

#include "processes.h"
#include "ksysguardprocesslist.h"

#include <KDesktopFile>
#include <KStandardAction>
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDialogButtonBox>

#if HAVE_QTWEBKITWIDGETS
#include <QWebView>
#include <QWebFrame>
#endif

class ScriptingHtmlDialog : public QDialog {
    public:
        ScriptingHtmlDialog(QWidget *parent) : QDialog(parent) {

            QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
            buttonBox->setStandardButtons(QDialogButtonBox::Close);
            connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

#if HAVE_QTWEBKITWIDGETS
            QVBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(&m_webView);
            layout->addWidget(buttonBox);
            setLayout(layout);
            (void)minimumSizeHint(); //Force the dialog to be laid out now
            layout->setContentsMargins(0,0,0,0);
            m_webView.settings()->setOfflineStoragePath(QString());
            m_webView.settings()->setObjectCacheCapacities(0,0,0);
            m_webView.settings()->setAttribute(QWebSettings::PluginsEnabled, false);
            m_webView.settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
            m_webView.page()->setNetworkAccessManager(nullptr); //Disable talking to remote servers
            m_webView.page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
            m_webView.page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);

            // inject a style sheet that follows system colors, otherwise we might end up with black text on dark gray background
            const QString styleSheet = QStringLiteral(
                "body { background: %1; color: %2; }" \
                "a { color: %3; }" \
                "a:visited { color: %4; } "
            ).arg(palette().background().color().name(),
                  palette().text().color().name(),
                  palette().link().color().name(),
                  palette().linkVisited().color().name());

            // you can only provide a user style sheet url, so we turn it into a data url here
            const QUrl dataUrl(QStringLiteral("data:text/css;charset=utf-8;base64,") + QString::fromLatin1(styleSheet.toUtf8().toBase64()));

            m_webView.settings()->setUserStyleSheetUrl(dataUrl);

#endif
        }
#if HAVE_QTWEBKITWIDGETS
        QWebView *webView() {
            return &m_webView;
        }
    protected:
        QWebView m_webView;
#endif
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
    mScriptingHtmlDialog = nullptr;
    loadContextMenu();
}
void Scripting::runScript(const QString &path, const QString &name) {
    //Record the script name and path for use in the script helper functions
    mScriptPath = path;
    mScriptName = name;

#if HAVE_QTWEBKITWIDGETS
    QUrl fileName = QUrl::fromLocalFile(path + "index.html");
    if(!mScriptingHtmlDialog) {
        mScriptingHtmlDialog = new ScriptingHtmlDialog(this);
        connect(mScriptingHtmlDialog, &QDialog::rejected, this, &Scripting::stopAllScripts);

        QAction *refreshAction = new QAction(QStringLiteral("refresh"), mScriptingHtmlDialog);
        refreshAction->setShortcut(QKeySequence::Refresh);
        connect(refreshAction, &QAction::triggered, this, &Scripting::refreshScript);
        mScriptingHtmlDialog->addAction(refreshAction);

        QAction *zoomInAction = KStandardAction::zoomIn(this, SLOT(zoomIn()), mScriptingHtmlDialog);
        mScriptingHtmlDialog->addAction(zoomInAction);

        QAction *zoomOutAction = KStandardAction::zoomOut(this, SLOT(zoomOut()), mScriptingHtmlDialog);
        mScriptingHtmlDialog->addAction(zoomOutAction);
    }

    //Make the process information available to the script
    mScriptingHtmlDialog->webView()->load(fileName);
    mScriptingHtmlDialog->show();
    connect(mScriptingHtmlDialog->webView()->page()->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared, this, &Scripting::setupJavascriptObjects);
    setupJavascriptObjects();
#else
    QMessageBox::critical(this, i18n("QtWebKitWidgets not available"),
            i18n("KSysGuard library was compiled without QtWebKitWidgets, please contact your distribution."));
#endif
}
#if HAVE_QTWEBKITWIDGETS
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
        mScriptingHtmlDialog->webView()->page()->mainFrame()->evaluateJavaScript(QStringLiteral("refresh();"));
    }
}
void Scripting::setupJavascriptObjects() {
    mProcessList->processModel()->update(0, KSysGuard::Processes::XMemory);
    mProcessObject = new ProcessObject(mProcessList->processModel(), mPid);
    mScriptingHtmlDialog->webView()->page()->mainFrame()->addToJavaScriptWindowObject(QStringLiteral("process"), mProcessObject, QWebFrame::ScriptOwnership);
}
#endif
void Scripting::stopAllScripts()
{
    if (mScriptingHtmlDialog)
        mScriptingHtmlDialog->deleteLater();
    mScriptingHtmlDialog = nullptr;
    mProcessObject = nullptr;
    mScriptPath.clear();
    mScriptName.clear();
}
void Scripting::loadContextMenu() {
    //Clear any existing actions
    qDeleteAll(mActions);
    mActions.clear();

    QStringList scripts;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("ksysguard/scripts/"), QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString& dir, dirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.desktop"), QDir::NoFilter, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            scripts.append(it.next());
        }
    }

    foreach(const QString &script, scripts) {
        KDesktopFile desktopFile(script);
        if(!desktopFile.name().isEmpty() && !desktopFile.noDisplay()) {
            QAction *action = new QAction(desktopFile.readName(), this);
            action->setToolTip(desktopFile.readComment());
            action->setIcon(QIcon(desktopFile.readIcon()));
            QString scriptPath = script;
            scriptPath.truncate(scriptPath.lastIndexOf('/'));
            action->setProperty("scriptPath", QString(scriptPath + QLatin1Char('/')));
            connect(action, &QAction::triggered, this, &Scripting::runScriptSlot);
            mProcessList->addAction(action);
            mActions << action;
        }
    }
}

void Scripting::runScriptSlot() {
    QAction *action = static_cast<QAction*>(sender());
    //All the files for the script should be in the scriptPath
    QString path = action->property("scriptPath").toString();

    QList<KSysGuard::Process *> selectedProcesses = mProcessList->selectedProcesses();
    if(selectedProcesses.isEmpty())
        return;
    mPid = selectedProcesses[0]->pid();

    runScript(path, action->text());
}
