/*
    tabChatWindow

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


#ifndef TABCHATWINDOW_H
#define TABCHATWINDOW_H

#include <QtGui>
#include "ui_tabchatwindow.h"
#include "chatwindow.h"
#include "icqtabwidget.h"



class tabChatWindow : public QWidget
{
    Q_OBJECT

public:
    tabChatWindow(QObject *,QWidget *parent = 0);
    ~tabChatWindow();
    void addChatWindow(chatWindow *);
    bool deleteChatWindows;
    void detachChildren();
    void updateStatusIcon(chatWindow *, const QIcon &);
    void setMessageTab(chatWindow *);
    void showWindow(chatWindow *);
    int count() { return tabWidget->count();}
protected:
	void closeEvent(QCloseEvent *);
	void keyPressEvent ( QKeyEvent * event);
	
private slots:
	void tabChanged(int);
    void changeTabToNext(int);
private:
    Ui::tabChatWindowClass ui;
    QObject *realP;
    icqTabWidget *tabWidget;
    QHash<int,QIcon> oldStatusIcons;
};

#endif // TABCHATWINDOW_H
