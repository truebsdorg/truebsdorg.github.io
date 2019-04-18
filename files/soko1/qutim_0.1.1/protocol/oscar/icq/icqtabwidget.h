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



#ifndef ICQTABWIDGET_H
#define ICQTABWIDGET_H

#include <QtGui>
//#include <QStyleOptionTabWidgetFrame>

class icqTabBar : QTabBar
{
	Q_OBJECT
public:
	icqTabBar(QWidget *parent = 0);
	~icqTabBar();
protected:
	void mouseDoubleClickEvent ( QMouseEvent * event );
signals:
	void closeTab(int);
};

class icqTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    icqTabWidget(QWidget *parent = 0);
    ~icqTabWidget();
    int addTab ( QWidget * page, const QIcon & icon, const QString & label );
    void messageTab(const QColor &, const QIcon &, int );
    void hideTabBar(){tabBar()->hide();}
private slots:
	void on_closeButton_clicked();
	void tabClosingEmit(int);
	
protected:
	void tabRemoved( int ); 
	
private:
    QToolButton *closeButton;
    icqTabBar *customTabBar;
};

#endif // ICQTABWIDGET_H
