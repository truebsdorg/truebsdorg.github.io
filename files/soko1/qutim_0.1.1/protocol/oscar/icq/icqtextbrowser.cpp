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


#include "icqtextbrowser.h"

icqTextBrowser::icqTextBrowser(QWidget *parent)
: QTextBrowser(parent)
{
	document()->addResource( QTextDocument::ImageResource, QUrl( "front.png" ), QPixmap( ":/icons/tray_pics/front.png" ) );
}

icqTextBrowser::~icqTextBrowser()
{
	clearMovieList();
}

void icqTextBrowser::focusInEvent(QFocusEvent *e)
{
	emit focusWindow();
	QTextBrowser::focusInEvent(e);
}

void icqTextBrowser::addSmile(const QString &url, const QString &path)
{
	if ( !urls.contains(url))
	{
		QMovie* movie = new QMovie(this);
		movie->setFileName(path);
		urls.insert(url, movie);
		connect(movie, SIGNAL(frameChanged(int)), this, SLOT(animate()));
		movie->setCacheMode(QMovie::CacheAll);
		movie->start();
	}
}

void icqTextBrowser::animate()
{
	if (QMovie* movie = qobject_cast<QMovie*>(sender()))
	{
		document()->addResource(QTextDocument::ImageResource,
			urls.key(movie), movie->currentPixmap());
		setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
	}
}
void icqTextBrowser::clearMovieList()
{
	foreach(QMovie *movie, urls)
		delete movie;
	urls.clear();
}

void icqTextBrowser::insertImage(const QString &imgData)
{
	QString tmpData = imgData;
	tmpData.chop(9);
	
	QByteArray rawData = QByteArray::fromHex(tmpData.right(tmpData.length() - 8).toLocal8Bit());
	
	QPixmap pic;
	
	if ( pic.loadFromData(rawData))
	{
	
	document()->addResource(QTextDocument::ImageResource,
		QUrl(Qt::escape(imgData)), pic);
	setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
	append("<img src='" + Qt::escape(imgData) +"'>");
	} else 
		append(Qt::escape(imgData));
	
	
}
