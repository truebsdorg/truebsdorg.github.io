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


#ifndef TREEBUDDYITEM_H_
#define TREEBUDDYITEM_H_

#include <QTreeWidgetItem>

#include "statusiconsclass.h"
#include "quticqglobals.h"

class messageFormat;
class contactListTree;	
class icqBuffer;
class tlv;

class treeBuddyItem : public QTreeWidgetItem
{
public:
	treeBuddyItem(QTreeWidgetItem *parent);
	treeBuddyItem(const QString &,QIcon ,contactListTree *, QTreeWidgetItem *parent);
	~treeBuddyItem();

	void checkForXStatus();
	void readData(icqBuffer *, quint16);
	void oncoming(icqBuffer *, quint16);
	void buddyOffline();
	void setBuddyUin(const QString &);
	quint16 groupID;
	QString groupName;
	quint16 itemId;
	bool isOffline;
	inline contactStatus getStatus() const { return status; };
	inline QString getName() const { return buddyName; };
	inline QString getUin() const { return buddyUin; };
	void setName(const QString& n) { buddyName = n; }
	QList<messageFormat *> messageList;
	statusIconClass::getIconFunction statusIconMethod;
	bool messageIcon;
	void readMessage();
	contactListTree *par;
	bool UTF8;
	bool statusChanged;
	void updateBuddyText();
	
	bool operator< ( const QTreeWidgetItem  &other ) const  ;
	bool underline;
	bool notAutho;
	bool birth;
	QDate birthDay;
	void setCustomFont(const QString &, int, const QColor &);
	
	quint8 picFlags;
	
	QByteArray avatarMd5Hash;
	
	void waitingForAuth(bool);
	bool authorizeMe;
	QString authMessage;
	
	QList<QByteArray> capabilitiesList;
	
	QString clientId;
	quint32 externalIP;
	quint32 internalIP;
	quint32 onlineTime;
	quint32 signonTime;
	quint32 regTime;
	
	bool fileTransferSupport;
	
	bool xStatusPresent;
	
	bool icqLite;
	QByteArray xStatusMessage;
	QString xStatusIcon;
	void createToolTip();
	
	QString xStatusCaption;
	QString xStatusMsg;

	quint8 protocolVersion;
	quint32 userPort;
	quint32 Cookie;
	quint32 lastInfoUpdate;
	quint32 lastExtInfoUpdate;
	quint32 lastExtStatusInfoUpdate;
	
	QList<quint16> shortCaps;
	
	inline QString getParentUin() const { return parentUin; };
private:
	void setCapabilities(QByteArray);
	bool isUtf8Cap(const QByteArray &);
	void takeOncomingTlv(tlv &);
	void takeTlv(tlv &);
	quint32 convertToInt32(const QByteArray &) const;
	quint16 byteArrayToInt16(const QByteArray &) const;
	quint8 byteArrayToInt8(const QByteArray &) const;
	void changeStatus(const QByteArray &);
	QString buddyUin;
	QString buddyName;
	contactStatus status;
	void readAvailableMessTlv(QByteArray);

	QString iconPath;
	QString parentUin;

	void setExtIp(const QByteArray &);
	void setIntIp(const QByteArray &);
	void setOnlTime(const QByteArray &);
	void setSignOn(const QByteArray &);
	void setregTime(const QByteArray &);
	void readShortCap(quint16 ,const QByteArray &);
	QString statToStr(contactStatus st);
};

#endif /*TREEBUDDYITEM_H_*/
