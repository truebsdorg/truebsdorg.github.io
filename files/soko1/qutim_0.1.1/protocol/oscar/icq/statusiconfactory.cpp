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

#include <QtXml>

#include "statusiconfactory.h"

statusIconFactory::statusIconFactory(statusIconType type, const QString settingsFile /* = QString() */ )
{
	fIconType = type;
	fIconSettingsFile = settingsFile;

	// Parsing adium icon style
	if (fIconType == AdiumStatusIcons)
	{
		do
		{
#if defined(Q_OS_WIN)
			QFile xmlFile(fIconSettingsFile + "\\Icons.plist");
#else
			QFile xmlFile(fIconSettingsFile + "/Icons.plist");
#endif
			if (!xmlFile.exists())
			{
				fIconType = BuiltInStatusIcons;
				break;
			}

			if (xmlFile.open(QIODevice::ReadOnly))
			{
				QDomDocument adiumIconsPref;

				if (adiumIconsPref.setContent(&xmlFile))
				{
					QDomElement rootElem = adiumIconsPref.documentElement();
					if (rootElem.isNull())
					{
						fIconType = BuiltInStatusIcons;
						break;
					}
					
					for (QDomNode node = rootElem.firstChild(); !node.isNull(); node = node.nextSibling())
					{
						for (QDomNode dictNode = node.firstChild(); !dictNode.isNull(); dictNode = dictNode.nextSibling())
						{
							QDomElement dictElem = dictNode.toElement();
							if (!dictElem.isNull())
							{
								//QString tagName = dictElem.tagName();
								QString attrName = dictElem.text();
								if (attrName == "List" || attrName == "Tabs")
								{
									dictNode = dictNode.nextSibling();
									QDomElement childDict = dictNode.toElement();
									if (!dictNode.isNull())
									{
										QDomElement childDict = dictNode.toElement();
										if (!childDict.isNull() && childDict.tagName() == "dict")
										{
											QString iconType;
											QString iconPath;

											for (QDomNode iconNode = childDict.firstChild(); !iconNode.isNull(); iconNode = iconNode.nextSibling())
											{
												QDomElement iconElement = iconNode.toElement();
												if (!iconElement.isNull())
												{
													if (iconElement.tagName() == "key")
													{
														iconType = iconElement.text();
													}
													else if (iconElement.tagName() == "string")
													{
														iconPath = fIconSettingsFile + "/" + iconElement.text();

														if (attrName == "List")
															fListIconsMap.insert(iconType, iconPath);
														else if (attrName == "Tabs")
															fTabIconsMap.insert(iconType, iconPath);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}

				xmlFile.close();
			}
		}
		while(false);

		if (fListIconsMap["Generic Available"] == "" || fListIconsMap["Offline"] == "")
		{
			fIconType = BuiltInStatusIcons;
		}
	}
}

statusIconFactory::~statusIconFactory()
{

}
	
QString statusIconFactory::getOnlinePath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Generic Available"].isEmpty())
	{
		return fListIconsMap["Generic Available"];
	}

	return QString(":/icons/icq/online.png");
}

QString	statusIconFactory::getFreeForChatPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Free for Chat"].isEmpty())
	{
		return fListIconsMap["Free for Chat"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getOnlinePath();
	}

	return QString(":/icons/icq/ffc.png");
}

QString	statusIconFactory::getAwayPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Generic Away"].isEmpty())
	{
		return fListIconsMap["Generic Away"];
	}

	return QString(":/icons/icq/away.png");
}

QString	statusIconFactory::getNotAvailablePath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Not Available"].isEmpty())
	{
		return fListIconsMap["Not Available"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getAwayPath();
	}

	return QString(":/icons/icq/na.png");
}

QString	statusIconFactory::getOccupiedPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Occupied"].isEmpty())
	{
		return fListIconsMap["Occupied"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getAwayPath();
	}

	return QString(":/icons/icq/occupied.png");
}

QString	statusIconFactory::getDoNotDisturbPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["DND"].isEmpty())
	{
		return fListIconsMap["DND"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getAwayPath();
	}

	return QString(":/icons/icq/dnd.png");
}

QString	statusIconFactory::getInvisiblePath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Invisible"].isEmpty())
	{
		return fListIconsMap["Invisible"];
	}

	return QString(":/icons/icq/invisible.png");
}

QString	statusIconFactory::getOfflinePath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Offline"].isEmpty())
	{
		return fListIconsMap["Offline"];
	}

	return QString(":/icons/icq/offline.png");
}

QString	statusIconFactory::getConnectingPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Unknown"].isEmpty())
	{
		return fListIconsMap["Unknown"];
	}

	return QString(":/icons/icq/connecting.png");
}

QString	statusIconFactory::getAtHomePath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Available At Home"].isEmpty())
	{
		return fListIconsMap["Available At Home"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getOnlinePath();
	}

	return QString(":/icons/icq/athome.png");
}

QString	statusIconFactory::getAtWorkPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Available At Work"].isEmpty())
	{
		return fListIconsMap["Available At Work"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getOnlinePath();
	}

	return QString(":/icons/icq/atwork.png");
}

QString	statusIconFactory::getLunchPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Lunch"].isEmpty())
	{
		return fListIconsMap["Lunch"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getOnlinePath();
	}	

	return QString(":/icons/icq/lunch.png");
}

QString	statusIconFactory::getEvilPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Available Evil"].isEmpty())
	{
		return fListIconsMap["Available Evil"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getOnlinePath();
	}

	return QString(":/icons/icq/evil.png");
}

QString	statusIconFactory::getDepressionPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Available Depression"].isEmpty())
	{
		return fListIconsMap["Available Depression"];
	}
	else if (fIconType == AdiumStatusIcons)
	{
		return getOnlinePath();
	}

	return QString(":/icons/icq/depression.png");
}

/*
 * Unread message from the user
 */
QString statusIconFactory::getContentPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["content"].isEmpty())
	{
		return fListIconsMap["content"];
	}

	return QString(":/icons/crystal_project/message.png");
}

/*
 * User is typing
 */
QString statusIconFactory::getTypingPath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["typing"].isEmpty())
	{
		return fListIconsMap["typing"];
	}

	return QString(":/icons/crystal_project/message.png");
}

/*
 * User is using mobile version of client
 */
QString statusIconFactory::getMobilePath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Mobile"].isEmpty())
	{
		return fListIconsMap["Mobile"];
	}

	return QString(":/icons/icq/online.png");
}

/*
 * User is in idle state
 */
QString statusIconFactory::getIdlePath() const
{
	if (fIconType == AdiumStatusIcons && !fListIconsMap["Idle"].isEmpty())
	{
		return fListIconsMap["Idle"];
	}

	return QString(":/icons/icq/online.png");
}
