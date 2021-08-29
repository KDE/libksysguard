/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LSOFWIDGET_H_
#define LSOFWIDGET_H_

#include <QObject>
#include <QProcess>
#include <QTreeWidget>

struct KLsofWidgetPrivate;

class Q_DECL_EXPORT KLsofWidget : public QTreeWidget
{
    Q_OBJECT
    Q_PROPERTY(qlonglong pid READ pid WRITE setPid)
public:
    KLsofWidget(QWidget *parent = nullptr);
    ~KLsofWidget() override;
    bool update();

private Q_SLOTS:
    /* For QProcess *process */
    // void error ( QProcess::ProcessError error );
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    // void readyReadStandardError ();
    // void readyReadStandardOutput ();
    // void started ();
    qlonglong pid() const;
    void setPid(qlonglong pid);

private:
    KLsofWidgetPrivate *const d;
};

/*  class LsofProcessInfo {
    public:
    pid_t tpid;
    int pidst;
    pid_t pid;
    pid_t ppid;
    pid_t pgrp;
    int uid;
    QString cmd;
    QString login;
  };
  class LsofFileInfo {
    QString file_descriptor;
    char access;
    int file_struct_share_count;
    char device_character_code;
    long major_minor;
    long file_struct_address;
    long file_flags;
    long inode;
    long link_count;
    char lock;
    long file_struct_node_id;
    long file_offset;
    QString protocol_name;
    QString stream_module;
    QString file_type;
    QString tcp_info;
  };
*/
#endif
