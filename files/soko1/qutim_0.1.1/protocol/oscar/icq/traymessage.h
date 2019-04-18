/*
    trayMessage

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


#ifndef TRAYMESSAGE_H
#define TRAYMESSAGE_H

#include <QtGui/QWidget>
#include "quticqglobals.h"
#include "ui_traymessage.h"

class trayMessage : public QWidget
{
    Q_OBJECT

public:
    trayMessage(const QString&, int, int, int, TrayPosition, int, int,
		bool f = true, QWidget *parent = 0);
    ~trayMessage();
    bool firstTrayWindow;
    bool showTray;

    void showTrayMessage();
    void setData(const QString &, const QString &, const QString &, const QString &);
    void setSystemData(const QString &, const QString &);
    void setTheme(const QString &, const QString&, const QString&, const QString&, const QString&);
private slots:
	void slideVerticallyUp();
	void slideVerticallyDown();
	void slideHorizontallyRight();
	void slideHorizontallyLeft();
	void startChatSlot();

signals:
	void startChat(const QString &);

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::trayMessageClass ui;
    TrayPosition position;
    int style;
    int time;
    int positionInStack;
    int moveToPointX;
    int moveToPointY;
    int fromX;
    int fromY;
    void slide();
    bool userMessage;
    QString contactUin;
    
    QString themePath;
    QString themeHeader;
    QString themeHeaderCSS;
    QString themeContent;
    QString themeContentCSS;
};

#endif // TRAYMESSAGE_H
