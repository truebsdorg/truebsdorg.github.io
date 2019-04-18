/*
    treeBuddyItem

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

#include <QSettings>
#include <QFile>
#include <QTextDocument>

#include "tlv.h"
#include "buffer.h"

#include "treebuddyitem.h"

treeBuddyItem::treeBuddyItem(QTreeWidgetItem *parent) : 
	QTreeWidgetItem(parent, 2), 
	groupID(0),
	groupName(0),
	itemId(0),
	isOffline(true),
	statusIconMethod(&statusIconClass::getOfflineIcon)
{

};

treeBuddyItem::treeBuddyItem(const QString &pUin, QIcon /* offlineIcon */, contactListTree *p, QTreeWidgetItem *parent) :
	QTreeWidgetItem(parent, 2), 
	statusIconMethod(&statusIconClass::getOfflineIcon),
	par(p), 
	parentUin(pUin)
{
	avatarMd5Hash.clear();
	setIcon(0, (statusIconClass::getInstance()->*statusIconMethod)());
	setText(3,QString::number(12));
	status = contactOffline;
	isOffline = true;
	groupID = 0;
	messageIcon = false;
	UTF8 = false;
	statusChanged = true;
	underline = true;
	notAutho = false;
	birth = false;
	authorizeMe = false;
	icqLite = false;
	externalIP = 0;
	internalIP = 0;
	onlineTime = 0;
	signonTime = 0;
	regTime = 0;
	clientId = "-";
	fileTransferSupport = false;
	xStatusPresent = false;
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+parentUin, "account");	
	iconPath = settings.fileName().section('/', 0, -2) + "/icqicons/";
	

}

treeBuddyItem::~treeBuddyItem()
{
	
}
void treeBuddyItem::setBuddyUin(const QString &text)
{
	buddyUin = text;
	buddyName = text;
	updateBuddyText();
}

