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

#include <QKeyEvent>

#include "icqtextedit.h"

//icqTextEdit::icqTextEdit(QWidget *parent)
//    : QTextEdit(parent)
//{
//	onEnter = false;
//
//}
//
//icqTextEdit::~icqTextEdit()
//{
//
//}

void icqTextEdit::keyPressEvent(QKeyEvent *e)
{
	if ( e->key() == Qt::Key_Escape )
		emit closeWindow();
	
	
	if ( onEnter )
	{
		if ( e->key() == Qt::Key_Return && e->modifiers() == Qt::ControlModifier ||
				e->key() == Qt::Key_Return && e->modifiers() == Qt::ShiftModifier	)
		{
			insertPlainText("\n");
			moveCursor(QTextCursor::End);
			ensureCursorVisible();	
			
		} else if ( e->key() == Qt::Key_Return )
			emit sendMessage();
		else 
			QTextEdit::keyPressEvent(e);
	} else 
		QTextEdit::keyPressEvent(e);
}

void icqTextEdit::focusInEvent ( QFocusEvent * event )
{
	
	emit focusWindow();
	QTextEdit::focusInEvent(event);
}
