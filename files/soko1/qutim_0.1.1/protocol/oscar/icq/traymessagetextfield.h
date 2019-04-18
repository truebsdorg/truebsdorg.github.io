/*
    trayMessageTextField

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


#ifndef TRAYMESSAGETEXTFIELD_H_
#define TRAYMESSAGETEXTFIELD_H_

#include <QtGui>

class trayMessageTextField : public QTextBrowser
{
	Q_OBJECT
public:
	trayMessageTextField(QWidget *parent = 0);
	~trayMessageTextField();
	void setData(const QString &, const QString &);
	void setSystemData(const QString &);
	
signals:
	void closeWindow();
	void startChat();

protected:
	void mousePressEvent ( QMouseEvent * event );
};

#endif /*TRAYMESSAGETEXTFIELD_H_*/
