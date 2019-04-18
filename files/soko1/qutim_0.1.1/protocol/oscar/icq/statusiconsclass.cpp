/*
    statusIconClass

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

#include "statusiconfactory.h"
#include "statusiconsclass.h"

// Statics
QMutex statusIconClass::fInstGuard;
statusIconClass *statusIconClass::fInstance = NULL;

statusIconClass::statusIconClass() :
	fIconFactory(NULL)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	statusIconType iconType = static_cast<statusIconType>(settings.value("statusicons/type", BuiltInStatusIcons).toInt());
	QString iconSettingsFile = settings.value("statusicons/settings", QString("")).toString();

	settings.setValue("statusicons/type", iconType);
	settings.setValue("statusicons/settings", iconSettingsFile);

	fIconFactory = new statusIconFactory(iconType, iconSettingsFile);
	reloadIcons(fIconFactory);
}

statusIconClass::~statusIconClass()
{
	delete fIconFactory;
	fIconFactory = NULL;
}

statusIconClass *statusIconClass::getInstance()
{
	if (!fInstance)
	{
		QMutexLocker locker(&fInstGuard);
		if (!fInstance)
		{
			fInstance = new statusIconClass;
			atexit(&statusIconClass::release);
		}
	}

	return fInstance;
}

void statusIconClass::release()
{
	QMutexLocker locker(&fInstGuard);
	if (fInstance)
	{
		delete fInstance;
		fInstance = NULL;
	}
}

bool statusIconClass::reloadIcons(const statusIconFactory *iconFactory)
{
	QMutexLocker locker(&fIconLocker);

	onlineIcon = QIcon(iconFactory->getOnlinePath());
	ffcIcon = QIcon(iconFactory->getFreeForChatPath());
	awayIcon = QIcon(iconFactory->getAwayPath());
	naIcon = QIcon(iconFactory->getNotAvailablePath());
	occupiedIcon = QIcon(iconFactory->getOccupiedPath());
	dndIcon = QIcon(iconFactory->getDoNotDisturbPath());
	invisibleIcon = QIcon(iconFactory->getInvisiblePath());
	offlineIcon = QIcon(iconFactory->getOfflinePath());
	connectingIcon = QIcon(iconFactory->getConnectingPath());
	atHomeIcon = QIcon(iconFactory->getAtHomePath());
	atWorkIcon = QIcon(iconFactory->getAtWorkPath());
	lunchIcon = QIcon(iconFactory->getLunchPath());
	evilIcon = QIcon(iconFactory->getEvilPath());
	depressionIcon = QIcon(iconFactory->getDepressionPath());
	contentIcon = QIcon(iconFactory->getContentPath());
	
	xstatusList.clear();
	xstatusList.append(":/icons/xstatus/icq_xstatus0.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus1.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus2.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus3.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus4.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus5.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus6.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus7.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus8.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus9.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus10.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus11.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus12.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus13.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus14.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus15.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus16.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus17.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus18.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus19.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus20.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus21.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus22.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus23.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus24.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus25.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus26.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus27.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus28.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus29.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus30.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus31.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus32.png");
	xstatusList.append(":/icons/xstatus/icq_xstatus33.png");
	
//	xstatusList.insert(0, ":/icons/xstatus/icq_xstatus0.png");
//	xstatusList.insert(1, ":/icons/xstatus/icq_xstatus1.png");
//	xstatusList.insert(2, ":/icons/xstatus/icq_xstatus2.png");
//	xstatusList.insert(3, ":/icons/xstatus/icq_xstatus3.png");
//	xstatusList.insert(4, ":/icons/xstatus/icq_xstatus4.png");
//	xstatusList.insert(5, ":/icons/xstatus/icq_xstatus5.png");
//	xstatusList.insert(6, ":/icons/xstatus/icq_xstatus6.png");
//	xstatusList.insert(7, ":/icons/xstatus/icq_xstatus7.png");
//	xstatusList.insert(8, ":/icons/xstatus/icq_xstatus8.png");
//	xstatusList.insert(9, ":/icons/xstatus/icq_xstatus9.png");
//	xstatusList.insert(10, ":/icons/xstatus/icq_xstatus10.png");
//	xstatusList.insert(11, ":/icons/xstatus/icq_xstatus11.png");
//	xstatusList.insert(12, ":/icons/xstatus/icq_xstatus12.png");
//	xstatusList.insert(13, ":/icons/xstatus/icq_xstatus13.png");
//	xstatusList.insert(14, ":/icons/xstatus/icq_xstatus14.png");
//	xstatusList.insert(15, ":/icons/xstatus/icq_xstatus15.png");
//	xstatusList.insert(16, ":/icons/xstatus/icq_xstatus16.png");
//	xstatusList.insert(17, ":/icons/xstatus/icq_xstatus17.png");
//	xstatusList.insert(18, ":/icons/xstatus/icq_xstatus18.png");
//	xstatusList.insert(19, ":/icons/xstatus/icq_xstatus19.png");
//	xstatusList.insert(20, ":/icons/xstatus/icq_xstatus20.png");
//	xstatusList.insert(21, ":/icons/xstatus/icq_xstatus21.png");
//	xstatusList.insert(22, ":/icons/xstatus/icq_xstatus22.png");
//	xstatusList.insert(23, ":/icons/xstatus/icq_xstatus23.png");
//	xstatusList.insert(24, ":/icons/xstatus/icq_xstatus24.png");
//	xstatusList.insert(25, ":/icons/xstatus/icq_xstatus25.png");
//	xstatusList.insert(26, ":/icons/xstatus/icq_xstatus26.png");
//	xstatusList.insert(27, ":/icons/xstatus/icq_xstatus27.png");
//	xstatusList.insert(28, ":/icons/xstatus/icq_xstatus28.png");
//	xstatusList.insert(29, ":/icons/xstatus/icq_xstatus29.png");
//	xstatusList.insert(30, ":/icons/xstatus/icq_xstatus30.png");
//	xstatusList.insert(31, ":/icons/xstatus/icq_xstatus31.png");
//	xstatusList.insert(32, ":/icons/xstatus/icq_xstatus32.png");
//	xstatusList.insert(33, ":/icons/xstatus/icq_xstatus33.png");
	return true;
}

bool statusIconClass::reloadIconsFromSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	statusIconType iconType = static_cast<statusIconType>(settings.value("statusicons/type", BuiltInStatusIcons).toInt());
	QString iconSettingsFile = settings.value("statusicons/settings", QString("")).toString();

	if (fIconFactory)
	{
		delete fIconFactory;
		fIconFactory = new statusIconFactory(iconType, iconSettingsFile);
	}

	return reloadIcons(fIconFactory);
}

const QIcon &statusIconClass::getOnlineIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return onlineIcon;
}

const QIcon	&statusIconClass::getFreeForChatIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return ffcIcon;
}

const QIcon	&statusIconClass::getAwayIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return awayIcon;
}

const QIcon	&statusIconClass::getNotAvailableIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return naIcon;
}

const QIcon	&statusIconClass::getOccupiedIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return occupiedIcon;
}

const QIcon &statusIconClass::getDoNotDisturbIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return dndIcon;
}

const QIcon &statusIconClass::getInvisibleIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return invisibleIcon;
}

const QIcon &statusIconClass::getOfflineIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return offlineIcon;
}

const QIcon &statusIconClass::getConnectingIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return connectingIcon;
}

const QIcon &statusIconClass::getAtHomeIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return atHomeIcon;
}

const QIcon &statusIconClass::getAtWorkIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return atWorkIcon;
}

const QIcon &statusIconClass::getLunchIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return lunchIcon;
}

const QIcon &statusIconClass::getEvilIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return evilIcon;
}

const QIcon &statusIconClass::getDepressionIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return depressionIcon;
}

const QIcon &statusIconClass::getContentIcon() const
{
	QMutexLocker locker(&fIconLocker);
	return contentIcon;
}

