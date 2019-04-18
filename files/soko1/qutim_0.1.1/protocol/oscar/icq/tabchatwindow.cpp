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

#include "statusiconsclass.h"
#include "tabchatwindow.h"

tabChatWindow::tabChatWindow(QObject *p, QWidget *parent)
    : QWidget(parent), realP(p)
{
	ui.setupUi(this);
//	ui.tabWidget->removeTab(0);
	deleteChatWindows = true;
	tabWidget = new icqTabWidget(this);
	ui.gridLayout->addWidget(tabWidget, 0, 0, 1, 1);
	connect(tabWidget, SIGNAL(currentChanged(int)),
			this ,SLOT(tabChanged(int)));
//	ui.tabWidget->setFrameShadow(QFrame::Plain);
}

tabChatWindow::~tabChatWindow()
{

	
}

void tabChatWindow::addChatWindow(chatWindow *w)
{
	if ( isHidden() )
		show();
	connect(w, SIGNAL(nextTab(int)), this, SLOT(changeTabToNext(int)));
	int index = tabWidget->addTab(w, w->windowIcon(), w->contactName);
	tabWidget->setCurrentIndex(index);
	tabWidget->setTabToolTip(index, w->contactName);

	if ( tabWidget->count() == 1)
	{
		setWindowTitle(tabWidget->tabText(0));
		setWindowIcon(tabWidget->tabIcon(0));
	}
	w->focusEditMessage();
	
}

void tabChatWindow::closeEvent(QCloseEvent * /* event */)
{
	if ( deleteChatWindows )
	{
		for(int i = tabWidget->count(); i >= 0 ; i--)
		{

			delete tabWidget->widget(i);
			tabWidget->removeTab(i);
		}
	}
	tabWidget->clear();
}

void tabChatWindow::tabChanged(int i)
{
	
	if ( oldStatusIcons.contains(i) )
	{
		QColor f;
		tabWidget->messageTab(f, oldStatusIcons.value(i), i);
		
		oldStatusIcons.remove(i);
	}
	setWindowTitle(tabWidget->tabText(i));
	setWindowIcon(tabWidget->tabIcon(i));
	if ( chatWindow *w = ((chatWindow*)tabWidget->widget(i)))
		w->focusEditMessage();
	
//	if ( tabWidget->count() == 1)
//	{
//		tabWidget->hideTabBar();
//	}
}

void tabChatWindow::detachChildren()
{
	for(; tabWidget->count();)
		{
			if (tabWidget->widget(0)->parent())
			{
				chatWindow *ch = (chatWindow*)tabWidget->widget(0);
				ch->setParent(0);
			}
		}
}

void tabChatWindow::updateStatusIcon(chatWindow *w, const QIcon &icon)
{
	int index = tabWidget->indexOf(w);
	if ( oldStatusIcons.contains(index) )
	{
		oldStatusIcons.remove(index);
		oldStatusIcons.insert(index,icon);
	}
	else
		tabWidget->setTabIcon(index, icon);
	if ( tabWidget->currentIndex() == index )
		setWindowIcon(icon);
}

void tabChatWindow::setMessageTab(chatWindow *w)
{
	int index = tabWidget->indexOf(w);
	if ( tabWidget->currentIndex() != index && !oldStatusIcons.contains(index))
	{
		oldStatusIcons.insert(index, tabWidget->tabIcon(index));
		tabWidget->messageTab(QColor(255,0,0), statusIconClass::getInstance()->getContentIcon(), index);
		
	}
}

void tabChatWindow::showWindow(chatWindow *w)
{
	if ( isMinimized() )
		showNormal();
	tabWidget->setCurrentIndex(tabWidget->indexOf(w));
	w->activateWindow();
	activateWindow();
}

void tabChatWindow::keyPressEvent(QKeyEvent *event)
{

	
	if ( event->key() == Qt::Key_Tab && event->modifiers() == Qt::CTRL)
		qDebug()<<"tab";
}

void tabChatWindow::changeTabToNext(int index)
{
	   if (((tabWidget->currentIndex() + 1) == tabWidget->count()) && (index > 0))
	   {
	      tabWidget->setCurrentIndex(0);
	   } else {
	      if (((tabWidget->currentIndex()) == 0) && (index < 0))
	      {
	         tabWidget->setCurrentIndex(tabWidget->count() - 1);
	      } else {
	         tabWidget->setCurrentIndex(tabWidget->currentIndex() + index);
	      }
	   }
}
