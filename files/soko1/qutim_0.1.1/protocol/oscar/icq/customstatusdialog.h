/*
    customStatusDialog

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


#ifndef CUSTOMSTATUSDIALOG_H
#define CUSTOMSTATUSDIALOG_H

#include <QtGui>
#include "ui_customstatusdialog.h"

class customStatusDialog : public QDialog
{
    Q_OBJECT

public:
    customStatusDialog(const QString &,QWidget *parent = 0);
    ~customStatusDialog();
    void setStatuses(int, const QList<QString> &);
    int status;
    QString statusCaption;
    QString statusMessage;
    void setCaption(const QString &t){ui.captionEdit->setText(t);}
    void setMessage(const QString &t){ui.awayEdit->setPlainText(t);}
private slots:
	void on_iconList_currentItemChanged ( QListWidgetItem * current, QListWidgetItem * previous );
    void on_chooseButton_clicked();
    void on_awayEdit_textChanged();
private:
    Ui::customStatusDialogClass ui;
    QList<QListWidgetItem *> itemList;
    QString getToolTip(int) const;
    int statusIndex;
    QString mineUin;
};

#endif // CUSTOMSTATUSDIALOG_H
