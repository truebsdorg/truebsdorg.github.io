/*
    statusIconFactory

    Copyright (c) 2008 by Dmitri Arekhta <DaemonES@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#ifndef _STATUSICONFACTORY_H_
#define _STATUSICONFACTORY_H_

#include <QString>
#include <QMap>

enum statusIconType
{
	BuiltInStatusIcons = 0,
	AdiumStatusIcons
};

class statusIconFactory
{
public:
	statusIconFactory(statusIconType type, const QString settingsFile = QString());
	~statusIconFactory();

	statusIconType						getStatusIconType() const { return fIconType; }
	QString								getSettingsFilePath() const { return fIconSettingsFile; }
	
	QString								getOnlinePath() const;
	QString								getFreeForChatPath() const;
	QString								getAwayPath() const;
	QString								getNotAvailablePath() const;
	QString								getOccupiedPath() const;
	QString								getDoNotDisturbPath() const;
	QString								getInvisiblePath() const;
	QString								getOfflinePath() const;
	QString								getConnectingPath() const;
	QString								getAtHomePath() const;
	QString								getAtWorkPath() const;
	QString								getLunchPath() const;
	QString								getEvilPath() const;
	QString								getDepressionPath() const;
	
	QString								getContentPath() const;
	QString								getTypingPath() const;
	QString								getMobilePath() const;
	QString								getIdlePath() const;
	
private:
	statusIconType						fIconType;
	QString								fIconSettingsFile;
	QMap<QString, QString>				fListIconsMap;
	QMap<QString, QString>				fTabIconsMap;

};




#endif
