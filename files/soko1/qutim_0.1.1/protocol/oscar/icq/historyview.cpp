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


#include "historyview.h"

historyView::historyView(const QString &u, historyXML *history, QWidget *parent)
    : QWidget(parent), historyObject(history), uin(u)
{
	ui.setupUi(this);
	setWindowTitle(tr("%1 history").arg(uin));
	setWindowIcon(QIcon(":/icons/crystal_project/history.png"));
	move(desktopCenter());
	serviceHistory = false;
}

historyView::~historyView()
{

}

void historyView::rellocateDialogToCenter(QWidget *widget)
{
	QDesktopWidget desktop;
	// Get current screen num
	int curScreen = desktop.screenNumber(widget);
	// Get available geometry of the screen
	QRect screenGeom = desktop.availableGeometry(curScreen);
	// Let's calculate point to move dialog
	QPoint moveTo(screenGeom.left(), screenGeom.top());
	
	moveTo.setX(moveTo.x() + screenGeom.width() / 2);
	moveTo.setY(moveTo.y() + screenGeom.height() / 2);
	
	moveTo.setX(moveTo.x() - this->size().width() / 2);
	moveTo.setY(moveTo.y() - this->size().height() / 2);
	
	this->move(moveTo);
}

void historyView::initializeHistory()
{
	serviceHistory = false;
	historyObject->addHistoryToList(ui.dateList, uin,ui.lSearch->text());
	ui.dateList->setCurrentRow(ui.dateList->count() -1 );
}

QPoint historyView::desktopCenter()
{
	QDesktopWidget desktop;
	return QPoint(desktop.width() / 2 - size().width() / 2, desktop.height() / 2 - size().height() / 2);
}

void historyView::on_dateList_currentItemChanged(QListWidgetItem * current, QListWidgetItem * /*previous*/)
{
	if ( current)
	{
		QStringList dateString = current->text().split("-");
		QDate date(dateString.at(0).toUInt(), dateString.at(1).toUInt(), dateString.at(2).toUInt() );
		if (!serviceHistory)
			historyObject->addHistoryMessages(ui.historyChat, date, uin, ui.lSearch->text());
		else
			historyObject->addServiceHistoryMessages(ui.historyChat, date);
	}
}


void historyView::initializeServiceHistory()
{
	setWindowTitle(tr("Service history"));
	serviceHistory = true;
	historyObject->addServiceHistoryToList(ui.dateList,ui.lSearch->text());
	ui.dateList->setCurrentRow(ui.dateList->count() -1 );
}

void historyView::on_bSearch_clicked()
{
	ui.historyChat->clear();
	ui.dateList->clear();
	if(!serviceHistory)
		initializeHistory();
	else
		initializeServiceHistory();
}