void treeBuddyItem::checkForXStatus()
{
	QList<QByteArray> capList = capabilitiesList;
	
	if ( capList.contains(QByteArray::fromHex("01d8d7eeac3b492aa58dd3d877e66b92")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(0);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(0)));
	} else
	if ( capList.contains(QByteArray::fromHex("5a581ea1e580430ca06f612298b7e4c7")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(1);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(1)));
	}else
	if ( capList.contains(QByteArray::fromHex("83c9b78e77e74378b2c5fb6cfcc35bec")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(2);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(2)));
	}else
	if ( capList.contains(QByteArray::fromHex("e601e41c33734bd1bc06811d6c323d81")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(3);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(3)));
	}else
	if ( capList.contains(QByteArray::fromHex("8c50dbae81ed4786acca16cc3213c7b7")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(4);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(4)));
	}else
	if ( capList.contains(QByteArray::fromHex("3fb0bd36af3b4a609eefcf190f6a5a7f")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(5);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(5)));
	}else
	if ( capList.contains(QByteArray::fromHex("f8e8d7b282c4414290f810c6ce0a89a6")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(6);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(6)));
	}else
	if ( capList.contains(QByteArray::fromHex("80537de2a4674a76b3546dfd075f5ec6")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(7);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(7)));
	}else
	if ( capList.contains(QByteArray::fromHex("f18ab52edc57491d99dc6444502457af")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(8);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(8)));
	}else
	if ( capList.contains(QByteArray::fromHex("1b78ae31fa0b4d3893d1997eeeafb218")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(9);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(9)));
	}else
	if ( capList.contains(QByteArray::fromHex("61bee0dd8bdd475d8dee5f4baacf19a7")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(10);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(10)));
	}else
	if ( capList.contains(QByteArray::fromHex("488e14898aca4a0882aa77ce7a165208")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(11);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(11)));
	}else
	if ( capList.contains(QByteArray::fromHex("107a9a1812324da4b6cd0879db780f09")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(12);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(12)));
	}else
	if ( capList.contains(QByteArray::fromHex("6f4930984f7c4affa27634a03bceaea7")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(13);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(13)));
	}else
	if ( capList.contains(QByteArray::fromHex("1292e5501b644f66b206b29af378e48d")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(14);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(14)));
	}else
	if ( capList.contains(QByteArray::fromHex("d4a611d08f014ec09223c5b6bec6ccf0")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(15);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(15)));
	}else
	if ( capList.contains(QByteArray::fromHex("609d52f8a29a49a6b2a02524c5e9d260")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(16);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(16)));
	}else
	if ( capList.contains(QByteArray::fromHex("63627337a03f49ff80e5f709cde0a4ee")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(17);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(17)));
	}else
	if ( capList.contains(QByteArray::fromHex("1f7a4071bf3b4e60bc324c5787b04cf1")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(18);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(18)));
	}else
	if ( capList.contains(QByteArray::fromHex("785e8c4840d34c65886f04cf3f3f43df")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(19);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(19)));
	}else
	if ( capList.contains(QByteArray::fromHex("a6ed557e6bf744d4a5d4d2e7d95ce81f")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(20);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(20)));
	}else
	if ( capList.contains(QByteArray::fromHex("12d07e3ef885489e8e97a72a6551e58d")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(21);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(21)));
	}else
	if ( capList.contains(QByteArray::fromHex("ba74db3e9e24434b87b62f6b8dfee50f")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(22);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(22)));
	}else
	if ( capList.contains(QByteArray::fromHex("634f6bd8add24aa1aab9115bc26d05a1")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(23);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(23)));
	}else
	if ( capList.contains(QByteArray::fromHex("2ce0e4e57c6443709c3a7a1ce878a7dc")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(24);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(24)));
	}else
	if ( capList.contains(QByteArray::fromHex("101117c9a3b040f981ac49e159fbd5d4")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(25);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(25)));
	}else
	if ( capList.contains(QByteArray::fromHex("160c60bbdd4443f39140050f00e6c009")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(26);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(26)));
	}else
	if ( capList.contains(QByteArray::fromHex("6443c6af22604517b58cd7df8e290352")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(27);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(27)));
	}else
	if ( capList.contains(QByteArray::fromHex("16f5b76fa9d240358cc5c084703c98fa")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(28);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(28)));
	}else
	if ( capList.contains(QByteArray::fromHex("631436ff3f8a40d0a5cb7b66e051b364")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(29);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(29)));
	}else
	if ( capList.contains(QByteArray::fromHex("b70867f538254327a1ffcf4cc1939797")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(30);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(30)));
	}else
	if ( capList.contains(QByteArray::fromHex("ddcf0ea971954048a9c6413206d6f280")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(31);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(31)));
	}else
	if ( capList.contains(QByteArray::fromHex("d4e2b0ba334e4fa598d0117dbf4d3cc8")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(32);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(32)));
	}else
	if ( capList.contains(QByteArray::fromHex("0072d9084ad143dd91996f026966026f")))
	{
		xStatusPresent = true;
		xStatusIcon = statusIconClass::getInstance()->xstatusList.at(33);
		setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(33)));
	}
	else
	{
		xStatusPresent = false;
		xStatusIcon.clear();
		waitingForAuth(authorizeMe);
	}
	
	if ( !xStatusPresent && !xStatusMessage.isEmpty())
		{
			QString stat = xStatusMessage;
			if ( stat == "icqmood23")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(0);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(0)));
			}else
			if ( stat == "icqmood1")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(1);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(1)));
			}else
			if ( stat == "icqmood2")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(2);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(2)));
			}else
			if ( stat == "icqmood3")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(3);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(3)));
			}else
			if ( stat == "icqmood4")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(4);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(4)));
			}else
			if ( stat == "icqmood5")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(5);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(5)));
			}else
			if ( stat == "icqmood6")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(6);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(6)));
			}else
			if ( stat == "icqmood7")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(7);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(7)));
			}else
			if ( stat == "icqmood8")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(8);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(8)));
			}else
			if ( stat == "icqmood9")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(9);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(9)));
			}else
			if ( stat == "icqmood10")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(10);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(10)));
			}else
			if ( stat == "icqmood11")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(11);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(11)));
			}else
			if ( stat == "icqmood12")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(12);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(12)));
			}else
			if ( stat == "icqmood13")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(13);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(13)));
			}else
			if ( stat == "icqmood14")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(14);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(14)));
			}else
			if ( stat == "icqmood15")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(15);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(15)));
			}else
			if ( stat == "icqmood16")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(16);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(16)));
			}else
			if ( stat == "icqmood17")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(18);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(18)));
			}else
			if ( stat == "icqmood18")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(19);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(19)));
			}else
				if ( stat == "icqmood19")
						{
							xStatusPresent = true;
							xStatusIcon = statusIconClass::getInstance()->xstatusList.at(20);
							setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(20)));
						}else
			if ( stat == "icqmood20")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(21);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(21)));
			}else
			if ( stat == "icqmood21")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(22);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(22)));
			}else
			if ( stat == "icqmood22")
			{
				xStatusPresent = true;
				xStatusIcon = statusIconClass::getInstance()->xstatusList.at(23);
				setIcon(1, QIcon(statusIconClass::getInstance()->xstatusList.at(23)));
			}
			
		}
}

