/*
    icqTabBar 
    icqTabWidget

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


#include "icqtabwidget.h"

icqTabBar::icqTabBar(QWidget *parent)
: QTabBar(parent)
{
	
}

icqTabBar::~icqTabBar()
{
	
}


void icqTabBar::mouseDoubleClickEvent(QMouseEvent *e)
{
	
	if ( e->button() == Qt::LeftButton )
	{
		emit closeTab(tabAt(e->pos()));
	}
}


icqTabWidget::icqTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
	customTabBar = new icqTabBar(this);
	setTabBar((QTabBar*)customTabBar);
	connect(tabBar(), SIGNAL(closeTab(int)), SLOT(tabClosingEmit(int)));
	closeButton  = new QToolButton(this);
	connect(closeButton, SIGNAL(clicked()), SLOT(on_closeButton_clicked()));
	closeButton->setIcon(QIcon(":/icons/crystal_project/deletetab.png"));
	closeButton->resize(16,16);
	closeButton->setAutoRaise(true);
	setCornerWidget(closeButton);
}

icqTabWidget::~icqTabWidget()
{

}

int icqTabWidget::addTab ( QWidget * page, const QIcon & icon, const QString & label )
{
//	if ( count() > 0 )
//	{
//		tabBar()->show();
//	}
//	else
//		tabBar()->hide();
	return QTabWidget::addTab(page, icon, label);
}

void icqTabWidget::messageTab(const QColor &c, const QIcon &i, int index)
{
	setTabIcon(index, i);
	tabBar()->setTabTextColor(index, c);
}

void icqTabWidget::on_closeButton_clicked()
{
	widget(currentIndex())->close();
//	if ( count() <= 2 )
//		tabBar()->hide();
}


void icqTabWidget::tabClosingEmit(int index)
{
	widget(index)->close();
//	if ( count() <= 2 )
//			tabBar()->hide();
}

void icqTabWidget::tabRemoved(int /*index*/)
{
	if ( !count() )
		((QWidget*)parent())->close();
}

