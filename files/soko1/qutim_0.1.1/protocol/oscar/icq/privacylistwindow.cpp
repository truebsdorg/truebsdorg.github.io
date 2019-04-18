/*
    privacyListWindow

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


#include "privacylistwindow.h"

privacyListWindow::privacyListWindow(const QString &uin, QWidget *parent)
    : QWidget(parent), accountUin(uin)
{
	ui.setupUi(this);
	setWindowTitle(tr("Privacy lists"));
	setWindowIcon(QIcon(":/icons/crystal_project/privacylist.png"));
	move(desktopCenter());
	ui.visibleTreeWidget->setColumnWidth(2,22);
	ui.visibleTreeWidget->setColumnWidth(3,22);
	ui.visibleTreeWidget->setColumnWidth(1,200);
	
	ui.invisibleTreeWidget->setColumnWidth(2,22);
	ui.invisibleTreeWidget->setColumnWidth(3,22);
	ui.invisibleTreeWidget->setColumnWidth(1,200);
	
	ui.ignoreTreeWidget->setColumnWidth(2,22);
	ui.ignoreTreeWidget->setColumnWidth(3,22);
	ui.ignoreTreeWidget->setColumnWidth(1,200);
	createLists();
}

privacyListWindow::~privacyListWindow()
{

}

void privacyListWindow::rellocateDialogToCenter(QWidget *widget)
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

QPoint privacyListWindow::desktopCenter()
{
	QDesktopWidget desktop;
	return QPoint(desktop.width() / 2 - size().width() / 2, desktop.height() / 2 - size().height() / 2);
}

void privacyListWindow::createLists()
{
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+accountUin, "contacts");
	ui.visibleTreeWidget->clear();
	
	QStringList visibleList = contacts.value("list/visible").toStringList();
	
	foreach(QString uin, visibleList)
	{
		QTreeWidgetItem *buddy = new QTreeWidgetItem(ui.visibleTreeWidget);
		buddy->setText(0,uin);
		buddy->setText(1, contacts.value(uin + "/nickname", "").toString());
		buddy->setIcon(2,QIcon(":/icons/crystal_project/contactinfo.png"));
		buddy->setIcon(3,QIcon(":/icons/crystal_project/delete_user.png"));
	}
	
	ui.invisibleTreeWidget->clear();
	
	QStringList invisibleList = contacts.value("list/invisible").toStringList();
	
	foreach(QString uin, invisibleList)
	{
		QTreeWidgetItem *buddy = new QTreeWidgetItem(ui.invisibleTreeWidget);
		buddy->setText(0,uin);
		buddy->setText(1, contacts.value(uin + "/nickname", "").toString());
		buddy->setIcon(2,QIcon(":/icons/crystal_project/contactinfo.png"));
		buddy->setIcon(3,QIcon(":/icons/crystal_project/delete_user.png"));
	}
	
	ui.ignoreTreeWidget->clear();
	
	QStringList ignoreList = contacts.value("list/ignore").toStringList();
	
	foreach(QString uin, ignoreList)
	{
		QTreeWidgetItem *buddy = new QTreeWidgetItem(ui.ignoreTreeWidget);
		buddy->setText(0,uin);
		buddy->setText(1, contacts.value(uin + "/nickname", "").toString());
		buddy->setIcon(2,QIcon(":/icons/crystal_project/contactinfo.png"));
		buddy->setIcon(3,QIcon(":/icons/crystal_project/delete_user.png"));
	}
	
}

void privacyListWindow::setOnline(bool online)
{

	ui.ignoreTreeWidget->setColumnHidden(2, !online);
	ui.visibleTreeWidget->setColumnHidden(2, !online);
	ui.invisibleTreeWidget->setColumnHidden(2, !online);
	
	ui.ignoreTreeWidget->setColumnHidden(3, !online);
	ui.visibleTreeWidget->setColumnHidden(3, !online);
	ui.invisibleTreeWidget->setColumnHidden(3, !online);
}

void privacyListWindow::on_visibleTreeWidget_itemClicked(QTreeWidgetItem * item, int column)
{
	if ( column == 2)
		emit openInfo(item->text(0), item->text(1), "", "");
	if ( column == 3)
	{
		emit deleteFromPrivacyList(item->text(0), 0);
		delete item;
	}
}

void privacyListWindow::on_invisibleTreeWidget_itemClicked(QTreeWidgetItem * item, int column)
{
	if ( column == 2)
	{
			emit openInfo(item->text(0), item->text(1), "", "");

	}
	if ( column == 3)
	{
		emit deleteFromPrivacyList(item->text(0), 1);
		delete item;
	}
}

void privacyListWindow::on_ignoreTreeWidget_itemClicked(QTreeWidgetItem * item, int column)
{
	if ( column == 2)
			emit openInfo(item->text(0), item->text(1), "", "");
	if ( column == 3)
	{
		emit deleteFromPrivacyList(item->text(0), 2);
		delete item;
	}
}