void treeBuddyItem::readData(icqBuffer *socket, quint16 length)
{
	notAutho = false;
	for ( ;length > 0; )
	{
		tlv tmpTlv;
		tmpTlv.readData(socket);
		takeTlv(tmpTlv);
		length -= tmpTlv.getLength();
	}
	updateBuddyText();
}

void treeBuddyItem::takeTlv(tlv &newTlv)
{
	QString tmpBuddyName;
	switch(newTlv.getTlvType())
	{
	case 0x0131:
		tmpBuddyName = QString::fromUtf8(newTlv.getTlvData());
		if ( tmpBuddyName != buddyUin)
		{
			buddyName = tmpBuddyName;
			updateBuddyText();
		}
		break;
		
	case 0x0066:
		notAutho = true;
		updateBuddyText();
		break;

	default:
		break;
	}
}

void treeBuddyItem::updateBuddyText()
{
	
	setText(1,buddyName);
	QFont tmpFont = font(1);
	if ( underline )
	{		
		tmpFont.setUnderline(notAutho);	
	} else {
		tmpFont.setUnderline(false);
	}
	
	if ( birth )
	{
		if ( birthDay == QDate::currentDate() )
			tmpFont.setWeight(QFont::Bold);
	} else {
		tmpFont.setWeight(QFont::Normal);
	}
	
	setFont(1, tmpFont);
	
	createToolTip();
}

void treeBuddyItem::oncoming(icqBuffer *socket, quint16 length)
{
	socket->read(2);
	length -= 4;
	quint16 arraySize = byteArrayToInt16(socket->read(2));
	for ( int i = 0; i < arraySize; i++ )
	{
		tlv tmpTlv;
		tmpTlv.readData(socket);
		takeOncomingTlv(tmpTlv);
		length -= tmpTlv.getLength();
	}
	
	if ( status == contactOffline )
	{
		QByteArray tmp;
		tmp.append(0x10);
		tmp.append((char)0x00);
		tmp.append((char)0x00);
		tmp.append((char)0x00);
		changeStatus(tmp);
		
	}
	if ( length )
		socket->read(length);
}

void treeBuddyItem::takeOncomingTlv(tlv &newTlv)
{
	switch(newTlv.getTlvType())
	{
	case 0x0006:
		changeStatus(newTlv.getTlvData());
		break;
		
	case 0x000d:
		setCapabilities(newTlv.getTlvData());
		break;
	case 0x001d:
		readAvailableMessTlv(newTlv.getTlvData());
		break;
	case 0x000a:
		setExtIp(newTlv.getTlvData());
		break;
	case 0x000c:
		setIntIp(newTlv.getTlvData());
		break;
	case 0x000f:
		setOnlTime(newTlv.getTlvData());
		break;
	case 0x0003:
		setSignOn(newTlv.getTlvData());
		break;
	case 0x0005:
		setregTime(newTlv.getTlvData());
		break;
	case 0x0019:
		readShortCap(newTlv.getTlvLength(), newTlv.getTlvData());
		break;
	default:
		break;
	}
}

quint16 treeBuddyItem::byteArrayToInt16(const QByteArray &array) const
{
	bool ok;
	return array.toHex().toUInt(&ok,16);	
}

