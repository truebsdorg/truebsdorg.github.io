/*
    statusDialog

    Copyright (c) 2008 by Rustam Chakin <qutim.develop@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/


#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

#include <QtGui>
#include "ui_statusdialog.h"

class statusDialog : public QDialog
{
    Q_OBJECT

public:
    statusDialog(QWidget *parent = 0);
    ~statusDialog();
    void setStatusMessage(const QString &);
    QString statusMessage;
    bool dontShow;
    
private slots:
	void closeAfter();
	void on_okButton_clicked();
	void on_statusEdit_textChanged();

private:
    Ui::statusDialogClass ui;
    QPoint desktopCenter();
    quint8 closeSec;
    QTimer closeTimer;
};

#endif // STATUSDIALOG_H
