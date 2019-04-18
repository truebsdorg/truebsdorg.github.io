/*
    historyView

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


#ifndef HISTORYVIEW_H
#define HISTORYVIEW_H

#include <QtGui>
#include "ui_historyview.h"
#include "historyxml.h"

class historyView : public QWidget
{
    Q_OBJECT

public:
    historyView(const QString &, historyXML *, QWidget *parent = 0);
    ~historyView();

	void rellocateDialogToCenter(QWidget *widget);
    void initializeHistory();
    void initializeServiceHistory();
    
private slots:
	void on_dateList_currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous);
	void on_bSearch_clicked();
private:
    Ui::historyViewClass ui;
    historyXML *historyObject;
    QString uin;
    QPoint desktopCenter();
    bool serviceHistory;
};

#endif // HISTORYVIEW_H