void treeBuddyItem::changeStatus(const QByteArray &userStatus)
{
	if ( userStatus.size() == 4 )
	{
		isOffline = false;
		quint16 userstatus = static_cast<quint16>(userStatus.at(2) * 0x100 + userStatus.at(3));
		quint16 userFlags = static_cast<quint16>(userStatus.at(0) * 0x100 + userStatus.at(1));
		if ( userFlags & 0x0008 )
		{
			birthDay = QDate::currentDate();
			updateBuddyText();
		}
		contactStatus tmpStatus = status;
		switch (userstatus)
		{
		case 0x0000:
			statusIconMethod = &statusIconClass::getOnlineIcon;
			status = contactOnline;
			break;
		case 0x0001:
			statusIconMethod = &statusIconClass::getAwayIcon;
			status = contactAway;
			break;
		case 0x0002:
		case 0x0013:
			statusIconMethod = &statusIconClass::getDoNotDisturbIcon;
			status = contactDnd;
			break;
		case 0x0004:
		case 0x0005:
			statusIconMethod = &statusIconClass::getNotAvailableIcon;
			status = contactNa;
			break;
		case 0x0010:
		case 0x0011:
			statusIconMethod = &statusIconClass::getOccupiedIcon;
			status = contactOccupied;
			break;
		case 0x0020:
			statusIconMethod = &statusIconClass::getFreeForChatIcon;
			status = contactFfc;
			break;
		case 0x0100:
			statusIconMethod = &statusIconClass::getInvisibleIcon;
			status = contactInvisible;
			break;
		case 0x2001:
			statusIconMethod = &statusIconClass::getLunchIcon;
			status = contactLunch;
			break;
		case 0x3000:
			statusIconMethod = &statusIconClass::getEvilIcon;
			status = contactEvil;
			break;		
		case 0x4000:
			statusIconMethod = &statusIconClass::getDepressionIcon;
			status = contactDepression;
			break;
		case 0x5000:
			statusIconMethod = &statusIconClass::getAtHomeIcon;
			status = contactAtHome;
			break;
		case 0x6000:
			statusIconMethod = &statusIconClass::getAtWorkIcon;
			status = contactAtWork;
			break;
		default:
			statusIconMethod = &statusIconClass::getOnlineIcon;
			status = contactOnline;
		}
		if ( status != tmpStatus )
		{
			setIcon(0, (statusIconClass::getInstance()->*statusIconMethod)());
			setText(3, QString::number(status));
			statusChanged = true;
		} else
			statusChanged = false;

		createToolTip();
	}
}

void treeBuddyItem::buddyOffline()
{
	statusIconMethod = &statusIconClass::getOfflineIcon;
	setIcon(0, (statusIconClass::getInstance()->*statusIconMethod)());	
	isOffline = true;
	status = contactOffline;
	setText(3,QString::number(contactOffline));
	createToolTip();
	waitingForAuth(authorizeMe);
	xStatusPresent = false;
}

void treeBuddyItem::readMessage()
{
//	par->readMessageStack();
}

void treeBuddyItem::setCapabilities(QByteArray capList)
{
	capabilitiesList.clear();
	int size = capList.length() / 16;
	for ( int i = 0; i < size; i++)
	{
		QByteArray capability = capList.right(16);
		
		capabilitiesList.append(capability);
		
		if (isUtf8Cap(capability))
		{
			UTF8 = true;
		}
		
		if ( capability == QByteArray::fromHex("094613434c7f11d18222444553540000"))
			fileTransferSupport = true;
		if ( capability == QByteArray::fromHex("178c2d9bdaa545bb8ddbf3bdbd53a10a"))
			icqLite = true;
		capList = capList.left(capList.length() - 16);
	}
}

bool treeBuddyItem::isUtf8Cap(const QByteArray &utf8Cap)
{
	bool ok;
	if ( utf8Cap.left(4).toHex().toUInt(&ok,16) == 0x0946134e )
	{
		return true;
	}
	else
		return false;
}


