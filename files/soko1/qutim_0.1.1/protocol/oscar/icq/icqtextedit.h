/*
    icqTextEdit

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


#ifndef ICQTEXTEDIT_H
#define ICQTEXTEDIT_H

//#include <QtGui>
#include <QTextEdit>

class icqTextEdit : public QTextEdit
{
	Q_OBJECT
public:
	icqTextEdit(QWidget *parent = 0) : QTextEdit(parent), onEnter(false) {};
	~icqTextEdit() {};
	bool onEnter;
	
signals:
	void sendMessage();
	void focusWindow();
	void closeWindow();

protected:
	void keyPressEvent ( QKeyEvent * );
	void focusInEvent ( QFocusEvent * event );
};

#endif // ICQTEXTEDIT_H
