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


#include "traymessagetextfield.h"

trayMessageTextField::trayMessageTextField(QWidget *parent): QTextBrowser(parent)
{
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

trayMessageTextField::~trayMessageTextField()
{
	
}

void trayMessageTextField::mousePressEvent ( QMouseEvent * event )
{
	if ( event->button() == Qt::RightButton )
		emit closeWindow();
	else if (event->button() == Qt::LeftButton )
		emit startChat();
		
	QTextBrowser::mousePressEvent(event);
}

void trayMessageTextField::setData(const QString &msg, const QString &picturePath)
{
	QString picture;
	if ( QFile::exists(picturePath) )
		picture = picturePath;
	else
		picture = QString(":/icons/crystal_project/noavatar.png");
	
	QString message = msg;

	append(message.replace("%avatar%", picture));
	
	moveCursor(QTextCursor::Start);
	ensureCursorVisible();
}

void trayMessageTextField::setSystemData(const QString &msg)
{

	append(msg);
		
		moveCursor(QTextCursor::Start);
		ensureCursorVisible();
}
