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

#include <QtGui>
#include "traymessage.h"

trayMessage::trayMessage(const QString &u, int width, int height, int t,
	TrayPosition pos, int st, int psInStack, bool f, QWidget *parent)
:QWidget(parent)
{
	ui.setupUi(this);
	ui.nickLabel->installEventFilter(this);

	connect(ui.textBrowser, SIGNAL(closeWindow()), this, SLOT(close()));
	connect(ui.textBrowser, SIGNAL(startChat()), this, SLOT(startChatSlot()));

	time = t;
	contactUin = u;
	position = pos;
	style = st;
	positionInStack = psInStack;
	firstTrayWindow = false;
	userMessage = f;
	setFixedSize(QSize(width, height));
	setAttribute(Qt::WA_QuitOnClose, false);
	setAttribute(Qt::WA_DeleteOnClose, true);
	QTimer::singleShot((time + 1) * 1000, this, SLOT(close()));
#if defined(Q_OS_MAC)
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#else
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
#endif
}

trayMessage::~trayMessage()
{

}

void trayMessage::showTrayMessage()
{
	QDesktopWidget desktopSize;
	QRect geometry = desktopSize.availableGeometry();
	moveToPointX = 0;
	moveToPointY = 0;

	switch(position)
	{
	case TopLeft:
		moveToPointX = geometry.left();
		moveToPointY = geometry.top() + (positionInStack - 1) * height();
		break;
	case TopRight:
		moveToPointX = geometry.right() - width();
		moveToPointY = geometry.top() + (positionInStack - 1) * height();
		break;
	case BottomLeft:
		moveToPointX = geometry.left();
		moveToPointY = geometry.bottom() - positionInStack * height();
		break;
//	case BottomRight:
//		moveToPointX = geometry.width() - width();
//		moveToPointY = geometry.height() - positionInStack * height();
//		break;
	default:
		moveToPointX = geometry.right() - width();
		moveToPointY = geometry.bottom() - positionInStack * height();
	}
	slide();
}

void trayMessage::slide()
{
	fromX = 0;
	fromY = 0;
	switch (style)
	{
	case 0:
		move(moveToPointX, moveToPointY);
		show();
		break;
	case 1:
		if (position == TopLeft || position == TopRight)
		{
			fromX = moveToPointX;
			fromY = moveToPointY - height();
			move(fromX, fromY);
			show();
			slideVerticallyDown();
		} else if (position == BottomLeft || position == BottomRight)
		{
			fromX = moveToPointX;
			fromY = moveToPointY + height();
			move(fromX, fromY);
			show();
			slideVerticallyUp();
		}
		break;
	case 2:
		if (position == TopLeft || position == BottomLeft)
		{
			fromX = moveToPointX - width();
			fromY = moveToPointY;
			move(fromX, fromY);
			show();
			slideHorizontallyRight();
		} else if (position == TopRight || position == BottomRight)
		{
			fromX = moveToPointX + width();
			fromY = moveToPointY;
			move(fromX, fromY);
			show();
			slideHorizontallyLeft();
		}
		break;
	default:
		move(moveToPointX, moveToPointY);
		show();
	}

}

void trayMessage::slideVerticallyUp()
{
	fromY -= 5;

	if (fromY >= moveToPointY)
		{
			move(moveToPointX, fromY);
			QTimer::singleShot(10, this, SLOT(slideVerticallyUp()));
		}
}

void trayMessage::slideVerticallyDown()
{
	fromY += 5;
	if (fromY <= moveToPointY)
	{
		move(moveToPointX, fromY);
		QTimer::singleShot(10, this, SLOT(slideVerticallyDown()));
	}
}

void trayMessage::slideHorizontallyRight()
{
	fromX += 5;
	if (fromX <= moveToPointX)
	{
		move(fromX, moveToPointY);
		QTimer::singleShot(10, this, SLOT(slideHorizontallyRight()));
	}
}

void trayMessage::slideHorizontallyLeft()
{
	fromX -= 5;
	if (fromX >= moveToPointX)
	{
		move(fromX, moveToPointY);
		QTimer::singleShot(10, this, SLOT(slideHorizontallyLeft()));
	}
}

void trayMessage::setData(const QString &mineNick, const QString &from,
		const QString &picturePath, const QString &msg)
{

	if ( themeHeader.isEmpty() )
	{

		ui.nickLabel->setStyleSheet("background-image : url(:/icons/tray_pics/header.png);\n\ncolor : white; \n");
		ui.nickLabel->setText(tr("<b>%1</b>").arg(Qt::escape(mineNick)));
	} else {
		themeHeader.replace("%path%", themePath);
		themeHeader.replace("%minenick%", (Qt::escape(mineNick)));
		themeHeader.replace("%fromnick%", Qt::escape(from));
		ui.nickLabel->setStyleSheet(themeHeaderCSS);
		ui.nickLabel->setText(themeHeader);
	}
//	ui.nickLabel->setStyleSheet("{ background-color: yellow }");

	if(themeContent.isEmpty())
	{

		QString message = tr("<b>%1</b><br /> <img align='left'  height='64' width='64' src='%avatar%' hspace='4' vspace='4'> %2").arg(Qt::escape(from)).arg(Qt::escape(msg));
		ui.textBrowser->setData(message, picturePath);
	} else {
		themeContent.replace("%path%", Qt::escape(themePath));
		themeContent.replace("%message%", Qt::escape(msg));
		themeContent.replace("%fromnick%", Qt::escape(from));
		ui.textBrowser->setStyleSheet(themeContentCSS);
		ui.textBrowser->setData(themeContent, picturePath);
	}

}

bool trayMessage::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent *mevent = (QMouseEvent *) event;
		if (mevent->button() == Qt::RightButton)
			close();
		else if (mevent->button() == Qt::LeftButton)
			startChatSlot();
	}
	return QWidget::eventFilter(obj,event);
}

void trayMessage::startChatSlot()
{
	if (userMessage)
		emit startChat(contactUin);
	close();
}

void trayMessage::setSystemData(const QString& nickName, const QString &msg)
{
	if ( themeHeader.isEmpty() )
	{
		ui.nickLabel->setStyleSheet("background-image : url(:/icons/tray_pics/header.png);\n\ncolor : white; \n");
		ui.nickLabel->setText(tr("<b>%1</b>").arg(Qt::escape(nickName)));
	} else {
		themeHeader.replace("%path%", Qt::escape(themePath));
		themeHeader.replace("%minenick%", Qt::escape(nickName));
		ui.nickLabel->setStyleSheet(themeHeaderCSS);
		ui.nickLabel->setText(themeHeader);
	}
	
	if ( themeContent.isEmpty())
	{
		QString message = tr("<b>%1</b>").arg(msg);
		ui.textBrowser->setSystemData(msg);
	} else {
		themeContent.replace("%path%", Qt::escape(themePath));
		themeContent.replace("%message%", msg);
		ui.textBrowser->setStyleSheet(themeContentCSS);
		ui.textBrowser->setSystemData(themeContent);
	}
	
}

void trayMessage::setTheme(const QString &header, const QString &headerCss, const QString &content, const QString &contentCss, const QString &path)
{
	themePath = path;
	themeHeader = header;
	themeHeaderCSS = headerCss;
	themeContent = content;
	themeContentCSS = contentCss;
	themeHeaderCSS.replace("%path%", Qt::escape(themePath));
	themeContentCSS.replace("%path%", Qt::escape(themePath));
}
