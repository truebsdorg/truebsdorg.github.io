/*
    historyXML

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


#ifndef HISTORYXML_H_
#define HISTORYXML_H_

#include <QList>

#include "icqmessage.h"

class QListWidget;

class chatWindow;
class icqTextBrowser;
class treeBuddyItem;

class historyXML
{
public:
	historyXML(const QString &);
	~historyXML();
	void saveHistoryMessage(const QString &,const QString &, const QDateTime &, bool, const QString &);
	void saveServiceMessage(const QString &, const QDateTime &, int, const QString &msg = QString(""));
	void setRecentMessages(chatWindow *, int);
	void saveUnreadedMessages(QHash<QString, treeBuddyItem  *> *);
	void loadUnreaded(QList<messageFormat> *);
	void addHistoryToList(QListWidget *, const QString &, const QString &);
	void addServiceHistoryToList(QListWidget *, const QString &);
	void addHistoryMessages(icqTextBrowser *, const QDate &, const QString &, const QString &);
	void addServiceHistoryMessages(icqTextBrowser *, const QDate &);
	void deleteHistory(const QString &);
private:
	QString historyPath;
	QTime setTimeFromString(const QString &);
	QString findUrls(const QString &);
};

#endif /*HISTORYXML_H_*/
