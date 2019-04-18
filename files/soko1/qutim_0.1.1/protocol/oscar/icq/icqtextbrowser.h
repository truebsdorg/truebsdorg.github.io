/*
    icqTextBrowser

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


#ifndef ICQTEXTBROWSER_H_
#define ICQTEXTBROWSER_H_

#include <QtGui>

class icqTextBrowser : public QTextBrowser
{
	Q_OBJECT
public:
	icqTextBrowser(QWidget *parent = 0);
	~icqTextBrowser();
	void addSmile(const QString &, const QString &);
	void clearMovieList();
	void insertImage(const QString &);
private slots:
	void animate();
signals:
	void focusWindow();

protected:
	void focusInEvent ( QFocusEvent * event );
private:
	QHash<QString, QMovie *> urls;
};

#endif /*ICQTEXTBROWSER_H_*/