bool treeBuddyItem::operator< ( const QTreeWidgetItem & other ) const  
{

			int column = treeWidget()->model()->headerData(0,Qt::Horizontal).toInt();
			if ( isOffline )
			{
				QString myText = text(1).toUpper();
				QString otherText = other.text(1).toUpper();
				return myText < otherText;
			}
			else 
			{
			if ( column == 3 )
			{
				int myNumber = text(3).toInt();
				int otherNumber = other.text(3).toInt();
				return myNumber < otherNumber;				
			} else{
				QString myText = text(1).toUpper();
				QString otherText = other.text(1).toUpper();
				return myText < otherText;
			}
				}
			
}

void treeBuddyItem::setCustomFont(const QString &f, int s, const QColor &c)
{
	QFont cfont = font(1);
	cfont.setFamily(f);
	cfont.setPointSize(s);
	setFont(1, cfont);
	setForeground(1, c);
}

quint8 treeBuddyItem::byteArrayToInt8(const QByteArray &array) const
{
	bool ok;
	return array.toHex().toUInt(&ok,16);
}

void treeBuddyItem::readAvailableMessTlv(QByteArray avTlv)
{
	xStatusMessage.clear();
	for ( ;avTlv.size(); )
	{
		quint16 id = byteArrayToInt16(avTlv.left(2));
		avTlv = avTlv.right(avTlv.size() - 2);
		picFlags = byteArrayToInt8(avTlv.left(1));
		avTlv = avTlv.right(avTlv.size() - 1);
		quint8 length = byteArrayToInt8(avTlv.left(1));
		avTlv = avTlv.right(avTlv.size() - 1);
		
		if ( id != 2 && id != 0x000e && length == 16 )
		{
			avatarMd5Hash = avTlv.left(length);
		}
		
		if ( id == 0x000e )
		{
			xStatusMessage = avTlv.left(length);

		}
		avTlv = avTlv.right(avTlv.size() - length);
		
	}
}

void treeBuddyItem::waitingForAuth(bool wait)
{
	authorizeMe = wait;
	
	if ( wait )
		// TO-DO: Will be changed in future
		//setIcon(1, QIcon(":/icons/icq/auth.png"));
		setIcon(1, statusIconClass::getInstance()->getConnectingIcon());
	else
	{
		setIcon(1, QIcon());
		authMessage.clear();
	}
}

QString treeBuddyItem::statToStr(contactStatus st)
{
	switch(st){
		case contactOnline:
			return statusIconClass::getInstance()->getIconFactory()->getOnlinePath();
		case contactFfc:
			return statusIconClass::getInstance()->getIconFactory()->getFreeForChatPath();
		case contactAway:
			return statusIconClass::getInstance()->getIconFactory()->getAwayPath();
		case contactLunch:
			return statusIconClass::getInstance()->getIconFactory()->getLunchPath();
		case contactAtWork:
			return statusIconClass::getInstance()->getIconFactory()->getAtWorkPath();
		case contactAtHome:
			return statusIconClass::getInstance()->getIconFactory()->getAtHomePath();
		case contactEvil:
			return statusIconClass::getInstance()->getIconFactory()->getEvilPath();
		case contactDepression:
			return statusIconClass::getInstance()->getIconFactory()->getDepressionPath();
		case contactNa:
			return statusIconClass::getInstance()->getIconFactory()->getNotAvailablePath();
		case contactOccupied:
			return statusIconClass::getInstance()->getIconFactory()->getOccupiedPath();
		case contactDnd:
			return statusIconClass::getInstance()->getIconFactory()->getDoNotDisturbPath();
		case contactInvisible:
			return statusIconClass::getInstance()->getIconFactory()->getInvisiblePath();
		case contactOffline:
			return statusIconClass::getInstance()->getIconFactory()->getOfflinePath();
		default:
			return "";
	}
}

void treeBuddyItem::createToolTip()
{
	QString customToolTip;
	QString st = statToStr(status);

	customToolTip.append("<table><tr><td>");

	customToolTip.append("<img src='" + st + "'> <b>" + Qt::escape(buddyName) + "</b> (" + buddyUin + ")<br>");
	if ( status != contactOffline ){
		if ( !xStatusIcon.isEmpty() && (!xStatusCaption.isEmpty() || !xStatusMsg.isEmpty()) )
		{
			customToolTip.append("<img src='" + xStatusIcon + "'> ");
			if ( !xStatusCaption.isEmpty())
				customToolTip.append("<b>" + Qt::escape(xStatusCaption) +"</b> ");
	
			if ( !xStatusMsg.isEmpty())
				customToolTip.append(Qt::escape(xStatusMsg));
			customToolTip.append("<br>");
		}
	}

	if ( externalIP )
	{
		customToolTip.append(QObject::tr("<font size='2'><b>External ip:</b> %1.%2.%3.%4<br></font>").arg(externalIP / 0x1000000).arg(
				     externalIP % 0x1000000 / 0x10000).arg( externalIP % 0x10000 / 0x100).arg(externalIP % 0x100));
	}
	
	if ( internalIP )
	{
		customToolTip.append(QObject::tr("<font size='2'><b>Internal ip:</b> %1.%2.%3.%4<br></font>").arg(internalIP / 0x1000000).arg(
				     internalIP % 0x1000000 / 0x10000).arg( internalIP % 0x10000 / 0x100).arg(internalIP % 0x100));
	}
	
	if ( status != contactOffline )
	{
		QDateTime time;
		time = time.toUTC();
		time.setTime_t(0);
		time = time.addSecs(onlineTime);
		
		customToolTip.append(QObject::tr("<font size='2'><b>Online time:</b> %1d %2h %3m %4s<br>").arg(time.date().day() - 1).arg(
				     time.time().hour()).arg(time.time().minute()).arg(time.time().second()));
		
		time.setTime_t(signonTime);
		customToolTip.append(QObject::tr("<b>Signed on:</b> %1<br>").arg(time.toLocalTime().toString("hh:mm:ss dd/MM/yyyy")));
		time.setTime_t(regTime);
		
		customToolTip.append(QObject::tr("<b>Reg. date:</b> %1<br>").arg(time.toString("hh:mm:ss dd/MM/yyyy")));
		customToolTip.append(QObject::tr("<b>Possible client:</b> %1</font>").arg(clientId));
	}
	customToolTip.append("</td><td>");
	if ( avatarMd5Hash.length() == 16 )
	{
		QString avatarPath = iconPath + avatarMd5Hash.toHex();
		if ( QFile::exists(avatarPath))
			customToolTip.append("<img src='" + avatarPath + "' align='right'>");
	}
	customToolTip.append("</td></tr></table>");
	setToolTip(1, customToolTip);
}

quint32 treeBuddyItem::convertToInt32(const QByteArray &array) const
{
	bool ok;
	return array.toHex().toULong(&ok,16);
}

void treeBuddyItem::setExtIp(const QByteArray &array)
{
	externalIP = convertToInt32(array);

}

void treeBuddyItem::setIntIp(const QByteArray &array)
{
	internalIP = convertToInt32(array.left(4));
	userPort = convertToInt32(array.left(8).right(4));
	protocolVersion = (quint8)array.at(10);
	Cookie = convertToInt32(array.left(15).right(4));
	lastInfoUpdate = convertToInt32(array.left(27).right(4));
	lastExtStatusInfoUpdate	= convertToInt32(array.left(31).right(4));
	lastExtInfoUpdate  = convertToInt32(array.left(35).right(4));
}

void treeBuddyItem::setOnlTime(const QByteArray &array)
{
	onlineTime = convertToInt32(array);
}

void treeBuddyItem::setSignOn(const QByteArray &array)
{
	signonTime = convertToInt32(array);

}

void treeBuddyItem::setregTime(const QByteArray &array)
{
	regTime = convertToInt32(array);

}

void treeBuddyItem::readShortCap(quint16 length, const QByteArray &array)
{
	shortCaps.clear();
	for(;length; length -= 2)
	{
		quint16 cap = byteArrayToInt16(array.left(length).right(2));
		shortCaps.append(cap);
		if ( cap == 0x134e)
			UTF8 =  true;
		if (cap == 0x1343)
			fileTransferSupport = true;
//			icqLite = (cap == 0x1343);

		
	}
}
