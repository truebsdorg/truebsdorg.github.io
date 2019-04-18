/*
    contactListTree

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

//#include <QtGui>
#include <QTcpSocket>
#include <QNetworkProxy>
#include "treegroupitem.h"
#include "treebuddyitem.h"
#include "icqmessage.h"
#include "chatwindow.h"
#include "buffer.h"
#include "tabchatwindow.h"
#include "metainformation.h"
//#include "statusiconsclass.h"
#include "servicessetup.h"
#include "historyxml.h"
#include "historyview.h"
#include "buddypicture.h"
#include "searchuser.h"
#include "userinformation.h"
#include "addbuddydialog.h"
//#include "modifyitem.h"
#include "multiplesending.h"
#include "privacylistwindow.h"
#include "passwordchangedialog.h"
#include "addrenamedialog.h"
#include "readawaydialog.h"
#include "deletecontactdialog.h"
#include "requestauthdialog.h"
#include "acceptauthdialog.h"
//#include "clientidentify.h"
#include "pluginsystem.h"
#include "filetransfer.h"
#include "snac.h"
#include "contactseparator.h"
#include "tlv.h"
#include "qutim.h"
#include "soundevents.h"

#include "contactlist.h"
#include "notewidget.h"

#if defined(_MSC_VER)
#pragma warning (disable:4138)
#endif

contactListTree::contactListTree(QTcpSocket *s, icqBuffer *buff, QTreeWidget *list, const QString &uin, quint16 &flap,  quint32 &snac, quint16 &meta, QObject *parent) :
	QObject(parent), 
        statusIconObject(statusIconClass::getInstance()),
	tcpSocket(s), 
	socket(buff), 
	contactList(list), 
        icqUin(uin)
{
	flapSeq = &flap;
	snacSeq = &snac;
	metaSeq = &meta;
	newMessages = false;
	isMergeAccounts = false;
	contactListChanged = false;
	iAmOnline = false;
	showGroups = false;
	clearNil = false;
	waitForMineInfo = false;
	mineMetaSeq = 0;
	pdInfoID = 0;
	justStarted = false;

	codec = QTextCodec::codecForName("Windows-1251");
	if (!codec)
		codec = QTextCodec::codecForLocale();
		
	Q_ASSERT(codec);
		
	currentStatus = offline;
	findUserWindowOpen = false;
	multipleSendingOpen = false;
	privacyListWindowOpen = false;
	waitForIconUpload = false;
	movingBuddy = false;
	avatarModified = false;
	iconObject.itemId = 0;
	currentContextGroup = 0;
	currentContextMenu = 0;
	allGroupCount = 0;
	allContactCount = 0;
	allContactTmpCount = 0;
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");	
	iconPath = settings.fileName().section('/', 0, -2) + "/icqicons/";
	

	letMeAskYouAboutXstatusPlease = true;
	rootItem = new QTreeWidgetItem(contactList);
//	rootItem->setHidden(true);
			rootItem ->setText(1, icqUin);
//			rootItem ->setIcon(1,QIcon(":/icons/icqaccount.png"));
//			rootItem ->setIcon(2,QIcon(":/icons/icqaccount.png"));
			rootItem ->setIcon(0,QIcon(":/icons/qutim.png"));
			rootItem->setTextAlignment(0, Qt::AlignRight);

			rootItem->setTextAlignment(2, Qt::AlignRight);


//			rootItem->setExpanded(true);

//			contactList->header()->setResizeMode(1, QHeaderView::ResizeToContents);
//			rootItem ->setIcon(1,QIcon(":/icons/icqaccount.png"));
//			rootItem->setText(0,"ttt");
//			rootItem->setHidden(true);
//			treeBuddyItem *buddy = new treeBuddyItem(this, rootItem);
//							buddyList.insert("12345", buddy);
//							buddy->setBuddyUin("12345");
			
//			createContact();
			
			historyObject = new historyXML(settings.fileName().section('/', 0, -2) + "/history");
			loadSettings();
			createContactList();
			if ( rootItem->childCount())
				rootItem->setExpanded(expandRoot);
			loadUnreadedMessages();

			buddyConnection = new buddyPicture(icqUin, this);
			
			connect ( buddyConnection, SIGNAL(emptyAvatarList()),this, SLOT(emptyAvatarList()));
			
			connect ( buddyConnection, SIGNAL(updateAvatar(const QString &, QByteArray)),
					this, SLOT(updateAvatar(const QString &, QByteArray)));
			avatarPort = 0;
			
			fileTransferObject = new FileTransfer(icqUin, this);
			
			connect ( fileTransferObject, SIGNAL(sendFile(QByteArray &, QByteArray &, QByteArray &)),
								this, SLOT(sendFile(QByteArray &, QByteArray &, QByteArray &)));
			
			
			connect ( fileTransferObject, SIGNAL(emitCancelSending(QByteArray &)),
								this, SLOT(sendCancelSending(QByteArray &)));
			
			connect ( fileTransferObject, SIGNAL(sendRedirectToProxy(const QByteArray &)),
								this, SLOT(redirectToProxy(const QByteArray &)));
			
			connect ( fileTransferObject, SIGNAL(emitAcceptSending(const QByteArray &)),
								this, SLOT(sendAcceptMessage(const QByteArray &)));
			
			createContactListActions();

        createSoundEvents();
}

contactListTree::~contactListTree()
{
	
	
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	QStringList chatWithList;
	foreach(chatWindow *win, chatWindowList)
	{
		chatWithList<<chatWindowList.key(win);
		delete win;
	}
	chatWindowList.clear();
	settings.setValue("list/chatwindow", chatWithList);
	
        qDeleteAll(historyList);
	delete historyObject;
	
	if ( findUserWindowOpen )
			delete searchWin;
	
	if ( multipleSendingOpen )
		delete multipleSendingWin;
	if (privacyListWindowOpen)
		delete privacyWindow;
	
        qDeleteAll(infoWindowList);
	infoWindowList.clear();
	
        qDeleteAll(awayMessageList);
	awayMessageList.clear();
	
	if ( currentContextMenu)
	{
		delete menuLabel;
		delete currentContextMenu;
	}
	
	qDeleteAll(noteWidgetsList);
	noteWidgetsList.clear();
}	



void contactListTree::createContact(bool last)
{	
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");

	Q_ASSERT(socket);
	if (!socket)
		return ;

	socket->read(1);
	quint16 arraySize = byteArrayToInt16(socket->read(2));
	QStringList groups = settings.value("list/group").toStringList();
	QStringList buddies = settings.value("list/contacts").toStringList();

	Q_ASSERT(contactList);
	if (contactList)
		contactList->setUpdatesEnabled(false);

	for ( quint16 i = 0; i < arraySize; i++ )
	{
		quint16 stringLength = byteArrayToInt16(socket->read(2));
		QString itemName = QString::fromUtf8(socket->read(stringLength));
		quint16 groupID = byteArrayToInt16(socket->read(2));
		quint16 itemId = byteArrayToInt16(socket->read(2));
		quint16 itemType = byteArrayToInt16(socket->read(2));
		
		if ( itemType == 0x0001 && groupID != 0x0000 )
		{
			settings.beginGroup(QString::number(groupID));
			if ( !groupList.contains(groupID))
			{
				treeGroupItem *group = new treeGroupItem(this, rootItem);
				group->setOnOffLists();
				group->userCount = 1;
				group->setExpanded(true);
				group->userCount = 0;
				group->onlineList->setHidden(true);
				group->offlineList->setHidden(true);
	//			group->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
				groupList.insert(groupID, group);
				group->setGroupText(itemName);
				quint16 groupLength = byteArrayToInt16(socket->read(2));
				group->readData(socket, groupLength);
				group->setCustomFont(grpFont.fontFamily, grpFont.fontSize, grpFont.fontColor);
				settings.setValue("id", groupID);
				contactListChanged = true;
				allContactCount += group->userCount;
//				if ( ! showGroups )
//					group->setHidden(true);
			} else {
				if ( groupList.value(groupID) && itemName != groupList.value(groupID)->name)
					contactListChanged = true;

				Q_ASSERT(groupList.value(groupID));
				if (groupList.value(groupID))
				{
					groupList.value(groupID)->setGroupText(itemName);
					quint16 groupLength = byteArrayToInt16(socket->read(2));
					groupList.value(groupID)->readData(socket, groupLength);
					allContactCount += groupList.value(groupID)->userCount;
					getGroups.removeAll(QString::number(groupID));
				}
			}

			settings.setValue("name", itemName);
			settings.endGroup();
			if ( !groups.contains(QString::number(groupID)))
				groups<<QString::number(groupID);

		}else if ( itemType == 0x0001 && !groupID )
		{
			quint16 tmpLength = byteArrayToInt16(socket->read(2));
			tlv rootTlv;
			
			for ( ;tmpLength > 0; )
			{
				tlv tmpTlv;
				tmpTlv.readData(socket);

				tmpLength -= tmpTlv.getLength();
				
				if ( tmpTlv.getTlvType() == 0x00c8)
					rootTlv = tmpTlv;
			}
			allGroupCount = rootTlv.getTlvLength() / 2;

		}
		else if (itemType == 0x0000 )
		{
			allContactTmpCount++;
			idBuddyList.append(itemId);
			settings.beginGroup(itemName);
			if ( !buddyList.contains(itemName))
			{
				if ( treeGroupItem *group = groupList.value(groupID) )
				{
					treeBuddyItem *buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(),
							this, group->offlineList);
					
					buddy->itemId = itemId;
					initializeBuddy(buddy);
					buddy->underline = !dontUnderlineNotAutho;
					buddy->groupID = groupID;
					buddy->groupName = group->name;
					buddyList.insert(itemName, buddy);
					buddy->setBuddyUin(itemName);
					quint16 buddyLength = byteArrayToInt16(socket->read(2));
					buddy->readData(socket, buddyLength);
					settings.setValue("nickname", buddy->getName());
					settings.setValue("authorized", !buddy->notAutho);
					buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
					contactListChanged = true;
					if (itemName == buddy->getName())
						requestUinInformation(itemName);
				}
			} else {
				quint16 buddyLength = byteArrayToInt16(socket->read(2));
				treeBuddyItem *tmpBuddy = buddyList.value(itemName);
				tmpBuddy->readData(socket, buddyLength);
				tmpBuddy->itemId = itemId;
				
				if (tmpBuddy->getName() != itemName)
					settings.setValue("nickname", tmpBuddy->getName());
//				else
//					requestUinInformation(itemName);
				settings.setValue("authorized", !tmpBuddy->notAutho);
				if ( groupID != tmpBuddy->groupID  && groupList.contains(tmpBuddy->groupID))
				{
					if (showGroups)
					{
						treeGroupItem *oldGrp =  groupList.value(tmpBuddy->groupID);

						Q_ASSERT(oldGrp);
						Q_ASSERT(oldGrp->offlineList);
						if (oldGrp && oldGrp->offlineList)
						{
							oldGrp->offlineList->removeChild(tmpBuddy);
							oldGrp->offlineList->sortChildren(1,Qt::AscendingOrder);
							if (!oldGrp)
							{
								oldGrp->userCount--;
								oldGrp->updateText();
							}
						}

						treeGroupItem *newGrp = groupList.value(groupID);

						Q_ASSERT(newGrp);
						Q_ASSERT(newGrp->offlineList);
						if (newGrp && newGrp->offlineList)
						{
							newGrp->offlineList->addChild(tmpBuddy);	
							newGrp->offlineList->sortChildren(1,Qt::AscendingOrder);
						}

						settings.setValue("groupid", groupID);
						
					}
					contactListChanged = true;
				}

			}

			getBuddies.removeAll(itemName);
			settings.setValue("name", itemName);
			settings.setValue("groupid", groupID);
			settings.endGroup();

			if ( ! buddies.contains(itemName))
				buddies<<itemName;

		} else if (itemType == 0x000E)
		{
			quint16 tmpLength = byteArrayToInt16(socket->read(2));
			socket->read(tmpLength);
			ignoreList<<itemName;
			
			itemFromList object;
			object.groupId = groupID;
			object.itemId = itemId;
			ignoreObjectList.insert(itemName, object);
			
		} else if ( itemType == 0x0002)
		{
			quint16 tmpLength = byteArrayToInt16(socket->read(2));
			socket->read(tmpLength);
			visibleList<<itemName;
			
			itemFromList object;
			object.groupId = groupID;
			object.itemId = itemId;
			visibleObjectList.insert(itemName, object);
			
		} else if ( itemType == 0x0003 )
		{
			quint16 tmpLength = byteArrayToInt16(socket->read(2));
			socket->read(tmpLength);
			invisibleList<<itemName;
			
			itemFromList object;
			object.groupId = groupID;
			object.itemId = itemId;
			invisibleObjectList.insert(itemName, object);
		}
		else if ( itemType ==  0x0004)
		{
			pdInfoID = itemId;
			pdInfoGroupId = groupID;
			quint16 tmpLength = byteArrayToInt16(socket->read(2));
			socket->read(tmpLength);
		} else if ( itemType == 0x0014)
		{
			
			iconObject.groupId = groupID;
			iconObject.itemId = itemId;
			
			quint16 tmpLength = byteArrayToInt16(socket->read(2));
			bool iconGetted = false;
			tlv iconTlv;
			
			for ( ;tmpLength > 0; )
			{
				tlv tmpTlv;
				tmpTlv.readData(socket);

				tmpLength -= tmpTlv.getLength();
				
				if ( tmpTlv.getTlvType() == 0x00d5)
				{
					iconGetted = true;
					iconTlv = tmpTlv;
				}
			}
			
			
			if ( iconGetted )
			{
				
				checkForOwnIcon(iconTlv.getTlvData());
			} else {
				removeIconHash();
			}
		}
		else {
			quint16 tmpLength = byteArrayToInt16(socket->read(2));
			socket->read(tmpLength);
		}
		
	}

// W00T W00T W00T
//	if ((allGroupCount == (groupList.count() - 1)) && (allContactCount == allContactTmpCount))
//		last = true;
	
	socket->read(4);
	if ( last )
	{
		
		foreach(QString name, getGroups)
		{
			getGroups.removeAll(name);
			groups.removeAll(name);
			settings.remove(name);

			Q_ASSERT(groupList.value(name.toShort())->offlineList);
			if (!groupList.value(name.toShort())->offlineList)
				continue;

			groupList.value(name.toShort())->offlineList->takeChildren();
			treeGroupItem *tmpItem = groupList.value(name.toShort());
			delete tmpItem;
			tmpItem = NULL;

			groupList.remove(name.toShort());
			contactListChanged = true;
			
			QStringList buddiesList = settings.value("list/contacts").toStringList();
					
					foreach(treeBuddyItem *buddy, buddyList)
					{
						Q_ASSERT(buddy);
						if (!buddy)
							continue;

						if ( buddy->groupID == name.toShort())
						{
							buddiesList.removeAll(buddy->getUin());
							getBuddies.removeAll(buddy->getUin());
							settings.remove(buddy->getUin());
						}
					}
					
			settings.setValue("list/contacts", buddiesList);
			
			
		}
		getGroups = settings.value("list/group").toStringList();
		
		foreach(QString uin, getBuddies)
		{

			getBuddies.removeAll(uin);
			treeBuddyItem *nilBuddy = buddyList.value(uin);
			
			Q_ASSERT(nilBuddy);
			if ( nilBuddy && nilBuddy->groupID)
			{
				Q_ASSERT(groupList.value(nilBuddy->groupID)->offlineList);
				if ( groupList.contains(nilBuddy->groupID) && groupList.value(nilBuddy->groupID)->offlineList)
					groupList.value(nilBuddy->groupID)->offlineList->removeChild(nilBuddy);

				nilBuddy->groupID = 0;
				treeGroupItem *newGrp = groupList.value(0);
				newGrp->offlineList->addChild(nilBuddy);
				newGrp->offlineList->sortChildren(1,Qt::AscendingOrder);
				newGrp->userCount++;
				newGrp->updateText();
				settings.setValue(uin + "/groupid", 0);
				contactListChanged = true;
			}
		}
		getBuddies = settings.value("list/contacts").toStringList();
		if ( contactListChanged  && isMergeAccounts )
			emit reupdateList();
		
		
		if (showGroups)
			foreach( treeGroupItem *group, groupList)
			{
				Q_ASSERT(group);
				if (!group)
					continue;

				if (group->userCount > 0)
				{
					if ( group->offlineList->childCount() )
					{
						if ( !showOffline )
							group->offlineList->setHidden(false);	
							group->offlineList->setExpanded(true);
					}
										//				newGroup2->setExpanded(true);
					if ( showGroups && !group->onlineList->childCount() && showOffline && hideEmpty)
						group->setHidden(true);	
				}

				Q_ASSERT(group->offlineList);
				if ( group->offlineList && !showOffline )
					group->offlineList->sortChildren(1,Qt::AscendingOrder);

			}
		
		
		Q_ASSERT(rootItem);
		if ( rootItem && rootItem->childCount() && !rootItem->isExpanded() )
			rootItem->setExpanded(expandRoot);
		
		
		
		
		emit incSnacSeq();
		servicesSetup privacySetup(icqUin);
		privacySetup.flap1309seq = *flapSeq;
		privacySetup.snac1309seq = *snacSeq;
		privacySetup.setPrivacy(icqUin, pdInfoID, pdInfoGroupId, tcpSocket);
		emit incFlapSeq();
		
		
		emit incSnacSeq();
		emit incMetaSeq();
		metaInformation metaInfo(icqUin);
		metaInfo.sendShortInfoReq(tcpSocket, *flapSeq, *snacSeq, *metaSeq, icqUin);
		mineMetaSeq = ((*metaSeq )% 0x100) * 0x100 + ((*metaSeq )/ 0x100);
		waitForMineInfo = true;
		emit incFlapSeq();
		
		sendReqForRedirect();
		
		setHideSeparators(hideSeparators);
		if ( rootItem && rootItem->childCount() && !rootItem->isExpanded() )
			rootItem->setExpanded(expandRoot);
		
		
		justStarted = true;
		QTimer::singleShot(3000, this, SLOT(disableJustStarted()));
		
		settings.setValue("list/ignore", ignoreList);
		settings.setValue("list/visible", visibleList);
		settings.setValue("list/invisible", invisibleList);
		
		if ( ignoreList.count() || visibleList.count() || invisibleList.count() )
		{
			//Q_ASSERT(privacyWindow);
			if ( privacyListWindowOpen && privacyWindow)
				privacyWindow->createLists();
		}
		allGroupCount = 0;
		allContactCount = 0;
		
		fileTransferObject->connectionProxy = tcpSocket->proxy();
		
	}
	settings.setValue("list/group", groups);
	settings.setValue("list/contacts", buddies);
	if ( !showGroups )
		hideEmptyGroups(true);

	Q_ASSERT(contactList);
	if (contactList)
		contactList->setUpdatesEnabled(true);
}


void contactListTree::removeContactList()
{

	delete rootItem;
}

quint16 contactListTree::byteArrayToInt16(const QByteArray &array)
{
	bool ok;
	return array.toHex().toUInt(&ok,16);	
}

quint32 contactListTree::byteArrayToInt32(const QByteArray &array)
{
	bool ok;
	return array.toHex().toULong(&ok,16);	
}

void contactListTree::oncomingBuddy(const QString &uin, quint16 length)
{

	bool fromOffline = false;
	if ( treeBuddyItem *buddy = buddyList.value(uin) )
	{
		
		buddy->xStatusCaption.clear();
		buddy->xStatusMsg.clear();
		treeGroupItem *group = groupList.value(buddy->groupID);
		
		Q_ASSERT(group);
		if (group && buddy && buddy->isOffline )
		{
			
			fromOffline = true;
			contactList->setUpdatesEnabled(false);
			if ( !isMergeAccounts )
			{
				Q_ASSERT(group->offlineList);
				Q_ASSERT(group->onlineList);
				if ( showGroups && group->offlineList && group->onlineList )
				{
					group->offlineList->removeChild(buddy);
					group->onlineList->addChild(
							buddy);

					
					group->offlineList->sortChildren(1,Qt::AscendingOrder);
					
					
					if ( !group->onlineList->childCount() )
						group->onlineList->setHidden(true);
						else
					{
						group->onlineList->setHidden(false);
						group->onlineList->setExpanded(true);
						if ( showOffline && hideEmpty)
							group->setHidden(false);
					}
							
					if ( !group->offlineList->childCount() )
						group->offlineList->setHidden(true);	
					else
					{
						if ( !showOffline )
							group->offlineList->setHidden(false);
							group->offlineList->setExpanded(true);
					}
					
									}
									else
									{
					offlineList->removeChild(buddy);
					onlineList->addChild(buddy);
					if ( !onlineList->childCount() )
									onlineList->setHidden(true);
							else
							{
								onlineList->setHidden(false);
								onlineList->setExpanded(true);
							}
							
							if ( !offlineList->childCount() )
									offlineList->setHidden(true);	
							else
							{
								if ( !showOffline )
									offlineList->setHidden(false);
								offlineList->setExpanded(true);
							}
				}
			}
			contactList->setUpdatesEnabled(true);
			if ( isMergeAccounts )
				emit buddyChangeStatus(buddy,false);
//			if ( showGroups )
				group->buddyOnline();
				buddy->setCustomFont(onlFont.fontFamily, onlFont.fontSize, onlFont.fontColor);
		
				if ( saveServiceHistory && onlineNotify)
				{
					historyObject->saveServiceMessage(buddy->getName(), QDateTime::currentDateTime(), 0);
				}
		
		}
		buddy->oncoming(socket, length);
		identifyContactClient.addContactClientId(buddy);
		buddy->checkForXStatus();
				
		if ( buddy->xStatusPresent )
			xStatusTickList.append(buddy);
				
		if ( letMeAskYouAboutXstatusPlease && buddy->xStatusPresent)
		{
			letMeAskYouAboutXstatusPlease = false;
			
			askForXstatusTimerTick();
		}


		
		
		
		if ( !showXStatuses)
			buddy->waitingForAuth(buddy->authorizeMe);
		if ( !isMergeAccounts )
		{
			if ( showGroups )
				group->updateOnline();
			else {
				contactList->setUpdatesEnabled(false);
				onlineList->sortChildren(1,Qt::AscendingOrder);
				contactList->setUpdatesEnabled(true);
			}
		}
		
		if ( buddy->statusChanged )
		{
			updateChatBuddyStatus(buddy->getUin(),
				(statusIconClass::getInstance()->*(buddy->statusIconMethod))());
		
			if (!justStarted)
		{
				// play sound
				emit playSoundEvent(static_cast<SoundEvent::Events>(buddy->getStatus()),
					currentStatus);

			if ( fromOffline && signOnNot )
			{
			if ( !contactList->hasFocus() ) 
				userMessage(uin, buddy->getName(), convertToStringStatus(buddy->getStatus()), statusNotyfication, true);
			setServiceMessageToWin(buddy->getUin(), convertToStringStatus(buddy->getStatus()));
			}
			
			if ( !fromOffline && changeStatusNot && !contactList->hasFocus())
			{
				userMessage(uin, buddy->getName(), convertToStringStatus(buddy->getStatus()), statusNotyfication, true);
			}
			
		}
		}

		emit updateOnlineList();	
		
		if ( buddy->avatarMd5Hash.length() != 16 )
		{
			QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
			contacts.remove(uin + "/iconhash");
		}
		

		
		if (! disableAvatars)
			askForAvatars(buddy->avatarMd5Hash, uin);
		if ( chatWindowList.contains(uin))
			chatWindowList.value(uin)->setContactClient(buddy->clientId);
	}
	else 
	{
		socket->read(length);
	}
}

void contactListTree::offlineBuddy(const QString &uin, quint16 length)
{
	if ( treeBuddyItem *buddy = buddyList.value(uin) )
	{
			if ( !buddy->isOffline )
			{
                        // play sound
                        playSoundEvent(SoundEvent::ContactOffline, currentStatus);

				treeGroupItem *group = groupList.value(buddy->groupID);
				if ( ! isMergeAccounts )
				{
				if (showGroups )
				{
					group->onlineList->removeChild(buddy);
				
					group->offlineList->addChild(buddy);
				
				
				if ( !group->onlineList->childCount() )
				{
							group->onlineList->setHidden(true);
							if ( showOffline && hideEmpty)
								group->setHidden(true);
				}
						else
						{
							group->onlineList->setHidden(false);
							group->onlineList->setExpanded(true);
						}
						
						if ( !group->offlineList->childCount() )
							group->offlineList->setHidden(true);	
						else
						{
							if ( !showOffline )
								group->offlineList->setHidden(false);
							group->offlineList->setExpanded(true);
						}
                                }
                                else
                                {
					onlineList->removeChild(buddy);
					offlineList->addChild(buddy);
					if ( !onlineList->childCount() )
									onlineList->setHidden(true);
							else
							{
								onlineList->setHidden(false);
								onlineList->setExpanded(true);
							}
							
							if ( !offlineList->childCount() )
									offlineList->setHidden(true);	
							else
							{
								if ( !showOffline )
									offlineList->setHidden(false);
								offlineList->setExpanded(true);
							}
					
				}
				}
				
				if( signOffNot && !contactList->hasFocus() && !justStarted)
					userMessage(uin, buddy->getName(), tr(" is offline"), statusNotyfication, true);
				setServiceMessageToWin(buddy->getUin(), tr("is offline"));
				buddy->buddyOffline();
				updateChatBuddyStatus(buddy->getUin(), (statusIconClass::getInstance()->*(buddy->statusIconMethod))());
//				if ( showGroups )
					group->buddyOffline();
				if ( !showGroups && !isMergeAccounts)
					offlineList->sortChildren(1,Qt::AscendingOrder);
				if ( isMergeAccounts )
					emit buddyChangeStatus(buddy,true);
				
				buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
				if ( saveServiceHistory && offlineNotify)
                                historyObject->saveServiceMessage(buddy->getName(),
                                        QDateTime::currentDateTime(), 1);
			}
			if ( chatWindowList.contains(uin))
				chatWindowList.value(uin)->setContactClient(buddy->clientId);	
			
	}	
	socket->read(length);
}

QString contactListTree::convertToStringStatus(contactStatus status)
{
	switch ( status )
		{
		case contactOnline:
			return tr("is online");
		case contactAway:
			return tr("is away");
		case contactDnd:
			return tr("is dnd");
		case contactNa:
			return tr("is n/a");
		case contactOccupied:
			return tr("is occupied");
		case contactFfc:
			return tr("is free for chat");
		case contactInvisible:
			return tr("is invisible");
		case contactOffline:
			return tr("is offline");
		case contactAtHome:
			return tr("at home");
		case contactAtWork:
			return tr("at work");
		case contactLunch:
			return tr("having lunch");
		case contactEvil:
			return tr("is evil");
		case contactDepression:
			return tr("in depression");
		default:
			return tr("is online");
		}
}

void contactListTree::getMessage(quint16 l)
{
	icqMessage newMessage(codepage);
	newMessage.readData(socket, l);
	

	if ( newMessage.fileAnswer)
	{

		if ( !buddyList.contains(newMessage.from))
			return;
		if( !newMessage.reason  )
			fileTransferObject->requestToRedirect(newMessage.from, newMessage.msgCookie, newMessage.connectToPeer,
					newMessage.peerIP, newMessage.peerPort, buddyList.value(newMessage.from)->getName(),
					newMessage.fileName, newMessage.fileSize, newMessage.aolProxyIP);
		if ( newMessage.reason == 0x0001)
			fileTransferObject->contactCanceled(newMessage.from, newMessage.msgCookie);
		if ( newMessage.reason == 0x0002)
			fileTransferObject->contactAccept(newMessage.from, newMessage.msgCookie);
	}
	
                if (!buddyList.contains(newMessage.from))
                {
    if (getOnlyFromContactList && ( !newMessage.messageType || newMessage.messageType == 4) )
	{
		if ( notifyAboutBlocked)
			notifyBlockedMessage(newMessage.from, newMessage.msg);
		
		if ( saveServiceHistory )
			saveBlocked(newMessage.from, newMessage.msg, QDateTime::currentDateTime());
		
		return;
	}
	
                        if (blockAuth && newMessage.messageType == 2) return;
	
                        if (blockUrlMessage && checkMessageForUrl(newMessage.msg)
                                || newMessage.messageType == 3)
		{
			if ( notifyAboutBlocked && ( !newMessage.messageType || newMessage.messageType == 4) )
						notifyBlockedMessage(newMessage.from, newMessage.msg);
			return;
		}
	
                        if (enableAntiSpamBot && ( !newMessage.messageType || newMessage.messageType == 4) )
                                if (turnOnAntiSpamBot(newMessage.from, newMessage.msg,
                                        QDateTime::currentDateTime())) return;
	}
	
	if ( newMessage.messageType == 0 )
	{
		bool playSound = true;
		messageFormat *msg = new messageFormat;
		msg->fromUin = newMessage.from;
		
		treeBuddyItem *tmpBuddy = 0;
		if ( (tmpBuddy = buddyList.value(msg->fromUin)) )
		{

			msg->from = tmpBuddy->getName();
			
			if ( saveHistory )
				historyObject->saveHistoryMessage(tmpBuddy->getUin(), 
						tmpBuddy->getName(), QDateTime::currentDateTime(), true, newMessage.msg);
                }
                else
                {
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
			
			treeGroupItem *group = groupList.value(0);
			msg->from = msg->fromUin;
			treeBuddyItem *buddy;
			if ( showGroups )
				buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, group->offlineList);
			else
				buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, offlineList);
			initializeBuddy(buddy);
			buddy->underline = !dontUnderlineNotAutho;
			buddy->groupID = 0;
			buddy->groupName = group->name;
			group->userCount++;
			group->updateText();
			buddyList.insert(msg->fromUin, buddy);
			buddy->setBuddyUin(msg->fromUin);
			buddy->setName(msg->fromUin);
			buddy->updateBuddyText();
			buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
			if ( showGroups )
				updateNil();
			requestUinInformation(buddy->getUin());
			settings.beginGroup(buddy->getUin());
			settings.setValue("name", buddy->getUin());
			settings.setValue("groupid", 0);
			settings.setValue("nickname", buddy->getName());
			settings.endGroup();
			
			QStringList contacts = settings.value("list/contacts").toStringList();
			contacts<<buddy->getUin();
			settings.setValue("list/contacts", contacts);
			
			if ( contactListChanged  && isMergeAccounts )
						emit reupdateList();
			
			if ( saveHistory && saveNilHistory )
			historyObject->saveHistoryMessage(buddy->getUin(), 
					buddy->getName(), QDateTime::currentDateTime() , true, newMessage.msg);
			
		}
			
		msg->message = newMessage.msg;
		
		msg->date = QDateTime::currentDateTime();
		
		if ( chatWindowList.contains(msg->fromUin))
		{
			chatWindow *w = chatWindowList.value(msg->fromUin);
			w->setMessage(msg->from,msg->message, msg->date);
			if ( !disableButtonBlinking)
				qApp->alert(w, 0);
			if ( tabMode )
				generalChatWindow->setMessageTab(w);
			if ( !w->isActiveWindow() )
			{

				if ( messageList.contains(msg->fromUin))
				{
					messageList.value(msg->fromUin)->messageList.append(msg);
				} else {
					messageList.insert(msg->fromUin, buddyList.value(msg->fromUin));
					messageList.value(msg->fromUin)->messageList.append(msg);
				}
				if ( !newMessages )
				{
				newMessages = true;
				QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
				}
				if ( !disableTrayBlinking )
					emit getNewMessage();
				if ( ! dontShowEvents )
				{
					if ( !dontShowIncomingMessagesInTrayEvents)
						emit userMessage(msg->fromUin, msg->from, msg->message, messageNotification, true);
					else
						emit userMessage(msg->fromUin, msg->from, tr("New message"), messageNotification, true);
				}
			}
                                else if (!sounds->playIfChatWindowIsActive())
                                        playSound = false;
                        }
                        else
                        {
			if ( messageList.contains(msg->fromUin))
			{
				messageList.value(msg->fromUin)->messageList.append(msg);
			} else {
				messageList.insert(msg->fromUin, buddyList.value(msg->fromUin));
				messageList.value(msg->fromUin)->messageList.append(msg);
			}
			if ( !dontShowIncomingMessagesInTrayEvents)
				emit userMessage(msg->fromUin, msg->from, msg->message, messageNotification, true);
			else
				emit userMessage(msg->fromUin, msg->from, tr("New message"), messageNotification, true);
			if ( !disableTrayBlinking )
				emit getNewMessage();
		}
		if ( !newMessages )
		{
			newMessages = true;
			QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
		}
		
		if ( openNew )
			doubleClickedBuddy(buddyList.value(msg->fromUin));
		if (playSound)
		{
                      // play sound
                      emit playSoundEvent(SoundEvent::MessageGet, currentStatus);
		};
        }
        else if ( newMessage.messageType == 1 )
	{
		if ( buddyList.contains(newMessage.from))
		{
//			emit userMessage(tr("%1 is reading your away message").arg(buddyList.value(newMessage.from)->getName()));
			if ( awayNot )
                                emit userMessage(newMessage.from, buddyList.value(newMessage.from)->getName(),
                                        "", readNotification, true);
			setServiceMessageToWin(buddyList.value(newMessage.from)->getUin(), tr("is reading your away message"));
			
			emit incSnacSeq();	
			
			newMessage.sendAutoreply(tcpSocket, getCurrentAwayMessage(),*flapSeq, *snacSeq );
			emit incFlapSeq();
			
			if ( saveServiceHistory )
                                historyObject->saveServiceMessage(buddyList.value(newMessage.from)->getName(),
                                        QDateTime::currentDateTime(), 2);
			
		}
		else
		{
//			emit userMessage(tr("%1 is reading your away message ( not from list )").arg(newMessage.from));
			if ( awayNot )
				emit userMessage("", newMessage.from, "", readNotification, false);
			if ( saveServiceHistory && readAwayNotify)
                                historyObject->saveServiceMessage(newMessage.from,
                                        QDateTime::currentDateTime(), 2);
			}
		}
        else if (newMessage.messageType == 4)
	{
		bool playSound = true;
		messageFormat *msg = new messageFormat;
		msg->fromUin = newMessage.from;
		
		
		if ( buddyList.contains(msg->fromUin) )
		{
			treeBuddyItem *buddy = buddyList.value(msg->fromUin);
			msg->from = buddy->getName();
			
			if ( saveHistory )
				historyObject->saveHistoryMessage(buddy->getUin(), 
						buddy->getName(), QDateTime::currentDateTime(), true, newMessage.msg);
                }
                else
                {
			
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
			
			treeGroupItem *group = groupList.value(0);
			msg->from = msg->fromUin;
			treeBuddyItem *buddy;
			if ( showGroups )
				buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, group->offlineList);
			else
				buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, offlineList);
			initializeBuddy(buddy);
			buddy->underline = !dontUnderlineNotAutho;
			buddy->groupID = 0;
			buddy->groupName = group->name;
			group->userCount++;
			group->updateText();
			buddyList.insert(msg->fromUin, buddy);
			buddy->setBuddyUin(msg->fromUin);
			buddy->setName(msg->fromUin);
			buddy->updateBuddyText();
			buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
			if ( showGroups )
				updateNil();
			requestUinInformation(buddy->getUin());
			settings.beginGroup(buddy->getUin());
			settings.setValue("name", buddy->getUin());
			settings.setValue("groupid", 0);
			settings.setValue("nickname", buddy->getName());
			settings.endGroup();
			
			QStringList contacts = settings.value("list/contacts").toStringList();
			contacts<<buddy->getUin();
			settings.setValue("list/contacts", contacts);
			
			if ( contactListChanged  && isMergeAccounts )
						emit reupdateList();
			
			if ( saveHistory && saveNilHistory )
			historyObject->saveHistoryMessage(buddy->getUin(), 
					buddy->getName(), QDateTime::currentDateTime() , true, newMessage.msg);
			
		}
			
		msg->message = newMessage.msg;
		
		msg->date = QDateTime::currentDateTime();
		
		if ( chatWindowList.contains(msg->fromUin))
		{
			chatWindow *w = chatWindowList.value(msg->fromUin);
			w->setMessage(msg->from,msg->message, msg->date);
			if ( !disableButtonBlinking)
				qApp->alert(w, 0);
			if ( tabMode )
				generalChatWindow->setMessageTab(w);
			if ( !w->isActiveWindow() )
			{

				if ( messageList.contains(msg->fromUin))
					messageList.value(msg->fromUin)->messageList.append(msg);
                                else
                                {
					messageList.insert(msg->fromUin, buddyList.value(msg->fromUin));
					messageList.value(msg->fromUin)->messageList.append(msg);
				}
				newMessages = true;
				QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
				if ( !disableTrayBlinking )
					emit getNewMessage();
				if ( ! dontShowEvents )
				{
					if ( !dontShowIncomingMessagesInTrayEvents)
                                                emit userMessage(msg->fromUin, msg->from, msg->message,
                                                        messageNotification, true);
					else
                                                emit userMessage(msg->fromUin, msg->from, tr("New message"),
                                                        messageNotification, true);
				}
			}
                        else if (!sounds->playIfChatWindowIsActive())
                                playSound = false;
                }
                else
                {
			if ( messageList.contains(msg->fromUin))
			{
				messageList.value(msg->fromUin)->messageList.append(msg);
                        }
                        else
                        {
				messageList.insert(msg->fromUin, buddyList.value(msg->fromUin));
				messageList.value(msg->fromUin)->messageList.append(msg);
			}
			if ( dontShowIncomingMessagesInTrayEvents )
                                emit userMessage(msg->fromUin, msg->from, msg->message,
                                        messageNotification, true);
			else
                                emit userMessage(msg->fromUin, msg->from, tr("New message"),
                                        messageNotification, true);

			if ( !disableTrayBlinking )
				emit getNewMessage();
		}
		if ( !newMessages )
		{
			newMessages = true;
			QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
		}
		
		if ( openNew )
			doubleClickedBuddy(buddyList.value(msg->fromUin));
		if (playSound)
		{
                        // play sound
                        emit playSoundEvent(SoundEvent::MessageGet, currentStatus);
		};
        }
        else if ( newMessage.messageType == 7)
	{
		if ( buddyList.contains(newMessage.from))
		{

			if ( awayNot )
                        emit userMessage(newMessage.from, buddyList.value(newMessage.from)->getName(),
                                "", xstatusReadNotification, true);
			setServiceMessageToWin(buddyList.value(newMessage.from)->getUin(), tr("is reading your xStatus message"));
			
			emit incSnacSeq();	
			
			newMessage.sendXstatusReply(tcpSocket, icqUin, *flapSeq, *snacSeq );
			emit incFlapSeq();
			
			if ( saveServiceHistory )
                        historyObject->saveServiceMessage(buddyList.value(newMessage.from)->getName(),
                                QDateTime::currentDateTime(), 2);
		}
		else
		{

			if ( awayNot )
				emit userMessage("", newMessage.from, "", xstatusReadNotification, false);
			if ( saveServiceHistory && readAwayNotify)
                                historyObject->saveServiceMessage(newMessage.from,
                                        QDateTime::currentDateTime(), 2);
		}
	}
	
}

void contactListTree::readMessageStack()
{
	foreach(treeBuddyItem *item, messageList)
	{
		readMessageFrom(item);
	}
}

void contactListTree::setMessageIconToContact()
{
	if ( !messageList.empty() )
	{
		foreach(treeBuddyItem *item, messageList)
		{
			if ( item->messageIcon )
			{
				contactList->setUpdatesEnabled(false);
				item->setIcon(0, statusIconClass::getInstance()->getContentIcon());
				contactList->setUpdatesEnabled(true);
			} else 
			{
				contactList->setUpdatesEnabled(false);
				item->setIcon(0, (statusIconClass::getInstance()->*(item->statusIconMethod))());
				contactList->setUpdatesEnabled(true);
			}
			item->messageIcon = !item->messageIcon;
		}
		QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
	} else {
		newMessages = false;
	}
}

void contactListTree::doubleClickedBuddy(treeBuddyItem *buddy)
{
	if ( buddy->authorizeMe )
	{
		openAuthReqFromBuddy(buddy);
		return;
	}
	
	
	if ( messageList.contains(buddy->getUin()) )
		readMessageFrom(buddy);
	else
		{
		if ( !chatWindowList.contains(buddy->getUin()))
			{
				chatWindow *winChat = new chatWindow();
				winChat->setUin(icqUin);
				winChat->setContactUin(buddy->getUin());
				connect( winChat, SIGNAL(destroyed ( QObject *)),
						this, SLOT(deleteChatWindow(QObject *)));
				connect( winChat, SIGNAL(sendMessage(const messageFormat &)),
						this, SLOT(sendMessage(const messageFormat &)));
				connect( winChat, SIGNAL(windowFocused(const QString &)),
										this, SLOT(activateWindow(const QString &)));
				winChat->setWindowTitle(buddy->getName());
				winChat->setWindowIcon((statusIconClass::getInstance()->*(buddy->statusIconMethod))());
				winChat->setAttribute(Qt::WA_QuitOnClose, false);
				winChat->setAttribute(Qt::WA_DeleteOnClose, true);
				chatWindowList.insert(buddy->getUin(), winChat);
				winChat->contactName = buddy->getName();
				initializeWindow(winChat);
				if ( tabMode ) {
					generalChatWindow->addChatWindow(winChat);
					generalChatWindow->raise();
					generalChatWindow->activateWindow();
					generalChatWindow->setFocus(Qt::OtherFocusReason);
					if ( generalChatWindow->isMinimized() )
						generalChatWindow->setWindowState(generalChatWindow->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
				} else {
					winChat->show();
					winChat->raise();
					winChat->activateWindow();
					winChat->setFocus(Qt::OtherFocusReason);
				}
				

			} else {
				chatWindow *w = chatWindowList.value(buddy->getUin());
//				if ( !w->isActiveWindow() )
//					{
//						if ( w->isMinimized() )
//							w->showNormal();
//						w->activateWindow();
//						if ( tabMode )
//							generalChatWindow->showWindow(w);
//					}
				
				if ( !w->isActiveWindow() )
				{
					if ( w->isMinimized() )
						 w->setWindowState(w->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);

				    w->raise();
				    w->activateWindow();
				    w->setFocus(Qt::OtherFocusReason);

					if ( tabMode ) {
				    	generalChatWindow->showWindow(w);
						generalChatWindow->raise();
						generalChatWindow->activateWindow();
						generalChatWindow->setFocus(Qt::OtherFocusReason);
					}
				 }
			}
		}
}

void contactListTree::readMessageFrom(treeBuddyItem *buddy)
{
	buddy->messageIcon = false;
	bool addMessageToWindow = true;
	if ( !chatWindowList.contains(buddy->getUin()))
	{
		chatWindow *winChat = new chatWindow();
		winChat->setUin(icqUin);
		winChat->setContactUin(buddy->getUin());
		connect( winChat, SIGNAL(destroyed ( QObject *)),
				this, SLOT(deleteChatWindow(QObject *)));
		connect( winChat, SIGNAL(sendMessage(const messageFormat &)),
				this, SLOT(sendMessage(const messageFormat &)));
		connect( winChat, SIGNAL(windowFocused(const QString &)),
				this, SLOT(activateWindow(const QString &)));
		winChat->setWindowIcon((statusIconClass::getInstance()->*(buddy->statusIconMethod))());
		winChat->setWindowTitle(buddy->getName());
		winChat->setAttribute(Qt::WA_QuitOnClose, false); 
		winChat->setAttribute(Qt::WA_DeleteOnClose, true);
		chatWindowList.insert(buddy->getUin(), winChat);
		winChat->contactName = buddy->getName();
		initializeWindow(winChat);
		if ( tabMode ) {
			generalChatWindow->addChatWindow(winChat);
			generalChatWindow->raise();
			generalChatWindow->activateWindow();
			generalChatWindow->setFocus(Qt::OtherFocusReason);
			if ( generalChatWindow->isMinimized() )
					generalChatWindow->setWindowState(generalChatWindow->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
		} else {
			winChat->show();
			winChat->raise();
			winChat->activateWindow();
			winChat->setFocus(Qt::OtherFocusReason);
		}
	} else {
			addMessageToWindow = false;

	}
	
	if ( showRecent )
		addMessageToWindow = false;
	chatWindow *w = chatWindowList.value(buddy->getUin());
	
	if ( !w->isActiveWindow() )
	{
		if ( w->isMinimized() )
			 w->setWindowState(w->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);

	    w->raise();
	    w->activateWindow();
	    w->setFocus(Qt::OtherFocusReason);

		if ( tabMode ) {
	    	generalChatWindow->showWindow(w);
			generalChatWindow->raise();
			generalChatWindow->activateWindow();
			generalChatWindow->setFocus(Qt::OtherFocusReason);
		}
	 }
	
	
	
	foreach(messageFormat *mesg, buddy->messageList)
	{
		if (addMessageToWindow )
			w->setMessage(mesg->from,mesg->message, mesg->date);
	}
	buddy->messageList.clear();
	buddy->setIcon(0, (statusIconClass::getInstance()->*(buddy->statusIconMethod))());
	messageList.remove(buddy->getUin());
	if ( messageList.empty() )
		emit readAllMessages();
}

void contactListTree::deleteChatWindow(QObject *obj)
{
	chatWindow *tempWindow = (chatWindow *)(obj);
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.setValue("chatwindow/size", tempWindow->size());
	chatWindowList.remove(chatWindowList.key(tempWindow));
}

void contactListTree::deleteHistoryWindow(QObject *obj)
{
	historyView *tempWindow = (historyView  *)(obj);
	historyList.remove(historyList.key(tempWindow));
}

void contactListTree::sendMessage(const messageFormat &msg)
{
	
	if ( buddyList.contains(msg.fromUin ) )
	{
		treeBuddyItem *buddy = buddyList.value(msg.fromUin);
		emit incSnacSeq();
		icqMessage message(codepage);
	
	if ( buddy->getStatus()!= contactOffline )
		message.sendMessage(tcpSocket, msg,*flapSeq, *snacSeq, buddyList.value(msg.fromUin)->UTF8);
	else
		message.sendMessage(tcpSocket, msg,*flapSeq, *snacSeq, false);
	emit incFlapSeq();
	
	messageCursorPositions.insert(message.msgCookie, msg.position);
	
	if ( saveHistory )
		historyObject->saveHistoryMessage(msg.fromUin, 
				accountNickname, QDateTime::currentDateTime() , false, msg.message);
	
                // play sound
                emit playSoundEvent(SoundEvent::MessageSend, currentStatus);
        }
}

void contactListTree::getOfflineMessage()
{
	socket->read(2);
	quint32 tempSenderUin = byteArrayToInt32(socket->read(4));
	quint32 senderUin = (tempSenderUin % 0x100) * 0x1000000 +
	(tempSenderUin % 0x10000 / 0x100) * 0x10000 + 
	(tempSenderUin % 0x1000000 / 0x10000) * 0x100 + 
	(tempSenderUin / 0x1000000);
	messageFormat *msg = new messageFormat;
	msg->fromUin = QString::number(senderUin, 10);
	bool messageFromList;
	
	
	
	
	bool ok;
		QDateTime offlineDateTime;
		quint16 year = (quint8)socket->read(1).toHex().toUShort(&ok, 16);
		year += ((quint8)socket->read(1).toHex().toUShort(&ok, 16) * 0x100);
		int month = (quint8)socket->read(1).toHex().toUShort(&ok, 16);
		int day = (quint8)socket->read(1).toHex().toUShort(&ok, 16);
		int hour = (quint8)socket->read(1).toHex().toUShort(&ok, 16);
		int min = (quint8)socket->read(1).toHex().toUShort(&ok, 16);
		QDate offlineDate(year, month, day);
		
		QTime offlineTime(hour, min);
		
		offlineDateTime.setDate(offlineDate);
		offlineDateTime.setTime(offlineTime);
		
		quint8 messageType = (quint8)socket->read(1).toHex().toUShort(&ok, 16);
		socket->read(1);
		quint16 tmpLength = byteArrayToInt16(socket->read(2));
		quint16 length = (tmpLength % 0x100) * 0x100 + tmpLength / 0x100;
		
		msg->message = codec->toUnicode(socket->read(length - 1));
		
		QDateTime curTime = QDateTime::currentDateTime();
		int offset = (curTime.toLocalTime().time().hour() - curTime.toUTC().time().hour()) * 3600 + 
			(curTime.toLocalTime().time().minute() - curTime.toUTC().time().minute()) * 60 - daylight * 3600;
		offlineDateTime = offlineDateTime.addSecs(offset);
		
		msg->date = offlineDateTime;
		socket->read(1);
	
	
	
		if ( !buddyList.contains(msg->fromUin) && getOnlyFromContactList)
			{
				if ( notifyAboutBlocked)
					notifyBlockedMessage(msg->fromUin, msg->message);
				
				if ( saveServiceHistory )
					saveBlocked(msg->fromUin, msg->message, msg->date);
				
				return;
			}
			
			if (!buddyList.contains(msg->fromUin) && blockAuth && messageType == 0x06)
			{
				return;
			}
			
			
			if (!buddyList.contains(msg->fromUin) && blockUrlMessage )
			{
				if ( checkMessageForUrl(msg->message) || messageType == 0x04)
				{
					if ( notifyAboutBlocked && messageType != 0x04 )
								notifyBlockedMessage(msg->fromUin, msg->message);
					return;
				}
			}
			
			if (!buddyList.contains(msg->fromUin) && enableAntiSpamBot )
			{
				if (turnOnAntiSpamBot(msg->fromUin, msg->message, msg->date))
					return;
			}
	
	
	
	
	
		if ( buddyList.contains(msg->fromUin) )
		{
			treeBuddyItem *buddy = buddyList.value(msg->fromUin);
			msg->from = buddy->getName();
			
			messageFromList = true;

			
			
		} else {
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
					
					treeGroupItem *group = groupList.value(0);
					msg->from = msg->fromUin;
					treeBuddyItem *buddy;
					if ( showGroups )
						buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, group->offlineList);
					else
						buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, offlineList);
					initializeBuddy(buddy);
					buddy->underline = !dontUnderlineNotAutho;
					buddy->groupID = 0;
					buddy->groupName = group->name;
					group->userCount++;
					group->updateText();
					buddyList.insert(msg->fromUin, buddy);
					buddy->setBuddyUin(msg->fromUin);
					buddy->setName(msg->fromUin);
					buddy->updateBuddyText();
					buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
					if ( showGroups )
						updateNil();
					requestUinInformation(buddy->getUin());
					settings.beginGroup(buddy->getUin());
					settings.setValue("name", buddy->getUin());
					settings.setValue("groupid", 0);
					settings.setValue("nickname", buddy->getName());
					settings.endGroup();
					
					QStringList contacts = settings.value("list/contacts").toStringList();
					contacts<<buddy->getUin();
					settings.setValue("list/contacts", contacts);
					
					if ( contactListChanged  && isMergeAccounts )
								emit reupdateList();
					
					messageFromList = false;
			
			
		}
	
	if ( messageType == 0x01 )
	{
		
		if ( chatWindowList.contains(msg->fromUin))
			{
			chatWindow *w = chatWindowList.value(msg->fromUin);	
			w->setMessage(msg->from,msg->message,offlineDateTime);
			if ( !disableButtonBlinking )	
				qApp->alert(w, 0);
				
				if ( !w->isActiveWindow() )
				{
					if ( messageList.contains(msg->fromUin))
					{
						messageList.value(msg->fromUin)->messageList.append(msg);
					} else {
						messageList.insert(msg->fromUin, buddyList.value(msg->fromUin));
						messageList.value(msg->fromUin)->messageList.append(msg);
					}
					newMessages = true;
					QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
					if ( !disableTrayBlinking )
						emit getNewMessage();
					if ( ! dontShowEvents )
						emit userMessage(msg->fromUin, msg->from, msg->message, messageNotification, true);
				}
				
			} else {
				if ( messageList.contains(msg->fromUin))
				{
					messageList.value(msg->fromUin)->messageList.append(msg);
				} else {
					messageList.insert(msg->fromUin, buddyList.value(msg->fromUin));
					messageList.value(msg->fromUin)->messageList.append(msg);
				}
				emit userMessage(msg->fromUin, msg->from, msg->message, messageNotification, true);
				if ( !disableTrayBlinking )
					emit getNewMessage();
			}
		
			if ( !newMessages )
			{
				newMessages = true;
				QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
			}	
			
	} 
	if ( openNew )
			doubleClickedBuddy(buddyList.value(msg->fromUin));	
	
	if ( messageFromList )
	{
		
		if ( saveHistory )
			historyObject->saveHistoryMessage(msg->fromUin, 
					buddyList.value(msg->fromUin)->getName(), msg->date, true, msg->message);
		
	} else {
		if ( saveHistory && saveNilHistory)
			historyObject->saveHistoryMessage(msg->fromUin, 
					buddyList.value(msg->fromUin)->getName(), msg->date, true, msg->message);
	}
	
	
//	msg->message = newMessage.msg;
}

QHash<quint16, treeGroupItem *> contactListTree::mergeAccounts(bool flag)
{

	rootItem->setHidden(flag);
	isMergeAccounts = flag;

	if ( !flag )
	{
		showHideGroups();
	} else {
		if ( !showGroups )
		{
			prepareForMerge();

		}
		emit sendGroupList(groupList);
	}

	return groupList;
	
}

void contactListTree::updateSorting()
{
	foreach( treeGroupItem *group, groupList)
	{
		group->updateOnline();
	}
}

void contactListTree::createContactList()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	getGroups = settings.value("list/group").toStringList();
	getBuddies = settings.value("list/contacts").toStringList();
	if (getGroups.size())
	{
	foreach(QString groupId, getGroups)
	{
		treeGroupItem *group = new treeGroupItem(this, rootItem);
		group->setOnOffLists();
		group->userCount = 1;
		group->setExpanded(true);
		group->userCount = 0;
		group->onlineList->setHidden(true);
		group->offlineList->setHidden(true);
		groupList.insert(groupId.toInt(), group);
		group->setGroupText(settings.value(groupId + "/name").toString());
		group->setCustomFont(grpFont.fontFamily, grpFont.fontSize, grpFont.fontColor);
		if (!showGroups )
			group->setHidden(true);
	}
	createNil();
	
	
	if ( showGroups )
		hideEmptyGroups(hideEmpty);
	
        // Days to birthday
        int birthTo;
	if ( getBuddies.size())
	{
		foreach(QString buddyUin, getBuddies)
		{
			if ( treeGroupItem *group = groupList.value(settings.value(buddyUin + "/groupid").toInt()) )
			{
					treeBuddyItem *buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, group->offlineList);
					initializeBuddy(buddy);
					settings.beginGroup(buddyUin);
					buddy->underline = !dontUnderlineNotAutho;
					buddy->groupID = settings.value("groupid").toInt();
					buddy->notAutho = !settings.value("authorized",true).toBool();
					buddy->birth = !hideBirth;
					buddy->birthDay = QDate(settings.value("birthyear", 0).toInt(),
							settings.value("birthmonth", 0).toInt(),
							settings.value("birthday", 0).toInt());

                                // if birthday is comming -> play sound
                                birthTo = QDate::currentDate().daysTo(buddy->birthDay);
                                if ((birthTo >= 0) && (birthTo <= 3))
                                // we don't check if birthday sound has already been played
                                // for today. SoundEvents class does it for us.
                                        emit playSoundEvent(SoundEvent::ContactBirthday, currentStatus);

					buddy->groupName = group->name;
					group->userCount++;
					group->updateText();
					buddyList.insert(buddyUin, buddy);
					buddy->setBuddyUin(buddyUin);
					buddy->setName(settings.value("nickname").toString());
					buddy->avatarMd5Hash = QByteArray::fromHex(settings.value("iconhash").toByteArray());
					buddy->updateBuddyText();
					buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
					settings.endGroup();
					
					
					if (group->userCount > 0)
					{
						if ( group->offlineList->childCount() )
						{
							if ( !showOffline )
								group->offlineList->setHidden(false);	
								group->offlineList->setExpanded(true);
						}
					//				newGroup2->setExpanded(true);
						if ( showGroups)
						{
							if ( hideEmpty && showOffline)
								group->setHidden(true);
							else
								group->setHidden(false);
						}
						
					}
					if ( !showOffline )
						group->offlineList->sortChildren(1,Qt::AscendingOrder);
					
					if ( showGroups )
						updateNil();
					
					
					
					
					
			}
		}
		
		if ( !showGroups )
		{
			createOnOffGroups();
										
			foreach(treeGroupItem *group, groupList)
				offlineList->addChildren(group->offlineList->takeChildren());
			if ( offlineList->childCount() )
			{
						if ( !showOffline )
				offlineList->setHidden(false);	
				offlineList->setExpanded(true);
			}
					if ( !showOffline )
							offlineList->sortChildren(1,Qt::AscendingOrder);
			
		}
	}
	}
	if ( clearNil )
		clearNilUsers();
	
	
	QStringList chatWithList = settings.value("list/chatwindow").toStringList();
	foreach(QString buddyUin, chatWithList)
	{
		if ( buddyList.contains(buddyUin ) )
			doubleClickedBuddy(buddyList.value(buddyUin));
	}
	settings.remove("list/chatwindow");
	
	// It seems there is something wrong with this call, contactSeparators were not created
	if ( hideSeparators )
		setHideSeparators(true);
}
	
void contactListTree::createSoundEvents()
{
	// create sound object
	sounds = new SoundEvents(icqUin, this);

	// connections
	connect(this, SIGNAL(playSoundEvent(const SoundEvent::Events &,
		const accountStatus &)), sounds,
		SLOT(play(const SoundEvent::Events &, const accountStatus &)));
	
	connect(this, SIGNAL(soundSettingsChanged()),
		sounds, SLOT(loadSettings()));
}

void contactListTree::createNil()
{
	treeGroupItem *group = new treeGroupItem(this, rootItem);
	group->setOnOffLists();
	group->userCount = 1;
	group->setExpanded(true);
	group->userCount = 0;
	group->onlineList->setHidden(true);
	group->offlineList->setHidden(true);
	groupList.insert(0, group);
	group->setGroupText(tr("Not in list"));
	group->setCustomFont(grpFont.fontFamily, grpFont.fontSize, grpFont.fontColor);
	if ( !showGroups )
		group->setHidden(true);
//	group->setHidden(true);
}

void contactListTree::updateNil()
{
	treeGroupItem *group = groupList.value(0);
	if (group->userCount > 0)
	{
		if ( group->offlineList->childCount() )
		{
			if ( !showOffline )
				group->offlineList->setHidden(false);	
			group->offlineList->setExpanded(true);
		}
						//				newGroup2->setExpanded(true);
		if ( showGroups)
		{
			if ( hideEmpty && showOffline)
				group->setHidden(true);
			else
				group->setHidden(false);
		}
	}
						if ( !showOffline )
	group->offlineList->sortChildren(1,Qt::AscendingOrder);
}

void contactListTree::goingOnline(bool iAmOnlineSignal)
{
	if ( iAmOnline = iAmOnlineSignal )
	{
		foreach(chatWindow *w, chatWindowList)
		w->setOnline(true);
		
		findUser->setEnabled(true);
		sendMultiple->setEnabled(true);
		changePassword->setEnabled(true);
		if ( privacyListWindowOpen)
			privacyWindow->setOnline(true);
//		privacyList->setEnabled(true);
		
	} else {
		fileTransferObject->disconnectFromAll();
		visibleList.clear();
		invisibleList.clear();
		ignoreList.clear();
		findUser->setEnabled(false);
		sendMultiple->setEnabled(false);
		changePassword->setEnabled(false);
		waitForIconUpload = false;
		
		if ( privacyListWindowOpen)
			privacyWindow->setOnline(false);
		
//		privacyList->setEnabled(false);
		foreach(treeBuddyItem *buddy, buddyList)
		{
			if ( !buddy->isOffline )
			{
				
				treeGroupItem *group = groupList.value(buddy->groupID);
				
				buddy->buddyOffline();
				updateChatBuddyStatus(buddy->getUin(), (statusIconClass::getInstance()->*(buddy->statusIconMethod))());
				buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
					group->buddyOffline();
					group->onlineList->setHidden(true);
				if ( isMergeAccounts )
					emit buddyChangeStatus(buddy,true);
			}
		}
		if ( showGroups )
		{				
			foreach(treeGroupItem *group, groupList)
			{
			group->offlineList->addChildren(group->onlineList->takeChildren());
			group->offlineList->sortChildren(1,Qt::AscendingOrder);
			if ( !group->onlineList->childCount() )
			{
				group->onlineList->setHidden(true);
				if ( showOffline && hideEmpty)
					group->setHidden(true);
			}
			else
			{
				group->onlineList->setHidden(false);
				group->onlineList->setExpanded(true);
			}
												
			if ( !group->offlineList->childCount() )
				group->offlineList->setHidden(true);	
			else
			{
				if ( !showOffline )
					group->offlineList->setHidden(false);
					group->offlineList->setExpanded(true);
			}
			}
		} else {
			offlineList->addChildren(onlineList->takeChildren());				
							contactList->setUpdatesEnabled(false);
							offlineList->sortChildren(1,Qt::AscendingOrder);
							contactList->setUpdatesEnabled(true);
							if ( !onlineList->childCount() )
								onlineList->setHidden(true);
							else
								{
									onlineList->setHidden(false);
									onlineList->setExpanded(true);
								}
												
								if ( !offlineList->childCount() )
									offlineList->setHidden(true);	
								else
								{
															if ( !showOffline )
								offlineList->setHidden(false);
								offlineList->setExpanded(true);
								}
						}
		foreach(chatWindow *w, chatWindowList)
			w->setOnline(false);
		buddyConnection->disconnectFromSST();
		avatarAddress.clear();
		avatarCookie.clear();
		
		avatartList.clear();
	}

	
}

void contactListTree::loadSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	accountNickname = settings.value("main/nick", icqUin).toString();
	disableAvatars = settings.value("connection/disavatars", false).toBool();
	settings.beginGroup("contactlist");
	showGroups = settings.value("showgroups", true).toBool();
	hideEmpty = settings.value("hideempty", false).toBool();
	expandRoot = settings.value("expanded", true).toBool();
	showOffline = settings.value("hideoff", false).toBool();
	hideSeparators =settings.value("hidesep", false).toBool();
	clearNil = settings.value("nilclear", false).toBool();
	dontUnderlineNotAutho = settings.value("auth", false).toBool();
	hideBirth = settings.value("birthday", false).toBool();
	customAccount = settings.value("acccustomf", false).toBool();
	customGroup = settings.value("grpcustomf", false).toBool();
	customOnline = settings.value("onlcustomf", false).toBool();
	customOffline = settings.value("offcustomf", false).toBool();
	
	if ( customAccount )
	{
		QFont font;
		font.setFamily(settings.value("acccustomfam").toString());
		int accSiz = settings.value("acccustomsizf", 9).toInt();
			accSiz = accSiz < 6 ? 6 : accSiz;
			accSiz = accSiz > 24 ? 24 : accSiz;
		
		font.setPointSize(accSiz);
		QColor accountCol;
		QVariant accCol= settings.value("acccustomcol");
		if ( accCol.canConvert<QColor>())
			accountCol = accCol.value<QColor>();
		
		rootItem->setFont(1,font);
		rootItem->setForeground(1,accountCol);
		
		
		
	}
	
	if ( customGroup )
	{
		grpFont.fontFamily = settings.value("grpcustomfam").toString();
		int grpSiz = settings.value("grpcustomsizf", 7).toInt();
		grpSiz = grpSiz < 6 ? 6 : grpSiz;
		grpSiz = grpSiz > 24 ? 24 : grpSiz;
		
		grpFont.fontSize = grpSiz;		
		
		QColor groupCol;
		QVariant grpCol= settings.value("grpcustomcol");
		if ( grpCol.canConvert<QColor>())
			groupCol = grpCol.value<QColor>();
			
		grpFont.fontColor = groupCol;
	} else {
		QFont font;
		grpFont.fontFamily = font.family();
		grpFont.fontSize = 7;
		QColor color;
		grpFont.fontColor = color;
	}
	
	if ( customOnline )
	{
		onlFont.fontFamily = settings.value("onlcustomfam").toString();
		int onlSiz = settings.value("onlcustomsizf", 9).toInt();
		onlSiz = onlSiz < 6 ? 6 : onlSiz;
		onlSiz = onlSiz > 24 ? 24 : onlSiz;
				
		onlFont.fontSize = onlSiz;		
				
		QColor onlineCol;
				QVariant onlCol= settings.value("onlcustomcol");
				if ( onlCol.canConvert<QColor>())
					onlineCol = onlCol.value<QColor>();
					
				onlFont.fontColor = onlineCol;	
	} else {
		QFont font;
		onlFont.fontFamily = font.family();
		onlFont.fontSize = font.pointSize();
		QColor color;
		onlFont.fontColor = color;
	}
	
	if ( customOffline )
		{
			offFont.fontFamily = settings.value("offcustomfam").toString();
			int offSiz = settings.value("offcustomsizf", 9).toInt();
			offSiz = offSiz < 6 ? 6 : offSiz;
			offSiz = offSiz > 24 ? 24 : offSiz;
					
			offFont.fontSize = offSiz;		
					
			QColor offlineCol;
					QVariant offCol= settings.value("offcustomcol");
					if ( offCol.canConvert<QColor>())
						offlineCol = offCol.value<QColor>();
						
					offFont.fontColor = offlineCol;	
		} else {
			QFont font;
			offFont.fontFamily = font.family();
			offFont.fontSize = font.pointSize();
			QColor color;
			offFont.fontColor = color;
		}
	
	settings.endGroup();
	
	//messaging settings
	
	settings.beginGroup("messaging");
	tabMode = settings.value("tab", true).toBool();
	if ( tabMode )
	{
		generalChatWindow = new tabChatWindow(this);
		generalChatWindow->setAttribute(Qt::WA_QuitOnClose, false);
	}
	
	showNames = settings.value("chatnames", true).toBool();
	timestamp = settings.value("timestamp", 1).toInt();
	sendOnEnter = settings.value("onenter", false).toBool();
	closeOnSend = settings.value("closeonsend", false).toBool();
	dontShowEvents = settings.value("event", false).toBool();
	openNew = settings.value("opennew", false).toBool();
	
//	delete codec;
	codepage = settings.value("codepage", "Windows-1251").toString();
	codec = QTextCodec::codecForName(codepage.toLocal8Bit());
	sendTyping = settings.value("typing", false).toBool();
	
	lightChatView = settings.value("lightchat", false).toBool();
	dontShowIncomingMessagesInTrayEvents = settings.value("dshowmsg", false).toBool();
	settings.endGroup();
	
	
	
	//status settings
	
	settings.beginGroup("statuses");
	
	webAware = settings.value("webaware", false).toBool();
	
	showXStatuses = settings.value("xstatus", true).toBool();
	
	showXstatusesinToolTips = settings.value("xstattool", false).toBool();
	settings.endGroup();
	
	
	settings.beginGroup("history");
	
	saveHistory = settings.value("save", true).toBool();
	saveNilHistory = settings.value("nil", true).toBool();
	saveServiceHistory = settings.value("service", true).toBool();
	showRecent = settings.value("recent", true).toBool();
	quint8 tmpCount = settings.value("recentcount", 3).toUInt();
	tmpCount = tmpCount < 1 ? 1:tmpCount;
	tmpCount = tmpCount > 10 ? 10:tmpCount;
	recentCount = tmpCount;
	
	onlineNotify = settings.value("onlnot", false).toBool();
	offlineNotify = settings.value("offnot", false).toBool();
	readAwayNotify = settings.value("awaynot", true).toBool();
	
	settings.endGroup();
	
	settings.beginGroup("events");
	
	disableTrayBlinking = settings.value("trayblink", false).toBool();
	disableButtonBlinking = settings.value("traybutton", false).toBool();
	
	signOnNot = settings.value("notsignon", true).toBool();
	signOffNot = settings.value("notsignoff", true).toBool();
	typingNot = settings.value("nottyping", true).toBool();
	changeStatusNot = settings.value("notchange", true).toBool();
	awayNot = settings.value("notaway", true).toBool();
	
	settings.endGroup();
	
	
	
	settings.beginGroup("antispam");
	
	getOnlyFromContactList = settings.value("accept", false).toBool();
	notifyAboutBlocked = settings.value("notify", true).toBool();
	blockAuth = settings.value("auth", false).toBool();
	blockUrlMessage = settings.value("url", false).toBool();
	enableAntiSpamBot = settings.value("bot", false).toBool();
	dontAnswerBotIfInvis = settings.value("reply", false).toBool();
	
	question = settings.value("question").toString();
	answer = settings.value("answer").toString();
	messageAfterAnswer = settings.value("rightansw").toString();
	
	
	settings.endGroup();
	
	settings.beginGroup("emoticons");
	
	emoticonXMLPath = settings.value("path", "").toString();
	if (emoticonXMLPath.isEmpty() || !QFile::exists(emoticonXMLPath))
	{
		QStringList paths;
		paths<<"/usr/share/qutim/emoticons/"<<settings.fileName().section('/', 0, -3) + "/emoticons/"<<"emoticons/";
		foreach(QString path, paths)
		{
			
			QDir emoticonPath = path;

			QStringList themes = emoticonPath.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
			
			foreach(QString dirName, themes)
			{

				QDir dir(path + "/" + dirName);

				QStringList filter;
				filter<<"*.xml";
				
				QStringList fileXML = dir.entryList(filter,QDir::Files);
				
				if ( fileXML.count())
				{
					emoticonXMLPath =  dir.path() + "/" + fileXML.at(0);

					break;
				}
			}
		}
		settings.setValue("path", emoticonXMLPath);

	}
	
	settings.endGroup();
}

void contactListTree::clSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.beginGroup("contactlist");
	bool showGrp = settings.value("showgroups", true).toBool();
	bool hideEmp = settings.value("hideempty", false).toBool();
	bool showOff = settings.value("hideoff", false).toBool();
	bool hideSep = settings.value("hidesep", false).toBool();
	bool dontundeNotAuth = settings.value("auth",false).toBool();
	bool hideBrth = settings.value("birthday", false).toBool();
	if ( showGrp != showGroups )
	{
		showGroups = showGrp;
		hideEmpty = hideEmp;
		showOffline = showOff;
		hideSeparators = hideSep;
		removeGroups();
		if ( !isMergeAccounts )
		{
			
			showHideGroups();
			
		}
	} else {
		if ( hideEmpty != hideEmp)
		{
			hideEmpty = hideEmp;
			hideEmptyGroups(hideEmpty);
		}
		
		if ( showOff != showOffline )
		{
			showOffline = showOff;
			showOfflineUsers();
		}
		
		if ( hideSeparators != hideSep )
		{
			hideSeparators = hideSep;
			setHideSeparators(hideSeparators);
		}
	}
	
	if ( dontundeNotAuth != dontUnderlineNotAutho  || hideBrth != hideBirth)
	{
		dontUnderlineNotAutho = dontundeNotAuth;
		hideBirth = hideBrth;
		updateBuddyListFlags();
	}
	
	customAccount = settings.value("acccustomf", false).toBool();
	if ( customAccount )
	{
		QFont font;
		font.setFamily(settings.value("acccustomfam").toString());
		int accSiz = settings.value("acccustomsizf", 9).toInt();
		accSiz = accSiz < 6 ? 6 : accSiz;
		accSiz = accSiz > 24 ? 24 : accSiz;
		
		font.setPointSize(accSiz);
		QColor accountCol;
		QVariant accCol= settings.value("acccustomcol");
		if ( accCol.canConvert<QColor>())
			accountCol = accCol.value<QColor>();
			
		rootItem->setFont(1,font);
		rootItem->setForeground(1,accountCol);
	} else {
		QFont font;
		rootItem->setFont(1,font);
		QColor accountCol;
		rootItem->setForeground(1,accountCol);
	}
	
	customGroup = settings.value("grpcustomf", false).toBool();
	
	if ( customGroup )
		{
			grpFont.fontFamily = settings.value("grpcustomfam").toString();
			int grpSiz = settings.value("grpcustomsizf", 7).toInt();
			grpSiz = grpSiz < 6 ? 6 : grpSiz;
			grpSiz = grpSiz > 24 ? 24 : grpSiz;
			
			grpFont.fontSize = grpSiz;		
			
			QColor groupCol;
			QVariant grpCol= settings.value("grpcustomcol");
			if ( grpCol.canConvert<QColor>())
				groupCol = grpCol.value<QColor>();
				
			grpFont.fontColor = groupCol;
			updateGroupCustomFont();
		} else {
			QFont font;
			grpFont.fontFamily = font.family();
			grpFont.fontSize = 7;
			QColor color;
			grpFont.fontColor = color;
			updateGroupCustomFont();
		}
	
	
	bool onlCust = settings.value("onlcustomf", false).toBool();
	bool offCust = settings.value("offcustomf", false).toBool();
	QString tmpOnlF = settings.value("onlcustomfam").toString();
	QString tmpOffF = settings.value("offcustomfam").toString();
	
	int onlSiz = settings.value("onlcustomsizf", 9).toInt();
	onlSiz = onlSiz < 6 ? 6 : onlSiz;
	onlSiz = onlSiz > 24 ? 24 : onlSiz;
	
	int tmpOnlS = onlSiz; 
	
	
	int offSiz = settings.value("offcustomsizf", 9).toInt();
	offSiz = offSiz < 6 ? 6 : offSiz;
	offSiz = offSiz > 24 ? 24 : offSiz;
		
	int tmpOffS = offSiz;
	
	QColor tmpOnlC;
	QVariant onlCol= settings.value("onlcustomcol");
	if ( onlCol.canConvert<QColor>())
		tmpOnlC = onlCol.value<QColor>();
	
	QColor tmpOffC;
	QVariant offCol= settings.value("offcustomcol");
	if ( offCol.canConvert<QColor>())
		tmpOffC = offCol.value<QColor>();
	
	bool contactsChanged = false;
	
	if ( customOnline != onlCust ||
			onlFont.fontFamily != tmpOnlF ||
			onlFont.fontSize != tmpOnlS || 
			onlFont.fontColor != tmpOnlC )
	{
		customOnline = onlCust;
		if ( customOnline )
		{
			onlFont.fontFamily = tmpOnlF;
							
			onlFont.fontSize = tmpOnlS;	
								
			onlFont.fontColor = tmpOnlC;
		} else {
			QFont font;
			onlFont.fontFamily = font.family();
			onlFont.fontSize = font.pointSize();
			QColor color;
			onlFont.fontColor = color;
		}
		contactsChanged = true;
	}
	
	if ( customOffline != offCust ||
				offFont.fontFamily != tmpOffF ||
				offFont.fontSize != tmpOffS || 
				offFont.fontColor != tmpOffC )
	{
		customOffline = offCust;	
		if ( customOffline )
		{
			offFont.fontFamily = tmpOffF;
						
			offFont.fontSize = tmpOffS;	
								
			offFont.fontColor = tmpOffC;
		} else {
			QFont font;
			offFont.fontFamily = font.family();
			offFont.fontSize = font.pointSize();
			QColor color;
			offFont.fontColor = color;
		}
		contactsChanged = true;
	}
	
	if ( contactsChanged )
		updateContactsCustomFont();
	
	settings.endGroup();
}

void contactListTree::createOnOffGroups()
{
	onlineList = new contactSeparator(rootItem);
	onlineList->setText(1,tr("Online"));
	onlineList->setTextAlignment(1, Qt::AlignCenter);
	offlineList = new contactSeparator(rootItem);
	offlineList->setText(1,tr("Offline"));
	offlineList->setTextAlignment(1, Qt::AlignCenter);
	offlineList->setFont(1, QFont(offlineList->font(1).family(), 7));
	QLinearGradient linearGrad(0,0,0,16);
	linearGrad.setColorAt(0, QColor(255,255,255));
	linearGrad.setColorAt(0.5, QColor(248,248,248));
	linearGrad.setColorAt(1, QColor(255,255,255));
	QBrush brush(linearGrad);
	offlineList->setBackground(1, brush);
	offlineList->setBackground(0, brush);
	offlineList->setBackground(2, brush);
	onlineList->setBackground(0, brush);
	onlineList->setBackground(1, brush);
	onlineList->setBackground(2, brush);
	onlineList->setFont(1, QFont(offlineList->font(1).family(), 7));
	onlineList->setHidden(true);
	offlineList->setHidden(true);
}

void contactListTree::removeGroups()
{
	if ( !isMergeAccounts )
	{
	if (showGroups)
	{
			offlineList->takeChildren();
		onlineList->takeChildren();
		delete offlineList;
		delete onlineList;
	} else {
		foreach( treeGroupItem *group, groupList)
			{
				group->onlineList->takeChildren();
				group->offlineList->takeChildren();
				group->setHidden(true);
			}
		}
	}
}

void contactListTree::prepareForMerge()
{
	foreach(treeBuddyItem *buddy, buddyList)
	{		
		Q_ASSERT(buddy);
		if ( !buddy)
			continue;

		if ( buddy->isOffline )
		{	
			offlineList->removeChild(buddy);
			if ( groupList.contains(buddy->groupID))
			{
				Q_ASSERT(groupList.value(buddy->groupID));
				Q_ASSERT(groupList.value(buddy->groupID)->offlineList);
				if (groupList.value(buddy->groupID) && groupList.value(buddy->groupID)->offlineList)
					groupList.value(buddy->groupID)->offlineList->addChild(buddy);
			}
		} 
		else 
		{
			onlineList->removeChild(buddy);
			if (groupList.contains(buddy->groupID))
			{
				Q_ASSERT(groupList.value(buddy->groupID));
				Q_ASSERT(groupList.value(buddy->groupID)->onlineList);
				if (groupList.value(buddy->groupID) && groupList.value(buddy->groupID)->onlineList)
					groupList.value(buddy->groupID)->onlineList->addChild(buddy);
			}
		}
	}
//	delete onlineList;
//	delete offlineList;
}

void contactListTree::showHideGroups()
{
	if ( showGroups )
			{
		
			foreach(treeBuddyItem *buddy, buddyList)
			{
				Q_ASSERT(buddy);
				Q_ASSERT(groupList.value(buddy->groupID));
				Q_ASSERT(groupList.value(buddy->groupID)->offlineList);
				Q_ASSERT(groupList.value(buddy->groupID)->onlineList);
				if ( buddy->isOffline )
				{
					if (groupList.value(buddy->groupID))
					{
						if (groupList.value(buddy->groupID)->offlineList) 
						{
							groupList.value(buddy->groupID)->offlineList->addChild(buddy);
							groupList.value(buddy->groupID)->offlineList->sortChildren(1,Qt::AscendingOrder);
						}
						groupList.value(buddy->groupID)->setHidden(false);
					}
				} else {
					if (groupList.value(buddy->groupID))
					{
						if (groupList.value(buddy->groupID)->onlineList)
						{
							groupList.value(buddy->groupID)->onlineList->addChild(buddy);
							groupList.value(buddy->groupID)->onlineList->sortChildren(1,Qt::AscendingOrder);
						}
						groupList.value(buddy->groupID)->setHidden(false);
					}
				}
			}
			prepareForShowGroups();
			hideEmptyGroups(hideEmpty);
			} else {
				createOnOffGroups();
				foreach(treeBuddyItem *buddy, buddyList)
				{
					if ( buddy->isOffline )
					{
						offlineList->addChild(buddy);
					} else {
						onlineList->addChild(buddy);
					}
				}
				
				if ( onlineList->childCount() )
				{
					onlineList->setHidden(false);
					onlineList->setExpanded(true);
				}
				if ( offlineList->childCount() )
				{
					if ( !showOffline )
						offlineList->setHidden(false);	
						offlineList->setExpanded(true);
				}
				onlineList->sortChildren(1,Qt::AscendingOrder);
				if ( !showOffline )
					offlineList->sortChildren(1,Qt::AscendingOrder);
			}

	showOfflineUsers();
	setHideSeparators(hideSeparators);
}

void contactListTree::prepareForShowGroups()
{
	foreach(treeGroupItem *group, groupList)
	{

				if (group->userCount > 0)
				{
					if ( group->onlineList->childCount() )
					{
						group->onlineList->setHidden(false);
						group->onlineList->setExpanded(true);
					}
					if ( group->offlineList->childCount() )
					{
						if ( !showOffline )
						group->offlineList->setHidden(false);	
						group->offlineList->setExpanded(true);
					}
					group->setHidden(false);
				}
				group->onlineList->sortChildren(1,Qt::AscendingOrder);
				if ( !showOffline )
				group->offlineList->sortChildren(1,Qt::AscendingOrder);
			}
}

void contactListTree::hideEmptyGroups(bool hide)
{
	foreach(treeGroupItem *group, groupList)
	{
		if ( !group->userCount ) 
			group->setHidden(hide);
		else if ( !group->onlineList->childCount() && showOffline)
		{
			group->setHidden(hide);
		}
	}
}

void contactListTree::showOfflineUsers()
{
	if ( !isMergeAccounts)
	{
		if ( showGroups ){
			foreach( treeGroupItem *group, groupList)
			{
				
				if ( group->offlineList->childCount() )
				{
					group->offlineList->setHidden(showOffline);

						group->setHidden(false);
						if ( hideEmpty && showOffline && !group->onlineList->childCount())
							group->setHidden(true);
				} else if (hideEmpty && !group->onlineList->childCount() )
					group->setHidden(true);
				group->offlineList->sortChildren(1, Qt::AscendingOrder);
			}
		} else {
			
			offlineList->sortChildren(1, Qt::AscendingOrder);
			if ( offlineList->childCount() )
				offlineList->setHidden(showOffline);
		}
	}
}

void contactListTree::clearNilUsers()
{
	if ( groupList.contains(0))
	{
	groupList.value(0)->userCount = 0;
	groupList.value(0)->offlineList->setHidden(true);
	groupList.value(0)->updateText();
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	QStringList notNil = contacts.value("list/contacts").toStringList();
	foreach(treeBuddyItem *buddy, buddyList)
	{
		if ( !buddy->groupID )
		{
			notNil.removeAll(buddy->getUin());
			contacts.remove(buddy->getUin());
			getBuddies.removeAll(buddy->getUin());
			buddyList.remove(buddy->getUin());
			delete buddy;
		}
	}
	contacts.setValue("list/contacts", notNil);
	}
}

void contactListTree::updateBuddyListFlags()
{
	foreach(treeBuddyItem *buddy, buddyList)
	{
		buddy->underline = !dontUnderlineNotAutho;
		buddy->birth = !hideBirth;
		buddy->updateBuddyText();
	}
}

void contactListTree::updateGroupCustomFont()
{
	foreach(treeGroupItem *group, groupList)
		group->setCustomFont(grpFont.fontFamily, grpFont.fontSize, grpFont.fontColor);
}

void contactListTree::updateContactsCustomFont()
{
	foreach(treeBuddyItem *buddy, buddyList)
	{
		if ( buddy->isOffline )
			buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
		else
			buddy->setCustomFont(onlFont.fontFamily, onlFont.fontSize, onlFont.fontColor);
	}
}


void contactListTree::msgSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.beginGroup("messaging");
	bool tbMd = settings.value("tab", true).toBool();
	if ( tabMode != tbMd )
	{
		if ( tabMode = tbMd )
		{
			generalChatWindow = new tabChatWindow(this);
			generalChatWindow->setAttribute(Qt::WA_QuitOnClose, false);
			foreach(chatWindow *c, chatWindowList)
			{
				generalChatWindow->addChatWindow(c);
			}
		}
		else
		{
			generalChatWindow->detachChildren();
			generalChatWindow->deleteChatWindows = false;
			delete generalChatWindow;
			foreach(chatWindow *c, chatWindowList)
			{
				c->show();
			}
		}
	}
	
	bool shNms = settings.value("chatnames", true).toBool();
	quint8 tmstmp = settings.value("timestamp", 1).toInt();
	bool onEnter = settings.value("onenter", false).toBool();
	bool clsOnSnd = settings.value("closeonsend", false).toBool();
	bool sndTpng = settings.value("typing", false).toBool();
	if ( showNames != shNms  || timestamp != tmstmp || sendOnEnter != onEnter ||
			closeOnSend != clsOnSnd  || sendTyping != sndTpng )
	{
		showNames = shNms;
		timestamp = tmstmp;
		sendOnEnter = onEnter;
		closeOnSend = clsOnSnd;
		sendTyping = sndTpng;
		foreach(chatWindow *w, chatWindowList)
		{
			w->showNames = showNames;
			w->timestamp = timestamp;
			w->setOnEnter(sendOnEnter);
			w->closeOnSend = closeOnSend;
			w->sendTyping = sendTyping;	
		}
			
	}
	dontShowEvents = settings.value("event", false).toBool();
	openNew = settings.value("opennew", false).toBool();
//	delete codec;
	codepage = settings.value("codepage", "Windows-1251").toString();
	codec = QTextCodec::codecForName(codepage.toLocal8Bit());
	lightChatView = settings.value("lightchat", false).toBool();
	dontShowIncomingMessagesInTrayEvents = settings.value("dshowmsg", false).toBool();
	settings.endGroup();
}

void contactListTree::updateChatBuddyStatus(const QString &buddy, const QIcon &icon)
{
	if ( chatWindowList.contains(buddy) )
	{
		if ( tabMode )
		{
			generalChatWindow->updateStatusIcon(chatWindowList.value(buddy),icon);
		} else {
			chatWindowList.value(buddy)->setWindowIcon(icon);
		}
	}
}

void contactListTree::initializeWindow(chatWindow *w)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	connect(w , SIGNAL(sendTypingNotification(const QString &, quint16)), 
			this, SLOT(sendTypingNotifications(const QString &, quint16)));
	connect(w , SIGNAL(showHistory(const QString &)), 
			this, SLOT(showHistory(const QString &)));
	connect(w , SIGNAL(openInfoWindow(const QString &)), 
				this, SLOT(openInfoWindow(const QString &)));
	connect(w , SIGNAL(sendImage(const QString &,const QString &)), 
				this, SLOT(sendImage(const QString &,const QString &)));
	connect(w , SIGNAL(sendFile(const QString &)), 
				this, SLOT(sendFileFromWindow(const QString &)));
	connect(w , SIGNAL(sendFontSignal(const QFont&, const QColor &, const QColor &)), 
				this, SLOT(getChangeFontSignal(const QFont &, const QColor &, const QColor &)));
	
	QVariant fontCol = settings.value("chatwindow/fontcolor");
	QColor fontColor;
	if ( fontCol.canConvert<QColor>())
		fontColor = fontCol.value<QColor>();
	
	QVariant backCol = settings.value("chatwindow/backcolor");
	QColor backColor;
	if ( backCol.canConvert<QColor>())
		backColor = backCol.value<QColor>();
	else
		backColor.setRgb(255,255,255);
	
	QVariant chatFontVar = settings.value("chatwindow/font");
	QFont chatFont;
	if ( chatFontVar.canConvert<QFont>())
		chatFont = chatFontVar.value<QFont>();
	
	
	/*Temp hack by Garfeild  17.07.2008
	*First time fill background of messageEdit by black colour,
	*second time by user's or white.
	*/
	QColor tempColor;
	tempColor.setRgb(0,0,0);
	w->setWindowFont(chatFont, fontColor, tempColor);
	w->setWindowFont(chatFont, fontColor, backColor);
	
	QString uin = w->chatWith;
	
	if ( buddyList.contains(uin) )
	{
		treeBuddyItem *buddy = buddyList.value(uin);
		QByteArray iconhash = buddy->avatarMd5Hash.toHex();
		if ( !iconhash.isEmpty())
			w->setAvatars(iconPath + iconhash);
		
		w->setContactName(buddy->getName());
		w->setContactClient(buddy->clientId);
//		qDebug()<<contacts.value(uin + "/note","").toString();
		w->setContactNote(contacts.value(uin + "/note","").toString());
	}
	
	QByteArray iconhash = settings.value("main/iconhash").toByteArray();
	if ( !iconhash.isEmpty())
		w->setOwnerAvatar(iconPath + iconhash);
	
	w->accountNickName = accountNickname;
	w->setOnline(iAmOnline);
	w->restoreState();
	w->showNames = showNames;
	w->timestamp = timestamp;
	w->setOnEnter(sendOnEnter);
	w->closeOnSend = closeOnSend;
	w->setTyping(sendTyping);
	w->lightVersion =lightChatView;
	w->resize(settings.value("chatwindow/size", QSize(400,300)).toSize());
	if ( tabMode )
	{
		if ( !generalChatWindow->count() )
		generalChatWindow->resize(settings.value("chatwindow/size", QSize(400,300)).toSize());
	}
	
	w->setEmoticonPath(emoticonXMLPath);

	if ( showRecent )
	{
		if ( messageList.contains(w->chatWith) )
		{
			if ( recentCount >= messageList.value(w->chatWith)->messageList.count() )
				historyObject->setRecentMessages(w, recentCount);
			else
				historyObject->setRecentMessages(w, messageList.value(w->chatWith)->messageList.count() );
		}
		else
			historyObject->setRecentMessages(w, recentCount);
	}
}

void contactListTree::activateWindow(const QString &uin)
{
	if ( messageList.contains(uin) )
	{
		
		readMessageFrom(messageList.value(uin));
	}
}


void contactListTree::requestUinInformation(const QString &uin)
{
	emit incSnacSeq();
	emit incMetaSeq();
	metaInformation metaInfo(icqUin);
	metaInfo.sendShortInfoReq(tcpSocket, *flapSeq, *snacSeq, *metaSeq, uin);
	int tmpMetaSeq = ((*metaSeq )% 0x100) * 0x100 + ((*metaSeq )/ 0x100);
	metaInfoRequestList.insert(tmpMetaSeq ,uin);
	emit incFlapSeq();
}

quint16 contactListTree::byteArrayToLEInt16(const QByteArray &array)
{
	bool ok;
	quint16 tmp = array.toHex().toUInt(&ok,16);
	return ((tmp % 0x100) * 0x100 + (tmp)/ 0x100);
}

void contactListTree::readShortInfo(const metaInformation &meta, quint16 metaSeq)
{

	if ( waitForMineInfo && metaSeq == mineMetaSeq)
	{
		 if ( meta.nick.size() )
			 accountNickname = codec->toUnicode(meta.nick);
		 else
			 accountNickname = icqUin;
		 QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
		 settings.setValue("main/nick", accountNickname);
		waitForMineInfo = false;
	}
	
	if ( metaInfoRequestList.contains(metaSeq) )
	{
		treeBuddyItem *buddy = buddyList.value(metaInfoRequestList.value(metaSeq));
		if ( meta.nick.size())
			buddy->setName(codec->toUnicode(meta.nick));
		else
			buddy->setName(buddy->getUin());
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
		buddy->updateBuddyText();
		settings.setValue(buddy->getUin() + "/nickname", buddy->getName());
		metaInfoRequestList.remove(metaSeq);
	}
}

void contactListTree::readMetaData(quint16 length, bool notAlone)
{
	quint16 metaSeq = byteArrayToInt16(socket->read(2));
	quint16 dataSubType = byteArrayToInt16(socket->read(2));
	metaInformation metaInfo(icqUin);
	quint8 success;
	switch( dataSubType )
	{
	case 0x0401:
		length = length - 4 - metaInfo.readShortInfo(socket);
		readShortInfo(metaInfo, metaSeq);
		if ( length  )
			socket->read(length);
		break;
	case 0xc800:
		length = length - 4 - metaInfo.readBasicUserInfo(socket);
		
		readBasicUserInfo(metaInfo, metaSeq);
		
		fullIndoEnd(metaSeq, notAlone);
		
		if ( length )
			socket->read(length);
		break;
	
	case 0xdc00:
		length = length - 4 - metaInfo.readMoreUserInfo(socket);
		
		readMoreUserInfo(metaInfo, metaSeq);
		
		fullIndoEnd(metaSeq, notAlone);
		
		if ( length )
			socket->read(length);
		break;
	case 0xd200:
		length = length - 4 - metaInfo.readWorkUserInfo(socket);
		
		readWorkUserInfo(metaInfo, metaSeq);
		
		fullIndoEnd(metaSeq, notAlone);
		
		if ( length )
			socket->read(length);
		break;
	case 0xf000:
		length = length - 4 - metaInfo.readInterestsUserInfo(socket);
		
		readInterestsUserInfo(metaInfo, metaSeq);
		
		fullIndoEnd(metaSeq, notAlone);
		
		if ( length > 0 )
			socket->read(length);
		break;
	case 0xe600:
		length = length - 4 - metaInfo.readAboutUserInfo(socket);
		
		readAboutUserInfo(metaInfo, metaSeq);
		
		fullIndoEnd(metaSeq, notAlone);
		
		if ( length > 0 )
			socket->read(length);
		break;
	case 0xeb00:
		fullIndoEnd(metaSeq, notAlone);
		socket->read(length - 4);
		break;
	case 0x0e01:
		fullIndoEnd(metaSeq, notAlone);
		socket->read(length - 4);
		break;
	case 0xfa00:
		fullIndoEnd(metaSeq, notAlone);
		socket->read(length - 4);
		break;	
	case 0xa401:
		length = length - 4 - metaInfo.readSearchResult(socket, false);
		
		addSearchResult(false, metaInfo.founded, metaInfo.foundedUin, metaInfo.foundedNick,
				metaInfo.foundedFirst, metaInfo.foundedLast, metaInfo.foundedEmail,
				metaInfo.authFlag, metaInfo.foundedStatus, metaInfo.foundedGender,
				metaInfo.foundedAge);
		
		if ( length )
			socket->read(length);
		break;
	case 0xae01:
		length = length - 4 - metaInfo.readSearchResult(socket, true);
		
		addSearchResult(true, metaInfo.founded, metaInfo.foundedUin, metaInfo.foundedNick,
				metaInfo.foundedFirst, metaInfo.foundedLast, metaInfo.foundedEmail,
				metaInfo.authFlag, metaInfo.foundedStatus, metaInfo.foundedGender,
				metaInfo.foundedAge);
		
		if ( length )
			socket->read(length);
		break;
		
	case 0xaa00:
		success = convertToInt8(socket->read(1));
		length--;
		
		if ( success == 0x0a )
			emit sendSystemMessage(tr("Password is successfully changed"));
		else
			emit sendSystemMessage(tr("Password is not changed"));
		
		if ( length)
			socket->read(length);
		
		break;
	default:
		socket->read(length - 4);
	}
}

QByteArray contactListTree::convertToByteArray(const quint16 &d)
{
	QByteArray packet;
		packet[0] = (d / 0x100);
		packet[1] = (d % 0x100);
		return packet;
}

void contactListTree::sendTypingNotifications(const QString &uin, quint16 type)
{

	emit incSnacSeq();
	QByteArray packet;
	packet[0] = 0x2a;
	packet[1] = 0x02;
	
	packet.append(convertToByteArray((quint16)*flapSeq));
	
	
	snac snac0414;
	snac0414.setFamily(0x0004);
	snac0414.setSubType(0x0014);
	snac0414.setReqId(*snacSeq);
	quint16 length = 10;
	QByteArray typeNotif;
	typeNotif[0] = 0x00;
	typeNotif[1] = 0x00;
	typeNotif[2] = 0x00;
	typeNotif[3] = 0x00;
	typeNotif[4] = 0x00;
	typeNotif[5] = 0x00;
	typeNotif[6] = 0x00;
	typeNotif[7] = 0x00;
	typeNotif[8] = 0x00;
	typeNotif[9] = 0x01;
	typeNotif[10] = (quint8)(uin.length());
	typeNotif.append(uin);
	typeNotif.append(convertToByteArray((quint16)type));
	length += typeNotif.size();	
	packet.append(convertToByteArray((quint16)length));
	packet.append(snac0414.getData());
	packet.append(typeNotif);
	
	tcpSocket->write(packet);
	emit incFlapSeq();
}

void contactListTree::getTypingNotification(quint16 length)
{
	int tmpLength = 13;
	socket->read(8);
	quint16 channel = byteArrayToInt16(socket->read(2));
	bool ok;
	quint8 uinlength = socket->read(1).toHex().toUInt(&ok, 16);
	QString uin(socket->read(uinlength));
	
	quint16 notifType = byteArrayToInt16(socket->read(2));
	
	if ( buddyList.contains( uin ))
	{
	if ( channel == 0x0001 )
	{
		if ( notifType  == 0x0002 )
		{
			if ( !notifList.count() )
				QTimer::singleShot(5000, this, SLOT(clearNotifList()));
			if ( !notifList.contains(uin) )
			{
				notifList<<uin;
				
				if ( chatWindowList.contains(uin) )
				{
					
					chatWindow *w = chatWindowList.value(uin);
					if ( !dontShowEvents )
					{
						if ( !w->isActiveWindow() && typingNot)
//							emit userMessage(tr("%1 is typing").arg(buddyList.value(uin)->getName()));
						emit userMessage(uin, buddyList.value(uin)->getName(), "", typingNotification, true);
					}
					
					w->typingIcon(true);
					
				} else 
				{
					if ( typingNot )
						emit userMessage(uin, buddyList.value(uin)->getName(), "", typingNotification, true);
				}
			}
		} else if ( notifType == 0x0000)
		{
			if ( chatWindowList.contains(uin) )
			{
				chatWindowList.value(uin)->typingIcon(false);
			}
		}
	}
	} else {
		if(typingNot)
			emit userMessage("", uin, "", typingNotification, false);
//		emit userMessage(tr("%1 is typing ( not from list )").arg(uin));
	}
	
	tmpLength += uinlength;
	if ( length - tmpLength > 0 )
		socket->read(length - tmpLength);
}

void contactListTree::statusSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.beginGroup("statuses");
	showXstatusesinToolTips = settings.value("xstattool", false).toBool();
	bool wbAware = settings.value("webaware", false).toBool();
	bool autoAw = settings.value("autoaway", true).toBool();
	quint32 awMin = settings.value("awaymin", 10).toUInt();
	
	bool shXStat = 	settings.value("xstatus", true).toBool();
	if ( wbAware != webAware )
	{
		webAware = wbAware;
		emit updateStatus();
	}
	
	if ( autoAw != autoAway || awMin != awayMin )
	{
		autoAway = autoAw;
		awayMin = awMin;
		emit restartAutoAway(autoAway, awayMin);
	}
	
	if ( shXStat != showXStatuses)
	{
		showXStatuses = shXStat;
		if ( showXStatuses)
		{
			foreach(treeBuddyItem *buddy, buddyList)
				buddy->checkForXStatus();
		} else 
		{
			foreach(treeBuddyItem *buddy, buddyList)
				buddy->waitingForAuth(buddy->authorizeMe);
		}
	}
	emit updateStatusMenu(settings.value("customstat", true).toBool());
	settings.endGroup();
}

void contactListTree::initializeBuddy(treeBuddyItem * /*buddy */)
{

}

void contactListTree::changePrivacy(quint8 flag)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.setValue("statuses/privacy", flag);
	

	emit incSnacSeq();
	servicesSetup privacySetup(icqUin);
	privacySetup.flap1309seq = *flapSeq;
	privacySetup.snac1309seq = *snacSeq;
	privacySetup.setPrivacy(icqUin, pdInfoID, pdInfoGroupId, tcpSocket);
	emit incFlapSeq();
}

QString contactListTree::getCurrentAwayMessage() const
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	QString awayMessage;
	switch(currentStatus)
	{
	case away:
		awayMessage = settings.value("autoreply/awaymsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
	case lunch:
		awayMessage = settings.value("autoreply/lunchmsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
		
	case evil:
		awayMessage = settings.value("autoreply/evilmsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
		
	case depression:
		awayMessage = settings.value("autoreply/depressionmsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
		
	case athome:
		awayMessage = settings.value("autoreply/athomemsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
		
	case atwork:
		awayMessage = settings.value("autoreply/atworkmsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
		
	case na:
		awayMessage = settings.value("autoreply/namsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
		
	case occupied:
		awayMessage = settings.value("autoreply/occupiedmsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
		
	case dnd:
		awayMessage = settings.value("autoreply/dndmsg", "").toString();
		awayMessage.append(QChar(0x00));
		return awayMessage;
			
	default:
		return QChar(0x00);
	}
}

void contactListTree::historySettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.beginGroup("history");
		
	saveHistory = settings.value("save", true).toBool();
	saveNilHistory = settings.value("nil", true).toBool();
	saveServiceHistory = settings.value("service", true).toBool();
	showRecent = settings.value("recent", true).toBool();
	quint8 tmpCount = settings.value("recentcount", 3).toUInt();
	tmpCount = tmpCount < 1 ? 1:tmpCount;
	tmpCount = tmpCount > 10 ? 10:tmpCount;
	recentCount = tmpCount;
		
	onlineNotify = settings.value("onlnot", false).toBool();
	offlineNotify = settings.value("offnot", false).toBool();
	readAwayNotify = settings.value("awaynot", true).toBool();
	
	settings.endGroup();
}

void contactListTree::appExiting()
{
	if ( messageList.size() )
				historyObject->saveUnreadedMessages(&messageList);	
	
}

void contactListTree::loadUnreadedMessages()
{
	QList<messageFormat> msgList;
	historyObject->loadUnreaded(&msgList);
	
	foreach(messageFormat msg, msgList)
	{
		
		if ( buddyList.contains(msg.fromUin) )
		{
			treeBuddyItem *buddy = buddyList.value(msg.fromUin);
			msg.from = buddy->getName();		
		} else {
			
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
			
			treeGroupItem *group = groupList.value(0);
			msg.from = msg.fromUin;
			treeBuddyItem *buddy;
			if ( showGroups )
				buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, group->offlineList);
			else
				buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, offlineList);

			initializeBuddy(buddy);
			buddy->underline = !dontUnderlineNotAutho;
			buddy->groupID = 0;
			buddy->groupName = group->name;
			group->userCount++;
			group->updateText();
			buddyList.insert(msg.fromUin, buddy);
			buddy->setBuddyUin(msg.fromUin);
			buddy->setName(msg.fromUin);
			buddy->updateBuddyText();
			buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
			if ( showGroups )
				updateNil();
			requestUinInformation(buddy->getUin());
			settings.beginGroup(buddy->getUin());
			settings.setValue("name", buddy->getUin());
			settings.setValue("groupid", 0);
			settings.setValue("nickname", buddy->getName());
			settings.endGroup();
			
			QStringList contacts = settings.value("list/contacts").toStringList();
			contacts<<buddy->getUin();
			settings.setValue("list/contacts", contacts);
			
			if ( contactListChanged  && isMergeAccounts )
						QTimer::singleShot(500, this, SIGNAL(reupdateList()));
			
		}
		
		if ( chatWindowList.contains(msg.fromUin))
		{
			chatWindow *w = chatWindowList.value(msg.fromUin);
			w->setMessage(msg.from,msg.message, msg.date);
			if ( !disableButtonBlinking )
				qApp->alert(w, 0);
			if ( tabMode )
				generalChatWindow->setMessageTab(w);
			if ( !w->isActiveWindow() )
			{
				if ( messageList.contains(msg.fromUin))
				{
					messageList.value(msg.fromUin)->messageList.append(new messageFormat(msg));
				} else {
					messageList.insert(msg.fromUin, buddyList.value(msg.fromUin));
					messageList.value(msg.fromUin)->messageList.append(new messageFormat(msg));
				}
				newMessages = true;
				QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
				if ( !messageList.isEmpty() && !disableTrayBlinking)
					QTimer::singleShot(500, this, SIGNAL(getNewMessage()));
			}
			
		} else {
			if ( messageList.contains(msg.fromUin))
			{
				messageList.value(msg.fromUin)->messageList.append(new messageFormat(msg));
			} else {
				messageList.insert(msg.fromUin, buddyList.value(msg.fromUin));
				messageList.value(msg.fromUin)->messageList.append(new messageFormat(msg));
			}
			if ( !messageList.isEmpty() && !disableTrayBlinking)
				QTimer::singleShot(500, this, SIGNAL(getNewMessage()));
		}
		if ( !newMessages )
		{
			newMessages = true;
			QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
		}
		
		if ( openNew )
			doubleClickedBuddy(buddyList.value(msg.fromUin));	
		
	}
}

void contactListTree::showHistory(const QString &uin)
{
	
	if ( !historyList.contains(uin) )
	{
		historyView *histView = new historyView(uin, historyObject);
		connect( histView, SIGNAL(destroyed ( QObject *)),
								this, SLOT(deleteHistoryWindow(QObject *)));
		histView->setAttribute(Qt::WA_QuitOnClose, false);
		histView->setAttribute(Qt::WA_DeleteOnClose, true);
		histView->initializeHistory();
		historyList.insert(uin, histView);
		histView->rellocateDialogToCenter(qutIM::getInstance());
		histView->show();
	}
}

void contactListTree::showServiceHistory()
{
	if ( !historyList.contains(icqUin) )
	{
			historyView *histView = new historyView(icqUin, historyObject);
			connect( histView, SIGNAL(destroyed ( QObject *)),
									this, SLOT(deleteHistoryWindow(QObject *)));
			histView->setAttribute(Qt::WA_QuitOnClose, false);
			histView->setAttribute(Qt::WA_DeleteOnClose, true);
			histView->initializeServiceHistory();
			historyList.insert(icqUin, histView);
			histView->rellocateDialogToCenter(qutIM::getInstance());
			histView->show();
		}
}

void contactListTree::setServiceMessageToWin(const QString &uin, const QString &msg)
{
	if ( chatWindowList.contains(uin) && buddyList.contains(uin))
	{
		treeBuddyItem *buddy = buddyList.value(uin);
		chatWindowList.value(uin)->setServiceMessage(buddy->getName(), msg, QDateTime::currentDateTime());
	}
}

void contactListTree::setHideSeparators(bool hide)
{
	contactList->setUpdatesEnabled(false);
	if ( !showGroups )
	{
		Q_ASSERT(onlineList);
		Q_ASSERT(offlineList);

		if (!onlineList || !offlineList)
			return ;

		onlineList->hideSeparator(hide);
		
		offlineList->hideSeparator(hide);
//		onlineList->sortChildren(1, Qt::AscendingOrder);
//		offlineList->sortChildren(1, Qt::AscendingOrder);
		
	} else {
		foreach(treeGroupItem *group, groupList)
		{
			
			group->hideSeparators(hide);	
		}
	}

	contactList->setUpdatesEnabled(true);
}

void contactListTree::eventsSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.beginGroup("events");
		
	disableTrayBlinking = settings.value("trayblink", false).toBool();
	disableButtonBlinking = settings.value("traybutton", false).toBool();
	
	signOnNot = settings.value("notsignon", true).toBool();
	signOffNot = settings.value("notsignoff", true).toBool();
	typingNot = settings.value("nottyping", true).toBool();
	changeStatusNot = settings.value("notchange", true).toBool();
	awayNot = settings.value("notaway", true).toBool();
	
	settings.endGroup();
}

void contactListTree::startChatWith(const QString &uin)
{
	if ( buddyList.contains(uin) )
	{
		doubleClickedBuddy(buddyList.value(uin));
	} 
}

bool contactListTree::checkBuddyPictureHash(const QByteArray &hash)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");	
	return QFile::exists(settings.fileName().section('/', 0, -2) + "/icqicons/" + hash.toHex());
	
}

void contactListTree::askForAvatars(const QByteArray &hash, const QString &uin)
{
	
	
	
	if ( !hash.isEmpty() && (hash.size() == 16 ))
	{
		if ( !checkBuddyPictureHash(hash) )
		{

			QHostAddress hostAddr = QHostAddress(avatarAddress);
			if ( !hostAddr.isNull())
			{

				if ( !buddyConnection->connectedToServ )
				{

					avatartList.insert(uin, hash);
					buddyConnection->connectToServ(avatarAddress, avatarPort, avatarCookie, tcpSocket->proxy());
					
				}
				else
				{
					if ( buddyConnection->canSendReqForAvatars )
					{
						

						buddyConnection->sendHash(uin,hash);
					}
					else
					{

						avatartList.insert(uin, hash);
					}
				}
			} else {

				avatartList.insert(uin, hash);
			}
			
//			emit incSnacSeq();
//				buddyPicture pic;		
//				pic.sendHash(tcpSocket, uin, hash, *flapSeq, *snacSeq);
//				emit incFlapSeq();
	
		} else {
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");	
			settings.setValue(uin + "/iconhash", hash.toHex());
		}
	}
	

	
}

void contactListTree::sendReqForRedirect()
{
	emit incSnacSeq();
	
	QByteArray packet;
	packet[0] = 0x2a;
	packet[1] = 0x02;
	packet.append(convertToByteArray((quint16)*flapSeq));
	
	packet.append(convertToByteArray((quint16)12));
	snac snac0104;
	snac0104.setFamily(0x0001);
	snac0104.setSubType(0x0004);
	snac0104.setReqId(*snacSeq);
	packet.append(snac0104.getData());
	
	packet.append(convertToByteArray((quint16)0x0010));
	
	
	
	emit incFlapSeq();	
	
	tcpSocket->write(packet);
}

void contactListTree::readSSTserver(quint16 length)
{
	
	socket->read(2);
	length -= 2;
	quint16 familyID = 0;
	
		for ( ;length > 0; )
		{
			tlv tmpTlv;
			tmpTlv.readData(socket);
			length -= tmpTlv.getLength();
			switch(tmpTlv.getTlvType())
			{
			case 0x000d:
			
				familyID = byteArrayToInt16(tmpTlv.getTlvData());
				break;
			case 0x0005:
				
				avatarAddress = tmpTlv.getTlvData();
				break;
				
			case 0x0006:
				avatarCookie = tmpTlv.getTlvData();
				break;
			default:
				;
			}
		}
		if ( familyID != 0x0010 )
		{
			avatarPort = 0;
		} else 
			avatarPort = tcpSocket->peerPort();
		
		if ( length )
			socket->read(length);
		
		
		if ( avatartList.count() || waitForIconUpload )
		{
			QHostAddress hostAddr = QHostAddress(avatarAddress);
						if ( !hostAddr.isNull())
							if ( !buddyConnection->connectedToServ )
								buddyConnection->connectToServ(avatarAddress, avatarPort, avatarCookie, tcpSocket->proxy());
		}
		
		if ( !disableAvatars )
		{
		QHostAddress hostAddr = QHostAddress(avatarAddress);
		if ( !hostAddr.isNull())
				if ( !buddyConnection->connectedToServ )
						buddyConnection->connectToServ(avatarAddress, avatarPort, avatarCookie, tcpSocket->proxy());
		}
		

}

void contactListTree::emptyAvatarList()
{
	if ( avatartList.count() )
	{
		foreach(QString uin, avatartList.keys() )
		{
			askForAvatars(avatartList.value(uin), uin);
		}
		
		avatartList.clear();
	}
	
	if ( waitForIconUpload )
	{
		buddyConnection->uploadIcon(ownIconPath);
		waitForIconUpload = false;
	}
}

void contactListTree::updateAvatar(const QString &uin, QByteArray hash)
{
	if ( buddyList.contains(uin) )
	{
		buddyList.value(uin)->avatarMd5Hash = hash;
	}
}


void contactListTree::antispamSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.beginGroup("antispam");
	
	getOnlyFromContactList = settings.value("accept", false).toBool();
	notifyAboutBlocked = settings.value("notify", true).toBool();
	blockAuth = settings.value("auth", false).toBool();
	blockUrlMessage = settings.value("url", false).toBool();
	enableAntiSpamBot = settings.value("bot", false).toBool();
	dontAnswerBotIfInvis = settings.value("reply", false).toBool();
	
	question = settings.value("question").toString();
	answer = settings.value("answer").toString();
	messageAfterAnswer = settings.value("rightansw").toString();
	
	settings.endGroup();	
}

void contactListTree::notifyBlockedMessage(const QString &from, const QString &message)
{
	emit userMessage(from, from, message, blockedMessage, false);
}

void contactListTree::saveBlocked(const QString &from, const QString& msg, const QDateTime &date)
{
	historyObject->saveServiceMessage(from, date, 3, msg);
}

bool contactListTree::checkMessageForUrl(const QString &msg)
{

	
	bool containsURLs = false;
	containsURLs = msg.contains("http:", Qt::CaseInsensitive) ? true : containsURLs;
	containsURLs = msg.contains("ftp:", Qt::CaseInsensitive) ? true : containsURLs;
	containsURLs = msg.contains("www.", Qt::CaseInsensitive) ? true : containsURLs;
	return containsURLs;
}


bool contactListTree::turnOnAntiSpamBot(const QString &from, const QString &msg, const QDateTime &date)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	quint32 privacy = settings.value("statuses/privacy", 4).toUInt();
	
	
	if ( notifyAboutBlocked)
			notifyBlockedMessage(from, msg);
			
	if ( saveServiceHistory )
		saveBlocked(from, msg, date);
	
	if ( dontAnswerBotIfInvis && (currentStatus == invisible ||
				privacy == 5) )
			return true;
	
	if ( msg == answer)
	{
		messageFormat sendAnswerMessage;
		sendAnswerMessage.fromUin = from;
		sendAnswerMessage.message = messageAfterAnswer;
		emit incSnacSeq();
		icqMessage message(codepage);
	
		message.sendMessage(tcpSocket, sendAnswerMessage,*flapSeq, *snacSeq, false);
		emit incFlapSeq();
		blockedBotList.removeAll(from);
		return false;
	}
	
	if ( !blockedBotList.contains(from) )
	{
		blockedBotList.append(from);
		messageFormat sendAnswerMessage;
		sendAnswerMessage.fromUin = from;
		sendAnswerMessage.message = question;
		emit incSnacSeq();
		icqMessage message(codepage);
	
		message.sendMessage(tcpSocket, sendAnswerMessage,*flapSeq, *snacSeq, false);
		emit incFlapSeq();
		return true;
	}
	
	return true;
	
}

void contactListTree::setAvatarDisabled(bool disable)
{
	disableAvatars = disable;

}

void contactListTree::initializaMenus(QMenu *findUserMenu, QMenu *serviceMenu, QMenu *multipleMenu, QMenu *privacyMenu, QMenu *selfInfoMenu, QMenu *passwordMenu)
{
	serviceMessages = new QAction(QIcon(":/icons/qutim.png"), icqUin, this);
	connect(serviceMessages, SIGNAL(triggered()), this, SLOT(showServiceHistory()));
	
	findUser = new QAction(QIcon(":/icons/qutim.png"), icqUin, this);
	connect(findUser, SIGNAL(triggered()), this, SLOT(findAddUser()));
	findUser->setEnabled(false);
	
	sendMultiple = new QAction(QIcon(":/icons/qutim.png"), icqUin, this);
	connect(sendMultiple, SIGNAL(triggered()), this, SLOT(sendMultipleWindow()));
	sendMultiple->setEnabled(false);
	
	privacyList = new QAction(QIcon(":/icons/qutim.png"), icqUin, this);
	connect(privacyList, SIGNAL(triggered()), this, SLOT(openPrivacyWindow()));
//	privacyList->setEnabled(false);
	
	selfInfo = new QAction(QIcon(":/icons/qutim.png"), icqUin, this);
	connect(selfInfo, SIGNAL(triggered()), this, SLOT(openSelfInfo()));
	
	changePassword = new QAction(QIcon(":/icons/qutim.png"), icqUin, this);
	connect(changePassword, SIGNAL(triggered()), this, SLOT(openChangePasswordDialog()));
	changePassword->setEnabled(false);
	
	findUserMenu->addAction(findUser);
	serviceMenu->addAction(serviceMessages);
	multipleMenu->addAction(sendMultiple);
	privacyMenu->addAction(privacyList);
	selfInfoMenu->addAction(selfInfo);
	passwordMenu->addAction(changePassword);
	
}

void contactListTree::findAddUser()
{
	searchWin = new searchUser;
	connect ( searchWin, SIGNAL(openChatWithFounded(const QString &, const QString &)),
			this, SLOT(openChatWindowWithFounded(const QString &, const QString &)));
	connect ( searchWin, SIGNAL(openInfoWindow(const QString &, const QString &, const QString &, const QString &)),
				this, SLOT(openInfoWindow(const QString &, const QString &, const QString &, const QString &)));
	connect( searchWin, SIGNAL(checkStatusFor(const QString &)),
					this, SLOT(checkStatusFor(const QString &)));
	connect( searchWin, SIGNAL(addUserToContactList(const QString &, const QString &, bool)),
					this, SLOT(addUserToList(const QString &, const QString &,bool)));
	findUserWindowOpen = true;
	searchWin->setAttribute(Qt::WA_QuitOnClose, false);
	searchWin->setAttribute(Qt::WA_DeleteOnClose, true);
	connect( searchWin, SIGNAL(destroyed ( QObject *)),
			this, SLOT(findUserWindowClosed(QObject *)));
	connect( searchWin, SIGNAL(findAskedUsers(int)),
				this, SLOT(searchForUsers(int)));
	findUser->setEnabled(false);
	searchWin->rellocateDialogToCenter(qutIM::getInstance());
	searchWin->show();
}

void contactListTree::findUserWindowClosed(QObject */*obj*/)
{
	findUserWindowOpen = false;
	findUser->setEnabled(true);
}

void contactListTree::searchForUsers(int index)
{
	if ( tcpSocket->state() == QAbstractSocket::ConnectedState)
	{
		if ( index == 0)
		{
			emit incSnacSeq();
			emit incMetaSeq();
			metaInformation metaInfo(icqUin);
			metaInfo.searchByUin(tcpSocket, *flapSeq, *snacSeq, *metaSeq, searchWin->getUin());
			emit incFlapSeq();	
		} else if ( index == 1 )
		{
			emit incSnacSeq();
			emit incMetaSeq();
			metaInformation metaInfo(icqUin);
			metaInfo.searchByEmail(tcpSocket, *flapSeq, *snacSeq, *metaSeq, searchWin->getEmail());
			emit incFlapSeq();	
		} else if (index == 2)
		{
			emit incSnacSeq();
			emit incMetaSeq();
			metaInformation metaInfo(icqUin);
			metaInfo.searchByOther(tcpSocket, *flapSeq, *snacSeq, *metaSeq, 
					searchWin->onlineOnly(), codec->fromUnicode(searchWin->getNick()), codec->fromUnicode(searchWin->getFirst()),
					codec->fromUnicode(searchWin->getLast()), searchWin->gender, searchWin->minAge, searchWin->maxAge,
					searchWin->countryCode, codec->fromUnicode(searchWin->getCity()), searchWin->interestsCode,
					searchWin->languageCode, searchWin->occupationCode, codec->fromUnicode(searchWin->getKeyWords()));
			emit incFlapSeq();		
		}
		
		
	}
}

void contactListTree::addSearchResult(bool last, bool founded, const QString &uin, const QString &nick,
		const QString &firstName, const QString &lastName, const QString &email, const quint8 &authFlag,
		const quint16 &status, const quint8 &gender, const quint16 &age)
{

	QByteArray nickArray;
	nickArray.append(nick);
	QByteArray firstArray;
	firstArray.append(firstName);
	QByteArray lastArray;
	lastArray.append(lastName);
	
	
	
	if ( findUserWindowOpen )
		searchWin->addFoundedContact(last,founded, uin, codec->toUnicode(nickArray),
				codec->toUnicode(firstArray), codec->toUnicode(lastArray),
				email, authFlag, status, gender, age);
}

void contactListTree::openChatWindowWithFounded(const QString &uin, const QString &nick)
{
	if ( buddyList.contains(uin) )
	{
		doubleClickedBuddy(buddyList.value(uin));
	} else {
		
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
					
		treeGroupItem *group = groupList.value(0);
		treeBuddyItem *buddy;
		if ( showGroups )
			buddy = new treeBuddyItem(icqUin ,statusIconObject->getOfflineIcon(), this, group->offlineList);
		else
			buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, offlineList);
		initializeBuddy(buddy);
		buddy->underline = !dontUnderlineNotAutho;
		buddy->groupID = 0;
		buddy->groupName = group->name;
		group->userCount++;
		group->updateText();
		buddyList.insert(uin, buddy);
		buddy->setBuddyUin(uin);
		buddy->setName(nick);
		buddy->updateBuddyText();
		buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
		if ( showGroups )
			updateNil();
//		requestUinInformation(buddy->getUin());
		settings.beginGroup(buddy->getUin());
		settings.setValue("name", buddy->getUin());
		settings.setValue("groupid", 0);
		settings.setValue("nickname",nick);
		settings.endGroup();
					
		QStringList contacts = settings.value("list/contacts").toStringList();
		contacts<<buddy->getUin();
		settings.setValue("list/contacts", contacts);
					
		if ( contactListChanged  && isMergeAccounts )
			emit reupdateList();
		
		doubleClickedBuddy(buddy);
		
	}
}


void contactListTree::openInfoWindow(const QString &uin, const QString &nick, const QString &firstName, const QString &lastName)
{
	if ( infoWindowList.contains(uin) && uin != icqUin)
		return;
	
	userInformation *infoWin;
	if ( buddyList.contains(uin))
	{
		infoWin= new userInformation(false,true,uin, icqUin);
		
		treeBuddyItem *buddy = buddyList.value(uin);
		
		infoWin->setAdditional(buddy->externalIP, buddy->internalIP,
				buddy->onlineTime, buddy->signonTime, buddy->regTime,
				buddy->clientId, buddy->capabilitiesList, buddy->shortCaps,
				buddy->lastInfoUpdate, buddy->lastExtStatusInfoUpdate,
				buddy->lastExtInfoUpdate, !buddy->isOffline, buddy->protocolVersion,
				buddy->userPort, buddy->Cookie);
		if ( iAmOnline)
			askForFullUserInfo(uin);
	}
	else
	{
		if ( uin == icqUin )
		{
			infoWin = new userInformation(true,true,uin, icqUin);
			selfInfo->setEnabled(false);
		}
		else 
			infoWin= new userInformation(false,false,uin, icqUin);
	}
	infoWin->setAttribute(Qt::WA_QuitOnClose, false);
	infoWin->setAttribute(Qt::WA_DeleteOnClose, true);
	connect( infoWin, SIGNAL(destroyed ( QObject *)),
			this, SLOT(infoUserWindowClosed(QObject *)));
	connect( infoWin, SIGNAL(requestUserInfo(const QString &)),
				this, SLOT(askForFullUserInfo(const QString &)));
	connect( infoWin, SIGNAL(saveOwnerInfo(bool,const QString &)),
				this, SLOT(saveOwnerInfo(bool, const QString &)));

	infoWindowList.insert(uin, infoWin);
	
	if ( !buddyList.contains(uin) )
	{
		
		infoWin->setNick(nick);
		infoWin->setFirst(firstName);
		infoWin->setLast(lastName);
		askForFullUserInfo(uin);
	}
	
	
	infoWin->rellocateDialogToCenter(qutIM::getInstance());
	infoWin->show();
	
	
}

void contactListTree::infoUserWindowClosed(QObject *obj)
{
	userInformation *tempWindow = (userInformation *)(obj);
	
	if ( tempWindow->contactUin == icqUin )
		selfInfo->setEnabled(true);
	
	infoWindowList.remove(infoWindowList.key(tempWindow));
}

void contactListTree::askForFullUserInfo(const QString &uin)
{
	emit incSnacSeq();
	emit incMetaSeq();
	metaInformation metaInfo(icqUin);
	metaInfo.getFullUserInfo(tcpSocket, *flapSeq, *snacSeq, *metaSeq, uin);
	int tmpMetaSeq = ((*metaSeq )% 0x100) * 0x100 + ((*metaSeq )/ 0x100);
	fullInfoRequests.insert(tmpMetaSeq ,uin);
	emit incFlapSeq();
}

void contactListTree::readBasicUserInfo(const metaInformation &metInfo, quint16 metSeqNum)
{

	if ( infoWindowList.contains(fullInfoRequests.value(metSeqNum)) && metInfo.basicInfoSuccess)
	{
		userInformation *infoWin = infoWindowList.value(fullInfoRequests.value(metSeqNum));
		
		infoWin->setNick(codec->toUnicode(metInfo.basicNick));
		infoWin->setFirst(codec->toUnicode(metInfo.basicFirst));
		infoWin->setLast(codec->toUnicode(metInfo.basicLast));
		infoWin->setEmail(codec->toUnicode(metInfo.basicEmail));
		infoWin->setHomeCity(codec->toUnicode(metInfo.basicCity));
		infoWin->setHomeState(codec->toUnicode(metInfo.basicState));
		infoWin->setHomePhone(codec->toUnicode(metInfo.basicPhone));
		infoWin->setHomeFax(codec->toUnicode(metInfo.basicFax));
		infoWin->setHomeAddress(codec->toUnicode(metInfo.basicAddress));
		infoWin->setCell(codec->toUnicode(metInfo.basicCell));
		infoWin->setHomeZip(codec->toUnicode(metInfo.basicZip));
		infoWin->setCountry(metInfo.country);
		infoWin->setAuth(metInfo.basicAuthFlag, metInfo.webAware, metInfo.publishEmail);
		
	}
	
	if ( !metInfo.basicInfoSuccess )
		fullIndoEnd(metSeqNum, false);
}

void contactListTree::fullIndoEnd(quint16 metaSeqNum, bool notAlone)
{
	if ( !notAlone )
	{
		if ( infoWindowList.contains(fullInfoRequests.value(metaSeqNum)))
			infoWindowList.value(fullInfoRequests.value(metaSeqNum))->enableRequestButton();
		fullInfoRequests.remove(metaSeqNum);
		
	}
}

void contactListTree::readMoreUserInfo(const metaInformation &metInfo, quint16 metSeqNum)
{

	if ( infoWindowList.contains(fullInfoRequests.value(metSeqNum)) && metInfo.moreInfoSuccess)
	{
		userInformation *infoWin = infoWindowList.value(fullInfoRequests.value(metSeqNum));
		
		infoWin->setAge(metInfo.moreAge);
		infoWin->setGender(metInfo.moreGender);
		infoWin->setHomePage(codec->toUnicode(metInfo.homepage));
		infoWin->setBirthDay(metInfo.moreBirthYear, metInfo.moreBirthMonth, metInfo.moreBirthDay);
		infoWin->setLang(1, metInfo.moreLang1);
		infoWin->setLang(2, metInfo.moreLang2);
		infoWin->setLang(3, metInfo.moreLang3);
		infoWin->setOriginalCity(codec->toUnicode(metInfo.moreCity));
		infoWin->setOriginalState(codec->toUnicode(metInfo.moreState));
		infoWin->setOriginalCountry(metInfo.moreCountry);
		
		
	}
	
	if ( !metInfo.moreInfoSuccess)
		fullIndoEnd(metSeqNum, false);
}

void contactListTree::readWorkUserInfo(const metaInformation &metInfo, quint16 metSeqNum)
{

	if ( infoWindowList.contains(fullInfoRequests.value(metSeqNum)) && metInfo.workInfoSuccess)
	{
		userInformation *infoWin = infoWindowList.value(fullInfoRequests.value(metSeqNum));
		
		infoWin->setWorkCity(codec->toUnicode(metInfo.workCity));
		infoWin->setWorkState(codec->toUnicode(metInfo.workState));
		infoWin->setWorkPhone(codec->toUnicode(metInfo.workPhone));
		infoWin->setWorkFax(codec->toUnicode(metInfo.workFax));
		infoWin->setWorkAddress(codec->toUnicode(metInfo.workAddress));
		infoWin->setWorkZip(codec->toUnicode(metInfo.workZip));
		infoWin->setWorkCountry(metInfo.workCountry);
		infoWin->setWorkCompany(codec->toUnicode(metInfo.workCompany));
		infoWin->setWorkDepartment(codec->toUnicode(metInfo.workDepartment));
		infoWin->setWorkPosition(codec->toUnicode(metInfo.workPosition));
		infoWin->setWorkOccupation(metInfo.workOccupation);
		
		infoWin->setWorkWebPage(codec->toUnicode(metInfo.workWebPage));

		
	}
	
	if ( !metInfo.workInfoSuccess)
		fullIndoEnd(metSeqNum, false);
}

void contactListTree::readInterestsUserInfo(const metaInformation &metInfo, quint16 metSeqNum)
{

	if ( infoWindowList.contains(fullInfoRequests.value(metSeqNum)) && metInfo.interestsInfoSuccess)
	{
		userInformation *infoWin = infoWindowList.value(fullInfoRequests.value(metSeqNum));
		
		infoWin->setInterests(codec->toUnicode(metInfo.interKeyWords1), metInfo.interCode1, 1);
		infoWin->setInterests(codec->toUnicode(metInfo.interKeyWords2), metInfo.interCode2, 2);
		infoWin->setInterests(codec->toUnicode(metInfo.interKeyWords3), metInfo.interCode3, 3);
		infoWin->setInterests(codec->toUnicode(metInfo.interKeyWords4), metInfo.interCode4, 4);


		
	}
	
	if ( !metInfo.interestsInfoSuccess)
		fullIndoEnd(metSeqNum, false);
}

void contactListTree::readAboutUserInfo(const metaInformation &metInfo, quint16 metSeqNum)
{

	if ( infoWindowList.contains(fullInfoRequests.value(metSeqNum)) && metInfo.aboutInfoSuccess)
	{
		userInformation *infoWin = infoWindowList.value(fullInfoRequests.value(metSeqNum));
		
		infoWin->setAboutInfo(codec->toUnicode(metInfo.about));


		
	}
	
	if ( !metInfo.aboutInfoSuccess)
		fullIndoEnd(metSeqNum, false);
}

void contactListTree::checkStatusFor(const QString &uin)
{
	
	
	
	emit incSnacSeq();
	QByteArray packet;
	packet[0] = 0x2a;
	packet[1] = 0x02;
	packet.append(convertToByteArray((quint16)*flapSeq));
	packet.append(convertToByteArray((quint16)(15 + uin.length())));
	
	snac snac0215;
	snac0215.setFamily(0x0002);
	snac0215.setSubType(0x0015);
	snac0215.setReqId(*snacSeq);
	packet.append(snac0215.getData());
		
	packet.append(convertToByteArray((quint16)0x0000));
	packet.append(convertToByteArray((quint16)0x0005));
	packet[packet.size()] = (quint8)uin.length();
	packet.append(uin);
	tcpSocket->write(packet);
	
	emit incFlapSeq();
	
}

void contactListTree::getStatusCheck(quint16 length)
{
	quint8 uinLength = convertToInt8(socket->read(1));
	length --;
	
	QString uin = socket->read(uinLength);
	
	length -= uinLength; 
	
	socket->read(2);
	
	length -= 2;
	
	quint16 arraySize = byteArrayToInt16(socket->read(2));
	length -= 2;
	bool statusPresent = false;
	tlv tlv06;
	for ( int i = 0; i < arraySize; i++ )
	{
			tlv tmpTlv;
			tmpTlv.readData(socket);
			if ( tmpTlv.getTlvType() == 0x0006)
			{
				tlv06 = tmpTlv;
				statusPresent = true;
			}
			length -= tmpTlv.getLength();
	}
	QString statusString;
	if ( statusPresent )
	{
		
		
		quint16 userstatus = tlv06.getTlvData().at(2) * 0x100 + tlv06.getTlvData().at(3);
		
		switch ( userstatus )
		{
				case 0x0000:
					statusString = convertToStringStatus(contactOnline);
					break;
				case 0x0001:
					statusString = convertToStringStatus(contactAway);
					break;
				case 0x0002:
				case 0x0013:
					statusString = convertToStringStatus(contactDnd);
					break;
				case 0x0004:
				case 0x0005:
					statusString = convertToStringStatus(contactNa);
					break;
				case 0x0010:
				case 0x0011:
					statusString = convertToStringStatus(contactOccupied);
					break;
				case 0x0020:
					statusString = convertToStringStatus(contactFfc);
					break;
				case 0x0100:
					statusString = convertToStringStatus(contactInvisible);
					break;
				case 0x2001:
					statusString = convertToStringStatus(contactLunch);
					break;
				case 0x3000:
					statusString = convertToStringStatus(contactEvil);
					break;		
				case 0x4000:
					statusString = convertToStringStatus(contactDepression);
					break;
				case 0x5000:
					statusString = convertToStringStatus(contactAtHome);
					break;
				case 0x6000:
					statusString = convertToStringStatus(contactAtWork);
					break;
				default:
					statusString = convertToStringStatus(contactOnline);
				}
	} else {
		statusString = tr("is offline");
	}
	
	if ( length )
		socket->read(length);
	
	if ( buddyList.contains(uin))
		emit userMessage(uin, buddyList.value(uin)->getName(), statusString, statusNotyfication, true);
//	else
	if( !buddyList.contains(uin) && !uin.isEmpty() )
	{
		emit userMessage(uin, uin, statusString, statusNotyfication, true);
	}
}

quint8 contactListTree::convertToInt8(const QByteArray &packet)
{
	bool ok;
	return packet.toHex().toUInt(&ok,16);
}

void contactListTree::addUserToList(const QString &uin, const QString &nick, bool authReq)
{
	if ( tcpSocket->state() == QAbstractSocket::ConnectedState )
	{
		
		quint16 groupId = 1;
	if ( buddyList.contains(uin) )
		groupId = buddyList.value(uin)->groupID;
		
	if ( !buddyList.contains(uin) || !groupId)
		{
		addBuddyDialog addDialog;
			addDialog.setTitle(uin);
			QStringList groups;
			foreach (treeGroupItem *group, groupList)
			{
				if ( groupList.key(group))
					groups<<group->name;
			}
			addDialog.setContactData(nick, groups);
			addDialog.rellocateDialogToCenter(qutIM::getInstance());

			
			if ( addDialog.exec() )
			{
				
				
				if ( !groupId)
				{
					QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
							
					QStringList contactList = contacts.value("list/contacts").toStringList();
							
					contactList.removeAll(uin);
					contacts.setValue("list/contacts", contactList);
					
					contacts.remove(uin);
						
					treeBuddyItem *buddy = buddyList.value(uin);
							
							idBuddyList.removeAll(buddy->itemId);
							
							if ( infoWindowList.contains(uin) )
							{
								infoWindowList.value(uin)->close();
								infoWindowList.remove(uin);
							}
							
							if ( historyList.contains(uin) )
							{
								historyList.value(uin)->close();
								historyList.remove(uin);
							}
							
							if ( chatWindowList.contains(uin) )
							{
								chatWindowList.value(uin)->close();
								chatWindowList.remove(uin);
							}
							quint16 groupCount;
							QString groupName;
							treeGroupItem *group = groupList.value(buddy->groupID);
								
								group->buddiesList.removeAll(buddy->itemId);
								
								group->userCount--;
								group->updateText();
								groupCount = group->buddiesList.count();
								groupName = group->name;
								if ( !group->offlineList->childCount() )
									group->offlineList->setHidden(true);
								if ( !group->onlineList->childCount() )
											group->onlineList->setHidden(true);
								if ( !group->userCount && hideEmpty)
									group->setHidden(true);
								if ( isMergeAccounts )
									emit reupdateList();
							buddyList.remove(uin);
							delete buddy;
				}
				
				
				
				
				sendUserAddReq(uin, addDialog.getNick(), authReq, addDialog.getGroup());
			}
		}
	}
	
}

void contactListTree::sendUserAddReq(const QString &uin, QString nick, bool authReq, const QString &groupName)
{
	quint16 groupId = 0;
	foreach(treeGroupItem *group, groupList)
	{
		if ( groupName == group->name)
			groupId = groupList.key(group);
	}
	

	
	if ( !groupId)
		return;
	
	quint16 userId = rand() % 0x03e6;
	for ( ;idBuddyList.contains(userId);)
		userId = rand() % 0x03e6;
	

	
	if (nick.trimmed().isEmpty())
			nick = uin;
	
	
	QByteArray editPack;
	
	emit incSnacSeq();
	QByteArray packet;
	packet[0] = 0x2a;
	packet[1] = 0x02;
	packet.append(convertToByteArray((quint16)*flapSeq));
	if (authReq)
		packet.append(convertToByteArray((quint16)14));
	else
		packet.append(convertToByteArray((quint16)10));
	
	snac snac1311;
	snac1311.setFamily(0x0013);
	snac1311.setSubType(0x0011);
	snac1311.setReqId(*snacSeq);
	packet.append(snac1311.getData());
		
	if ( authReq )
	{
		packet.append(convertToByteArray((quint16)0x0001));
		packet.append(convertToByteArray((quint16)0x0000));
	}
	emit incFlapSeq();
	editPack.append(packet);
	
	emit incSnacSeq();
	QByteArray packet2;
	packet2[0] = 0x2a;
	packet2[1] = 0x02;
	packet2.append(convertToByteArray((quint16)*flapSeq));
	if ( authReq)
		packet2.append(convertToByteArray((quint16)(28 + uin.length() + nick.toUtf8().length())));
	else
		packet2.append(convertToByteArray((quint16)(24 + uin.length() + nick.toUtf8().length())));
		
	snac snac1308;
	snac1308.setFamily(0x0013);
	snac1308.setSubType(0x0008);
	snac1308.setReqId(*snacSeq);
	packet2.append(snac1308.getData());
	packet2.append(convertToByteArray((quint16)uin.length()));
	packet2.append(uin);
	packet2.append(convertToByteArray((quint16)groupId));
	packet2.append(convertToByteArray((quint16)userId));
	
	packet2.append(convertToByteArray((quint16)0x0000));
	
	if ( authReq )
		packet2.append(convertToByteArray((quint16)(8 + nick.toUtf8().length())));
	else
		packet2.append(convertToByteArray((quint16)(4 + nick.toUtf8().length())));
	
	
	
	packet2.append(convertToByteArray((quint16)0x0131));
	packet2.append(convertToByteArray((quint16)nick.toUtf8().length()));
	packet2.append(nick.toUtf8());
	
	if ( authReq )
	{
		packet2.append(convertToByteArray((quint16)0x0066));
		packet2.append(convertToByteArray((quint16)0x0000));
	}
	emit incFlapSeq();
	
	editPack.append(packet2);
	
	
	tcpSocket->write(editPack);
				
	
	modifyObject addBuddy;
	
	addBuddy.itemId = userId;
	addBuddy.groupId = groupId;
	addBuddy.itemType = 0;
	addBuddy.operation = 0;
	addBuddy.buddyUin = uin;
	addBuddy.buddyName = nick;
	addBuddy.authReq = authReq;
	
	modifyReqList.append(addBuddy);
	
//	treeGroupItem *group = groupList.value(0);
//	treeBuddyItem *buddy;
//	if ( showGroups )
//			buddy = new treeBuddyItem(statusIconObject->getOfflineIcon(), this, group->offlineList);
//	else
//			buddy = new treeBuddyItem(statusIconObject->getOfflineIcon(), this, offlineList);
//	initializeBuddy(buddy);
//	if ( authReq)
//		buddy->notAutho = true;
//	buddy->underline = !dontUnderlineNotAutho;
//	buddy->groupID = 0;
//	buddy->groupName = group->name;
//	buddyList.insert(uin, buddy);
//	buddy->setBuddyUin(uin);
//	buddy->setName(nick);
//	buddy->updateBuddyText();
//	buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
//	buddy->setHidden(true);
	
	
		
}

void contactListTree::getModifyItemFromServer(quint16 length)
{
//	socket->read(8);
//	length -= 8;
//	
//	quint16 itemLength = byteArrayToInt16(socket->read(2));
//	length -= 2;
//	
//	QString itemName = QString::fromUtf8(socket->read(itemLength));
//	
//	length -= itemLength;
//
//	quint16 groupId = byteArrayToInt16(socket->read(2));
//	length -= 2;
//	
//	quint16 itemId= byteArrayToInt16(socket->read(2));
//	length -= 2;
//	
//	quint16 itemType = byteArrayToInt16(socket->read(2));
//	length -= 2;
//	
//	quint16 tlvLength = byteArrayToInt16(socket->read(2));
//	length -= 2;
//	
//	tlv nameTlv;
//	bool namePresent = false;
//	
//	for(;tlvLength > 0;)
//	{
//		tlv tmpTlv;
//		tmpTlv.readData(socket);
//		length -= tmpTlv.getLength();
//		tlvLength -= tmpTlv.getLength();
//		
//		if ( tmpTlv.getTlvType() == 0x0131)
//		{
//			nameTlv = tmpTlv;
//			namePresent = true;
//		}
//	}
//	
//	if ( length )
//		socket->read(length);
	
	socket->read(8);
	
	length -= 8;
	
	int count = length / 2;
	for ( int i = 0; i < count; i++)
	{
		quint16 code = byteArrayToInt16(socket->read(2));
		length -= 2;
		if ( modifyReqList.count())
		{
			
			if ( code == 0x0000 )
			{

				modifyObject tmpObject = modifyReqList.at(0);
				
				if ( tmpObject.itemType == 0 && tmpObject.operation == 0)
				{
					addModifiedBuddyToGroup(tmpObject.groupId, tmpObject.itemId, tmpObject.buddyUin, tmpObject.authReq, tmpObject.buddyName);
				}
				
				else if ( tmpObject.itemType == 1 && tmpObject.groupId && tmpObject.operation == 0)
				{
					addNewGroupToRoot(tmpObject.buddyName, tmpObject.groupId);
				}
				
				else if ( tmpObject.itemType == 1 && tmpObject.groupId && tmpObject.operation == 1)
				{
					renameGroupToName(tmpObject.buddyName, tmpObject.groupId);
				}
				
				else if ( tmpObject.itemType == 1 && tmpObject.groupId && tmpObject.operation == 2)
				{
					deleteSelectedGroup(tmpObject.groupId);
				}
				
				else if ( tmpObject.itemType == 0 && tmpObject.operation == 1)
				{
					renameContact(tmpObject.buddyUin, tmpObject.buddyName);
				}
				
				else if ( tmpObject.itemType == 0 && tmpObject.operation == 2)
				{
					removeContact(tmpObject.buddyUin);
				}
				
				else if ( tmpObject.itemType == 0x0014 && tmpObject.operation == 1)
				{
					avatarModified = true;
				}
			}
			if ( code == 0x000e)
			{
				modifyObject tmpObject = modifyReqList.at(0);
				
				if ( tmpObject.itemType == 0 && tmpObject.operation == 0)
				{
					sendUserAddReq(tmpObject.buddyUin, tmpObject.buddyName, true, groupList.value(tmpObject.groupId)->name);
				}
			}
			
			modifyReqList.removeAt(0);
		}
	}
	
	if ( length )
		socket->read(2);
	
	
//	if ( itemType == 0x0000)
//	{
//		if ( buddyList.contains(itemName) )
//		{
//			if ( buddyList.value(itemName)->groupID != groupId)
//				addModifiedBuddyToGroup(groupId, itemId, itemName);
//		}
//	}
	
	
}

void contactListTree::addModifiedBuddyToGroup(quint16 groupId, quint16 itemId, const QString &uin, bool authReq, const QString &nick)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	if ( groupList.contains(groupId))
	{
		
			treeGroupItem *newGrp = groupList.value(groupId);
			treeBuddyItem *buddy;
			if ( showGroups )
				buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, newGrp->offlineList);
			else
				buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, offlineList);
			initializeBuddy(buddy);
			if ( authReq)
				buddy->notAutho = true;
			buddy->underline = !dontUnderlineNotAutho;
			buddy->groupID = groupId;
			buddy->itemId = itemId;
			buddy->groupName = newGrp->name;
			buddyList.insert(uin, buddy);
			newGrp->userCount++;
			newGrp->updateText();
			buddy->setBuddyUin(uin);
			buddy->setName(nick);
			buddy->updateBuddyText();
			buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
		
			idBuddyList.append(itemId);
			

			buddyList.insert(uin, buddy);
			
			if (newGrp->userCount > 0)
			{
				if ( newGrp->offlineList->childCount() )
					{
						if ( !showOffline )
							newGrp->offlineList->setHidden(false);	
							newGrp->offlineList->setExpanded(true);
						}
				if ( showGroups)
					newGrp->setHidden(false);
			}
					if ( !showOffline )
						newGrp->offlineList->sortChildren(1,Qt::AscendingOrder);
		
		
		settings.beginGroup(uin);
		
		settings.setValue("groupid", groupId);
		settings.setValue("name", uin);
		settings.setValue("authorized", !buddy->notAutho);
		settings.setValue("nickname", buddy->getName());
		
		settings.endGroup();
		
		QStringList buddies = settings.value("list/contacts").toStringList();
		
		if ( ! buddies.contains(uin))
		{
						buddies<<uin;
						settings.setValue("list/contacts", buddies);
		}
		
		if ( isMergeAccounts )
			emit reupdateList();
		
		treeGroupItem *editGroup = groupList.value(groupId);
		
		modifyObject groupObject;
		groupObject.groupId = groupId;
		groupObject.itemId = 0x0000;
		groupObject.buddyName = editGroup->name;
		groupObject.itemType = 1;
		groupObject.operation = 3;
		modifyReqList.append(groupObject);
		
		QByteArray editClose;
		int groupItemCount = editGroup->buddiesList.count();
		
		emit incSnacSeq();
		QByteArray packet;
		packet[0] = 0x2a;
		packet[1] = 0x02;
		packet.append(convertToByteArray((quint16)*flapSeq));

		packet.append(convertToByteArray((quint16)(24 + editGroup->name.toUtf8().length() + groupItemCount * 2)));

		snac snac1309;
		snac1309.setFamily(0x0013);
		snac1309.setSubType(0x0009);
		snac1309.setReqId(*snacSeq);
		packet.append(snac1309.getData());
				

			packet.append(convertToByteArray((quint16)editGroup->name.toUtf8().length()));
			packet.append(editGroup->name.toUtf8());
			packet.append(convertToByteArray((quint16)groupId));
			packet.append(convertToByteArray((quint16)0x0000));
			packet.append(convertToByteArray((quint16)0x0001));
			packet.append(convertToByteArray((quint16)(4 + groupItemCount * 2)));
			
			packet.append(convertToByteArray((quint16)0x00c8));
			
			packet.append(convertToByteArray((quint16)(groupItemCount * 2)));
			
			for ( int i = 0; i < groupItemCount; i++)
			{
				packet.append(convertToByteArray((quint16)editGroup->buddiesList.at(i)));
			}
		emit incFlapSeq();
		
		editClose.append(packet);
		emit incSnacSeq();
		QByteArray packet2;
		packet2[0] = 0x2a;
		packet2[1] = 0x02;
		packet2.append(convertToByteArray((quint16)*flapSeq));

		packet2.append(convertToByteArray((quint16)10));

		snac snac1312;
		snac1312.setFamily(0x0013);
		snac1312.setSubType(0x0012);
		snac1312.setReqId(*snacSeq);
		packet2.append(snac1312.getData());
		
		editClose.append(packet2);
		emit incFlapSeq();
		tcpSocket->write(editClose);
		if ( isMergeAccounts )
			emit reupdateList();
		
	} 
}


void contactListTree::youWereAdded(quint16 length)
{
	socket->read(8);
	length -= 8;
	
	quint8 uinLength = convertToInt8(socket->read(1));
	
	length -= 1;
	
	QString uin = socket->read(uinLength);
	
	length -= uinLength;
	
	QString addedMsg(tr("You were added"));
	
	if (!buddyList.contains(uin) && blockAuth)
	{
		if ( notifyAboutBlocked)
			notifyBlockedMessage(uin, addedMsg);
		
		if ( saveServiceHistory )
			saveBlocked(uin, addedMsg, QDateTime::currentDateTime());
		
		
		return;
	}
	
	messageFormat *msg = new messageFormat;
	msg->fromUin = uin;
	msg->message = addedMsg;
	msg->date = QDateTime::currentDateTime();
	
	if ( buddyList.contains(msg->fromUin) )
	{
		treeBuddyItem *buddy = buddyList.value(msg->fromUin);
		msg->from = buddy->getName();
		
		if ( saveHistory )
			historyObject->saveHistoryMessage(buddy->getUin(), 
					buddy->getName(), QDateTime::currentDateTime(), true, msg->message);
		
		
		
	} else {
		
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
		
		treeGroupItem *group = groupList.value(0);
		msg->from = msg->fromUin;
		treeBuddyItem *buddy;
		if ( showGroups )
			buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, group->offlineList);
		else
			buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, offlineList);
		initializeBuddy(buddy);
		buddy->underline = !dontUnderlineNotAutho;
		buddy->groupID = 0;
		buddy->groupName = group->name;
		group->userCount++;
		group->updateText();
		buddyList.insert(msg->fromUin, buddy);
		buddy->setBuddyUin(msg->fromUin);
		buddy->setName(msg->fromUin);
		buddy->updateBuddyText();
		buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
		if ( showGroups )
			updateNil();
		requestUinInformation(buddy->getUin());
		settings.beginGroup(buddy->getUin());
		settings.setValue("name", buddy->getUin());
		settings.setValue("groupid", 0);
		settings.setValue("nickname", buddy->getName());
		settings.endGroup();
		
		QStringList contacts = settings.value("list/contacts").toStringList();
		contacts<<buddy->getUin();
		settings.setValue("list/contacts", contacts);
		
		if ( contactListChanged  && isMergeAccounts )
					emit reupdateList();
		
		if ( saveHistory && saveNilHistory )
		historyObject->saveHistoryMessage(buddy->getUin(), 
				buddy->getName(), QDateTime::currentDateTime() , true, msg->message);
		
	}
		
	
	if ( chatWindowList.contains(msg->fromUin))
	{
		chatWindow *w = chatWindowList.value(msg->fromUin);
		w->setMessage(msg->from,msg->message, msg->date);
		if ( !disableButtonBlinking)
			qApp->alert(w, 0);
		if ( tabMode )
			generalChatWindow->setMessageTab(w);
		if ( !w->isActiveWindow() )
		{
			if ( messageList.contains(msg->fromUin))
			{
				messageList.value(msg->fromUin)->messageList.append(msg);
			} else {
				messageList.insert(msg->fromUin, buddyList.value(msg->fromUin));
				messageList.value(msg->fromUin)->messageList.append(msg);
			}
			newMessages = true;
			QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
			if ( !disableTrayBlinking )
				emit getNewMessage();
			if ( ! dontShowEvents )
				emit userMessage(msg->fromUin, msg->from, msg->message, messageNotification, true);
		}
		
	} else {
		if ( messageList.contains(msg->fromUin))
		{
			messageList.value(msg->fromUin)->messageList.append(msg);
		} else {
			messageList.insert(msg->fromUin, buddyList.value(msg->fromUin));
			messageList.value(msg->fromUin)->messageList.append(msg);
		}
		emit userMessage(msg->fromUin, msg->from, msg->message, messageNotification, true);
		if ( !disableTrayBlinking )
			emit getNewMessage();
	}
	if ( !newMessages )
	{
		newMessages = true;
		QTimer::singleShot(1000,this, SLOT(setMessageIconToContact()));
	}
	
	if ( openNew )
		doubleClickedBuddy(buddyList.value(msg->fromUin));
}

void contactListTree::sendMultipleWindow()
{
	multipleSendingWin = new multipleSending;
	multipleSendingWin->setTreeModel(icqUin, &groupList, &buddyList);
	multipleSendingOpen = true;
	multipleSendingWin->setAttribute(Qt::WA_QuitOnClose, false);
	multipleSendingWin->setAttribute(Qt::WA_DeleteOnClose, true);
		connect( multipleSendingWin, SIGNAL(destroyed ( QObject *)),
				this, SLOT(deleteSendMultipleWindow(QObject *)));
		connect( multipleSendingWin, SIGNAL(sendMessageToContact(const messageFormat &)),
				this, SLOT(sendMessage(const messageFormat &)));
	sendMultiple->setEnabled(false);
	multipleSendingWin->rellocateDialogToCenter(qutIM::getInstance());
	multipleSendingWin->show();
}

void contactListTree::deleteSendMultipleWindow(QObject */*obj*/)
{
	multipleSendingOpen = false;
	sendMultiple->setEnabled(true);
}

void contactListTree::openPrivacyWindow()
{
	privacyWindow = new privacyListWindow(icqUin);
	privacyWindow->setOnline(iAmOnline);
	privacyListWindowOpen = true;
	privacyWindow->setAttribute(Qt::WA_QuitOnClose, false);
	privacyWindow->setAttribute(Qt::WA_DeleteOnClose, true);
		connect( privacyWindow, SIGNAL(destroyed ( QObject *)),
				this, SLOT(deletePrivacyWindow(QObject *)));
		connect( privacyWindow, SIGNAL(openInfo( const QString &, const QString &, const QString &, const QString &)),
						this, SLOT(openInfoWindow(const QString &, const QString &, const QString &, const QString &)));
		connect( privacyWindow, SIGNAL(deleteFromPrivacyList(const QString &, int)),
						this, SLOT(deleteFromPrivacyList(const QString &, int)));
	privacyList->setEnabled(false);
	privacyWindow->rellocateDialogToCenter(qutIM::getInstance());
	privacyWindow->show();
}

void contactListTree::deletePrivacyWindow(QObject */*obj*/)
{
	privacyListWindowOpen = false;
	privacyList->setEnabled(true);
}

void contactListTree::deleteFromPrivacyList(const QString &uin, int type)
{
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	
	emit incSnacSeq();
	QByteArray packet2;
	packet2[0] = 0x2a;
	packet2[1] = 0x02;
	packet2.append(convertToByteArray((quint16)*flapSeq));

	packet2.append(convertToByteArray((quint16)(20 + uin.length())));
	snac snac130a;
	snac130a.setFamily(0x0013);
	snac130a.setSubType(0x000a);
	snac130a.setReqId(*snacSeq);
	packet2.append(snac130a.getData());
			
	
	itemFromList object;
	quint16 objectType = 0x0002;
	
	if (type == 0)
	{
		object = visibleObjectList.value(uin);
		objectType = 0x0002;
		
		visibleList.removeAll(uin);
		visibleObjectList.remove(uin);
		contacts.setValue("list/visible", visibleList);
	}
	else if (type == 1)
	{
		object = invisibleObjectList.value(uin);
		objectType = 0x0003;
		invisibleList.removeAll(uin);
		invisibleObjectList.remove(uin);
		contacts.setValue("list/invisible", invisibleList);
	}
	else if (type == 2)
	{
		object = ignoreObjectList.value(uin);
		objectType = 0x000e;
		ignoreList.removeAll(uin);
		ignoreObjectList.remove(uin);
		contacts.setValue("list/ignore", ignoreList);
	}

	packet2.append(convertToByteArray((quint16)uin.length()));
	packet2.append(uin);
	
	packet2.append(convertToByteArray((quint16)object.groupId));
	packet2.append(convertToByteArray((quint16)object.itemId));
	packet2.append(convertToByteArray((quint16)objectType));
	
	modifyObject addBuddy;
	
	addBuddy.itemId = object.itemId;
	addBuddy.groupId = object.groupId;
	addBuddy.itemType = objectType;
	addBuddy.operation = 2;
	addBuddy.buddyUin = uin;
	
	modifyReqList.append(addBuddy);
	
	packet2.append(convertToByteArray((quint16)0x0000));
	emit incFlapSeq();
	tcpSocket->write(packet2);
}

void contactListTree::openSelfInfo()
{
	openInfoWindow(icqUin);
}

void contactListTree::saveOwnerInfo(bool saveAvatar, const QString &avatarPath)
{
	if ( infoWindowList.contains(icqUin))
	{
		userInformation *win = infoWindowList.value(icqUin);
		emit incSnacSeq();
		emit incMetaSeq();
		metaInformation metaInfo(icqUin);
		metaInfo.basicNick = codec->fromUnicode(win->getNick());
		metaInfo.basicFirst = codec->fromUnicode(win->getFirst());
		metaInfo.basicLast = codec->fromUnicode(win->getLast());
		metaInfo.basicEmail = codec->fromUnicode(win->getEmail());
		metaInfo.publishEmail = win->getPublish();
		metaInfo.country = win->getHomeCountry();
		
		metaInfo.basicCity = codec->fromUnicode(win->getHomeCity());
		metaInfo.basicState = codec->fromUnicode(win->getHomeState());
		metaInfo.zip = win->getHomeZip();
		
		metaInfo.basicPhone = codec->fromUnicode(win->getHomePhone());
		metaInfo.basicFax = codec->fromUnicode(win->getHomeFax());
		metaInfo.basicCell = codec->fromUnicode(win->getCellular());
		metaInfo.basicAddress = codec->fromUnicode(win->getHomeStreet());
		
		metaInfo.moreCountry = win->getOrigCountry();
		metaInfo.moreCity = codec->fromUnicode(win->getOrigCity());
		metaInfo.moreState = codec->fromUnicode(win->getOrigState());
		
		metaInfo.workCountry = win->getWorkCountry();
		metaInfo.workCity = codec->fromUnicode(win->getWorkCity());
		metaInfo.workState = codec->fromUnicode(win->getWorkState());
		metaInfo.wzip = win->getWorkZip();
		
		metaInfo.workPhone = codec->fromUnicode(win->getWorkPhone());
		metaInfo.workFax = codec->fromUnicode(win->getWorkFax());
		metaInfo.workAddress = codec->fromUnicode(win->getWorkStreet());
		
		metaInfo.workCompany = codec->fromUnicode(win->getCompanyName());
		metaInfo.workOccupation = win->getOccupation();
		metaInfo.workDepartment = codec->fromUnicode(win->getDepartment());
		metaInfo.workPosition = codec->fromUnicode(win->getPosition());
		metaInfo.workWebPage = codec->fromUnicode(win->getWebPage());
		
		metaInfo.foundedGender = win->getGender();
		metaInfo.homepage = codec->fromUnicode(win->getHomePage());
		metaInfo.setBirth = win->sendBirth();
		QDate birth = win->getBirth();
		
		metaInfo.moreBirthYear = birth.year();
		metaInfo.moreBirthMonth = birth.month();
		metaInfo.moreBirthDay = birth.day();
		
		metaInfo.moreLang1 = win->getLang1();
		metaInfo.moreLang2 = win->getLang2();
		metaInfo.moreLang3 = win->getLang3();
		
		
		metaInfo.interCode1 = win->getInterests(1);
		metaInfo.interKeyWords1 = codec->fromUnicode(win->getInterestString(1));
		
		
		metaInfo.interCode2 = win->getInterests(2);
		metaInfo.interKeyWords2 = codec->fromUnicode(win->getInterestString(2));
		
		metaInfo.interCode3 = win->getInterests(3);
		metaInfo.interKeyWords3 = codec->fromUnicode(win->getInterestString(3));
		
		metaInfo.interCode4 = win->getInterests(4);
		metaInfo.interKeyWords4 = codec->fromUnicode(win->getInterestString(4));
		
		metaInfo.about = codec->fromUnicode(win->getAbout());
		
		metaInfo.authFlag = win->getAuth();
		metaInfo.webAware = win->webAware();
		
		metaInfo.saveOwnerInfo(tcpSocket, *flapSeq, *snacSeq, *metaSeq);
		

		
		emit incFlapSeq();
//		
//		emit incSnacSeq();
//		emit incMetaSeq();
//		metaInfo.sendMoreInfo(tcpSocket, *flapSeq, *snacSeq, *metaSeq);
//		emit incFlapSeq();
		
		if ( !waitForIconUpload && saveAvatar)
		{
			waitForIconUpload = saveAvatar;
			ownIconPath = avatarPath;
			uploadIcon();
		}
	}
}

void contactListTree::checkForOwnIcon(QByteArray avatarArray)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	QByteArray icon = avatarArray.right(16);
	
	if ( icon.length() == 16)
	{
		if ( settings.value("main/iconhash").toByteArray() != icon.toHex() )
		{
			
			askForAvatars(icon, icqUin);
			settings.setValue("main/iconhash", icon.toHex());
		}
	}
}

void contactListTree::uploadIcon()
{
	
	
	
	if ( QFile::exists(ownIconPath))
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
		QFile pic(ownIconPath);
		if ( pic.open(QIODevice::ReadOnly))
		{
			QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(pic.readAll());
		
		QByteArray hashArray = hash.result();
		settings.setValue("main/iconhash",hashArray.toHex());
		pic.copy(iconPath + hashArray.toHex() );
		
		bool iconExistsOnServer = true;
		if ( !iconObject.itemId)
		{
			iconExistsOnServer = false;
			iconObject.itemId = rand() % 0x03e6;
		}
			emit incSnacSeq();
			QByteArray packet;
			packet[0] = 0x2a;
			packet[1] = 0x02;
			packet.append(convertToByteArray((quint16)*flapSeq));
			packet.append(convertToByteArray((quint16)43));

			
			snac snac1308;
			snac1308.setFamily(0x0013);
			if (iconExistsOnServer)
				snac1308.setSubType(0x0009);
			else
				snac1308.setSubType(0x0008);
			snac1308.setReqId(*snacSeq);
			packet.append(snac1308.getData());
			
			packet.append(convertToByteArray((quint16)0x0001));
			packet.append(QString("1"));
			
			packet.append(convertToByteArray((quint16)iconObject.groupId));
			packet.append(convertToByteArray((quint16)iconObject.itemId));
			packet.append(convertToByteArray((quint16)0x0014));
			packet.append(convertToByteArray((quint16)22));
			
			packet.append(convertToByteArray((quint16)0x00d5));
			packet.append(convertToByteArray((quint16)0x0012));
			
			packet.append(convertToByteArray((quint16)0x0110));
			packet.append(hashArray);
			
			
			
			modifyObject addBuddy;
			
			addBuddy.itemId = iconObject.itemId;
			addBuddy.groupId = iconObject.groupId;
			addBuddy.itemType = 0x0014;
			addBuddy.operation = 1;
			addBuddy.buddyName = "1";
			
			modifyReqList.append(addBuddy);
			
			tcpSocket->write(packet);
			
			emit incFlapSeq();
		}
	} else {

		bool iconExistsOnServer = true;
		if ( !iconObject.itemId)
		{
			iconExistsOnServer = false;
			iconObject.itemId = rand() % 0x03e6;
		}
			emit incSnacSeq();
			QByteArray packet;
			packet[0] = 0x2a;
			packet[1] = 0x02;
			packet.append(convertToByteArray((quint16)*flapSeq));
			packet.append(convertToByteArray((quint16)32));

			
			snac snac1308;
			snac1308.setFamily(0x0013);
			if (iconExistsOnServer)
				snac1308.setSubType(0x0009);
			else
				snac1308.setSubType(0x0008);
			snac1308.setReqId(*snacSeq);
			packet.append(snac1308.getData());
			
			packet.append(convertToByteArray((quint16)0x0001));
			packet.append(QString("1"));
			
			packet.append(convertToByteArray((quint16)iconObject.groupId));
			packet.append(convertToByteArray((quint16)iconObject.itemId));
			packet.append(convertToByteArray((quint16)0x0014));
			packet.append(convertToByteArray((quint16)11));
			
			packet.append(convertToByteArray((quint16)0x00d5));
			packet.append(convertToByteArray((quint16)0x0007));
			packet.append(convertToByteArray((quint16)0x0005));
			packet.append(convertToByteArray((quint16)0x0201));
			packet.append(convertToByteArray((quint16)0xd204));
			packet.append(QChar(0x72));
			
			
			modifyObject addBuddy;
			
			addBuddy.itemId = iconObject.itemId;
			addBuddy.groupId = iconObject.groupId;
			addBuddy.itemType = 0x0014;
			addBuddy.operation = 1;
			addBuddy.buddyName = "1";
			
			modifyReqList.append(addBuddy);
			
			tcpSocket->write(packet);
			
			emit incFlapSeq();
			removeIconHash();
			
			
	}
}

void contactListTree::getUploadIconData(quint16 length)
{
	socket->read(8);
	length -= 8;
	
	
	quint16 noticeType = byteArrayToInt16(socket->read(2));
	length -= 2;
	
	socket->read(length);
	
	if ( noticeType == 1 && avatarModified)
	{
		avatarModified = false;
		QHostAddress hostAddr = QHostAddress(avatarAddress);
		if ( !hostAddr.isNull() && avatarCookie.length() == 256)
		{
			if ( !buddyConnection->connectedToServ )
				buddyConnection->connectToServ(avatarAddress, avatarPort, avatarCookie, tcpSocket->proxy());
			else
			{
				
				if ( buddyConnection->canSendReqForAvatars )
				{
					waitForIconUpload = false;
					buddyConnection->uploadIcon(ownIconPath);
				}
			}
		}
	}
}

void contactListTree::removeIconHash()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.remove("main/iconhash");
}

void contactListTree::openChangePasswordDialog()
{
	passwordChangeDialog dialog(icqUin);
	
	if ( dialog.exec() )
	{
		emit incSnacSeq();
		emit incMetaSeq();
		metaInformation metaInfo(icqUin);
		metaInfo.changePassword(tcpSocket, *flapSeq, *snacSeq, *metaSeq, dialog.newPass);
		emit incFlapSeq();
	}
}

void contactListTree::showGroupMenu(treeGroupItem *group)
{

	currentContextGroup = group;
	
	currentContextMenu->clear();
	currentContextMenu->addAction(menuTitle);
	menuLabel->setText("<b>"+ group->name +"</b>");
	if ( iAmOnline)
	{
		currentContextMenu->addAction(createGroup);
		
		if ( groupList.key(group))
		{
			currentContextMenu->addAction(renameGroup);
			currentContextMenu->addAction(deleteGroup);
		}
	}
	currentContextMenu->popup(contactList->mapToGlobal(contactList->mapFromGlobal(QCursor::pos())));
	
	
}

void contactListTree::createContactListActions()
{
	currentContextMenu = new QMenu;
	
	
	menuLabel = new QLabel;
	menuLabel->setAlignment(Qt::AlignCenter);
	menuTitle = new QWidgetAction(this);
	menuTitle->setDefaultWidget(menuLabel);
	

	
	createGroup = new QAction(QIcon(":/icons/crystal_project/add.png"), tr("New group"), this);
	connect(createGroup, SIGNAL(triggered()), this, SLOT(createNewGroup()));
	
	renameGroup = new QAction(QIcon(":/icons/crystal_project/clear.png"), tr("Rename group"), this);
	connect(renameGroup, SIGNAL(triggered()), this, SLOT(renameSelectedGroup()));
	
	deleteGroup = new QAction(QIcon(":/icons/crystal_project/remove.png"), tr("Delete group"), this);
	connect(deleteGroup, SIGNAL(triggered()), this, SLOT(deleteSelectedGroup()));
	
	sendMessageAction = new QAction(statusIconClass::getInstance()->getContentIcon(), tr("Send message"), this);
	connect(sendMessageAction, SIGNAL(triggered()), this, SLOT(sendMessageActionTriggered()));
	
	userInformationAction = new QAction(QIcon(":/icons/crystal_project/contactinfo.png"), tr("Contact details"), this);
	connect(userInformationAction, SIGNAL(triggered()), this, SLOT(userInformationActionTriggered()));
	
	copyUINAction = new QAction(QIcon(":/icons/crystal_project/copy_uin.png"), tr("Copy UIN to clipboard"), this);
	connect(copyUINAction, SIGNAL(triggered()), this, SLOT(copyUINActionTriggered()));
	
	statusCheckAction= new QAction(QIcon(":/icons/crystal_project/checkstat.png"), tr("Contact status check"), this);
	connect(statusCheckAction, SIGNAL(triggered()), this, SLOT(statusCheckActionTriggered()));
	
	messageHistoryAction = new QAction(QIcon(":/icons/crystal_project/history.png"), tr("Message history"), this);
	connect(messageHistoryAction, SIGNAL(triggered()), this, SLOT(messageHistoryActionTriggered()));
	
	readAwayAction = new QAction(QIcon(":/icons/crystal_project/readaway.png"), tr("Read away message"), this);
	connect(readAwayAction, SIGNAL(triggered()), this, SLOT(readAwayActionTriggered()));
	
	renameContactAction = new QAction(QIcon(":/icons/crystal_project/edituser.png"), tr("Rename contact"), this);
	connect(renameContactAction, SIGNAL(triggered()), this, SLOT(renameContactActionTriggered()));
	
	deleteContactAction = new QAction(QIcon(":/icons/crystal_project/deleteuser.png"), tr("Delete contact"), this);
	connect(deleteContactAction, SIGNAL(triggered()), this, SLOT(deleteContactActionTriggered()));
	
	moveContactAction = new QAction(QIcon(":/icons/crystal_project/moveuser.png"), tr("Move to group"), this);
	connect(moveContactAction, SIGNAL(triggered()), this, SLOT(moveContactActionTriggered()));
	
	addToVisibleAction = new QAction(QIcon(":/icons/crystal_project/visible.png"), tr("Add to visible list"), this);
	connect(addToVisibleAction, SIGNAL(triggered()), this, SLOT(addToVisibleActionTriggered()));
	
	addToInvisibleAction = new QAction(QIcon(":/icons/crystal_project/privacy.png"), tr("Add to invisible list"), this);
	connect(addToInvisibleAction, SIGNAL(triggered()), this, SLOT(addToInvisibleActionTriggered()));
	
	addToIgnoreAction = new QAction(QIcon(":/icons/ignorelist.png"), tr("Add to ignore list"), this);
	connect(addToIgnoreAction , SIGNAL(triggered()), this, SLOT(addToIgnoreActionTriggered()));
	
	deleteFromVisibleAction = new QAction(QIcon(":/icons/crystal_project/visible.png"), tr("Delete from visible list"), this);
	connect(deleteFromVisibleAction, SIGNAL(triggered()), this, SLOT(deleteFromVisibleActionTriggered()));
	
	deleteFromInvisibleAction = new QAction(QIcon(":/icons/crystal_project/privacy.png"), tr("Delete from invisible list"), this);
	connect(deleteFromInvisibleAction, SIGNAL(triggered()), this, SLOT(deleteFromInvisibleActionTriggered()));
	
	deleteFromIgnoreAction = new QAction(QIcon(":/icons/ignorelist.png"), tr("Delete from ignore list"), this);
	connect(deleteFromIgnoreAction , SIGNAL(triggered()), this, SLOT(deleteFromIgnoreActionTriggered()));

	// TO-DO: Investigate a better way of adding auth icon
	requestAuthorizationAction = new QAction(statusIconClass::getInstance()->getConnectingIcon(), tr("Authorization request"), this);
	connect(requestAuthorizationAction , SIGNAL(triggered()), this, SLOT(requestAuthorizationActionTriggered()));
	
	addToContactListAction = new QAction(QIcon(":/icons/crystal_project/add_user.png"), tr("Add to contact list"), this);
	connect(addToContactListAction , SIGNAL(triggered()), this, SLOT(addToContactListActionTriggered()));
	
	allowToAddMe = new QAction(QIcon(":/icons/crystal_project/apply.png"), tr("Allow contact to add me"), this);
	connect(allowToAddMe , SIGNAL(triggered()), this, SLOT(allowToAddMeTriggered()));
	
	removeMyself = new QAction(QIcon(":/icons/crystal_project/deletetab2.png"), tr("Remove myself from contact's list"), this);
	connect(removeMyself , SIGNAL(triggered()), this, SLOT(removeMyselfTriggered()));
	
	connect(fileTransferObject->getSendFileAction() , SIGNAL(triggered()), this, SLOT(sendFileActionTriggered()));
	
	readXstatus = new QAction(QIcon(":/icons/crystal_project/xstatus.png"), tr("Read custom status"), this);
	connect(readXstatus , SIGNAL(triggered()), this, SLOT(readXstatusTriggered()));
	
	editNoteAction = new QAction(QIcon(":/icons/crystal_project/note.png"), tr("Edit note"), this);
	connect(editNoteAction , SIGNAL(triggered()), this, SLOT(editNoteActionTriggered()));
}

void contactListTree::createNewGroup()
{
	addRenameDialog dialog;
	
	dialog.setWindowTitle(tr("Create group"));
	
	if ( dialog.exec() )
	{
		quint16 groupId = rand() % 0x03e6;
		for ( ;groupList.contains(groupId);)
			groupId = rand() % 0x03e6;
		
		
		QString groupName = dialog.name;
		QByteArray editPack;
			
			emit incSnacSeq();
			QByteArray packet;
			packet[0] = 0x2a;
			packet[1] = 0x02;
			packet.append(convertToByteArray((quint16)*flapSeq));
			packet.append(convertToByteArray((quint16)10));
			
			snac snac1311;
			snac1311.setFamily(0x0013);
			snac1311.setSubType(0x0011);
			snac1311.setReqId(*snacSeq);
			packet.append(snac1311.getData());
			emit incFlapSeq();
			editPack.append(packet);
			
			emit incSnacSeq();
			QByteArray packet2;
			packet2[0] = 0x2a;
			packet2[1] = 0x02;
			packet2.append(convertToByteArray((quint16)*flapSeq));
			packet2.append(convertToByteArray((quint16)(24 + groupName.toUtf8().length())));
				
			snac snac1308;
			snac1308.setFamily(0x0013);
			snac1308.setSubType(0x0008);
			snac1308.setReqId(*snacSeq);
			packet2.append(snac1308.getData());
			
			
			packet2.append(convertToByteArray((quint16)groupName.toUtf8().length()));
			packet2.append(groupName.toUtf8());
			
			packet2.append(convertToByteArray((quint16)groupId));
			packet2.append(convertToByteArray((quint16)0x0000));
			
			packet2.append(convertToByteArray((quint16)0x0001));
			
			
			packet2.append(convertToByteArray((quint16)0x0004));
			packet2.append(convertToByteArray((quint16)0x00c8));
			packet2.append(convertToByteArray((quint16)0x0000));
			
			emit incFlapSeq();
			
			editPack.append(packet2);
			
			
			tcpSocket->write(editPack);
						
			
			modifyObject addGroup;
			
			addGroup.itemId = 0;
			addGroup.groupId = groupId;
			addGroup.itemType = 1;
			addGroup.operation = 0;
			addGroup.buddyName = groupName;
			
			modifyReqList.append(addGroup);
	}
}

void contactListTree::renameSelectedGroup()
{
	addRenameDialog dialog;
		
		dialog.setWindowTitle(tr("Rename group"));
		
		if ( dialog.exec() )
		{
			
			quint16 groupId = groupList.key(currentContextGroup);
			QString groupName = dialog.name;
			QByteArray editPack;
				
				emit incSnacSeq();
				QByteArray packet;
				packet[0] = 0x2a;
				packet[1] = 0x02;
				packet.append(convertToByteArray((quint16)*flapSeq));
				packet.append(convertToByteArray((quint16)10));
				
				snac snac1311;
				snac1311.setFamily(0x0013);
				snac1311.setSubType(0x0011);
				snac1311.setReqId(*snacSeq);
				packet.append(snac1311.getData());
				emit incFlapSeq();
				editPack.append(packet);
				
				emit incSnacSeq();
				QByteArray packet2;
				packet2[0] = 0x2a;
				packet2[1] = 0x02;
				packet2.append(convertToByteArray((quint16)*flapSeq));
				packet2.append(convertToByteArray((quint16)(24 + groupName.toUtf8().length() + currentContextGroup->buddiesList.count() * 2)));
					
				snac snac1308;
				snac1308.setFamily(0x0013);
				snac1308.setSubType(0x0009);
				snac1308.setReqId(*snacSeq);
				packet2.append(snac1308.getData());
				
				
				packet2.append(convertToByteArray((quint16)groupName.toUtf8().length()));
				packet2.append(groupName.toUtf8());
				
				packet2.append(convertToByteArray((quint16)groupId));
				packet2.append(convertToByteArray((quint16)0x0000));
				
				packet2.append(convertToByteArray((quint16)0x0001));
				
				
				packet2.append(convertToByteArray((quint16)(0x0004 + currentContextGroup->buddiesList.count() * 2)));
				packet2.append(convertToByteArray((quint16)0x00c8));
				packet2.append(convertToByteArray((quint16)currentContextGroup->buddiesList.count() * 2));
				
				foreach(quint16 listgroupId, currentContextGroup->buddiesList)
				{
					if ( listgroupId)
						packet2.append(convertToByteArray((quint16)listgroupId));
				}
				
				emit incFlapSeq();
				
				editPack.append(packet2);
				
				
				tcpSocket->write(editPack);
							
				
				modifyObject addGroup;
				
				addGroup.itemId = 0;
				addGroup.groupId = groupId;
				addGroup.itemType = 1;
				addGroup.operation = 1;
				addGroup.buddyName = groupName;
				
				modifyReqList.append(addGroup);
		}
}

void contactListTree::deleteSelectedGroup()
{
	QString groupName = currentContextGroup->name;
	quint16 groupId = groupList.key(currentContextGroup);
	QByteArray editPack;
		
		emit incSnacSeq();
		QByteArray packet;
		packet[0] = 0x2a;
		packet[1] = 0x02;
		packet.append(convertToByteArray((quint16)*flapSeq));
		packet.append(convertToByteArray((quint16)10));
		
		snac snac1311;
		snac1311.setFamily(0x0013);
		snac1311.setSubType(0x0011);
		snac1311.setReqId(*snacSeq);
		packet.append(snac1311.getData());
		emit incFlapSeq();
		editPack.append(packet);
		
		emit incSnacSeq();
		QByteArray packet2;
		packet2[0] = 0x2a;
		packet2[1] = 0x02;
		packet2.append(convertToByteArray((quint16)*flapSeq));
		packet2.append(convertToByteArray((quint16)(24 + groupName.toUtf8().length())));
			
		snac snac130a;
		snac130a.setFamily(0x0013);
		snac130a.setSubType(0x000a);
		snac130a.setReqId(*snacSeq);
		packet2.append(snac130a.getData());
		
		
		packet2.append(convertToByteArray((quint16)groupName.toUtf8().length()));
		packet2.append(groupName.toUtf8());
		
		packet2.append(convertToByteArray((quint16)groupId));
		packet2.append(convertToByteArray((quint16)0x0000));
		
		packet2.append(convertToByteArray((quint16)0x0001));
		
		
		packet2.append(convertToByteArray((quint16)0x0004));
		packet2.append(convertToByteArray((quint16)0x00c8));
		packet2.append(convertToByteArray((quint16)0x0000));
		
		emit incFlapSeq();
		
		editPack.append(packet2);
		
		
		tcpSocket->write(editPack);
					
		
		modifyObject delGroup;
		
		delGroup.itemId = 0;
		delGroup.groupId = groupId;
		delGroup.itemType = 1;
		delGroup.operation = 2;
		delGroup.buddyName = groupName;
		
		modifyReqList.append(delGroup);
		
		
}



void contactListTree::addNewGroupToRoot(const QString &name, quint16 groupID)
{
	
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	contacts.beginGroup(QString::number(groupID));
	
	treeGroupItem *group = new treeGroupItem(this, rootItem);
	group->setOnOffLists();
	group->userCount = 1;
	group->setExpanded(true);
	group->userCount = 0;
	group->onlineList->setHidden(true);
	group->offlineList->setHidden(true);
	groupList.insert(groupID, group);
	group->setGroupText(name);
	group->setCustomFont(grpFont.fontFamily, grpFont.fontSize, grpFont.fontColor);
	contacts.setValue("id", groupID);
	contacts.setValue("name", name);
	if ( hideEmpty )
		group->setHidden(true);
	
	contacts.endGroup();
	
	QStringList groups = contacts.value("list/group").toStringList();
	groups<<QString::number(groupID);
	contacts.setValue("list/group", groups);
	
	
	quint16 groupCount = groupList.count();
	if ( groupList.contains(0) )
		groupCount--;
	
	QByteArray editPack;
		

		
		emit incSnacSeq();
		QByteArray packet2;
		packet2[0] = 0x2a;
		packet2[1] = 0x02;
		packet2.append(convertToByteArray((quint16)*flapSeq));
		packet2.append(convertToByteArray((quint16)(24 + groupCount * 2)));
			
		snac snac1308;
		snac1308.setFamily(0x0013);
		snac1308.setSubType(0x0009);
		snac1308.setReqId(*snacSeq);
		packet2.append(snac1308.getData());
		
		
		packet2.append(convertToByteArray((quint16)0));
		
		packet2.append(convertToByteArray((quint16)0));
		packet2.append(convertToByteArray((quint16)0));
		
		packet2.append(convertToByteArray((quint16)0x0001));
		
		
		packet2.append(convertToByteArray((quint16)(4 + groupCount * 2)));
		packet2.append(convertToByteArray((quint16)0x00c8));
		packet2.append(convertToByteArray((quint16)(groupCount * 2)));
		
		foreach(quint16 listgroupId, groupList.keys())
		{
			if ( listgroupId)
				packet2.append(convertToByteArray((quint16)listgroupId));
		}
		
		
		emit incFlapSeq();
		
		editPack.append(packet2);
		
		
		emit incSnacSeq();
		QByteArray packet;
		packet[0] = 0x2a;
		packet[1] = 0x02;
		packet.append(convertToByteArray((quint16)*flapSeq));
		packet.append(convertToByteArray((quint16)10));
		
		snac snac1312;
		snac1312.setFamily(0x0013);
		snac1312.setSubType(0x0012);
		snac1312.setReqId(*snacSeq);
		packet.append(snac1312.getData());
		emit incFlapSeq();
		editPack.append(packet);
		
		
		tcpSocket->write(editPack);
					
		
		modifyObject addGroup;
		
		addGroup.itemId = 0;
		addGroup.groupId = 0;
		addGroup.itemType = 1;
		addGroup.operation = 1;
		
		modifyReqList.append(addGroup);
		
		if ( isMergeAccounts )
			emit reupdateList();
}

void contactListTree::renameGroupToName(const QString &name, quint16 groupId)
{
	treeGroupItem *group = groupList.value(groupId);
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	contacts.setValue(QString::number(groupId) + "/name", name);
	
	group->setGroupText(name);
	
	if ( isMergeAccounts )
		emit reupdateList();
	
}

void contactListTree::deleteSelectedGroup(quint16 groupId)
{
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	contacts.remove(QString::number(groupId));
		
		

		
		groupList.value(groupId)->onlineList->takeChildren();
		groupList.value(groupId)->offlineList->takeChildren();
		delete groupList.value(groupId);
		groupList.remove(groupId);

		QStringList buddiesList = contacts.value("list/contacts").toStringList();
		
		foreach(treeBuddyItem *buddy, buddyList)
		{
			if ( buddy->groupID == groupId)
			{
				buddiesList.removeAll(buddy->getUin());
				contacts.remove(buddy->getUin());
			}
		}
		
		contacts.setValue("list/contacts", buddiesList);
		QStringList groups = contacts.value("list/group").toStringList();
		groups.removeAll(QString::number(groupId));
		contacts.setValue("list/group", groups);
		
		
		quint16 groupCount = groupList.count();
		if ( groupList.contains(0) )
			groupCount--;
		
		QByteArray editPack;
			

			
			emit incSnacSeq();
			QByteArray packet2;
			packet2[0] = 0x2a;
			packet2[1] = 0x02;
			packet2.append(convertToByteArray((quint16)*flapSeq));
			packet2.append(convertToByteArray((quint16)(24 + groupCount * 2)));
				
			snac snac1308;
			snac1308.setFamily(0x0013);
			snac1308.setSubType(0x0009);
			snac1308.setReqId(*snacSeq);
			packet2.append(snac1308.getData());
			
			
			packet2.append(convertToByteArray((quint16)0));
			
			packet2.append(convertToByteArray((quint16)0));
			packet2.append(convertToByteArray((quint16)0));
			
			packet2.append(convertToByteArray((quint16)0x0001));
			
			
			packet2.append(convertToByteArray((quint16)(4 + groupCount * 2)));
			packet2.append(convertToByteArray((quint16)0x00c8));
			packet2.append(convertToByteArray((quint16)(groupCount* 2)));
			
			foreach(quint16 listgroupId, groupList.keys())
			{
				if ( listgroupId)
					packet2.append(convertToByteArray((quint16)listgroupId));
			}
			
			
			emit incFlapSeq();
			
			editPack.append(packet2);
			
			
			emit incSnacSeq();
			QByteArray packet;
			packet[0] = 0x2a;
			packet[1] = 0x02;
			packet.append(convertToByteArray((quint16)*flapSeq));
			packet.append(convertToByteArray((quint16)10));
			
			snac snac1312;
			snac1312.setFamily(0x0013);
			snac1312.setSubType(0x0012);
			snac1312.setReqId(*snacSeq);
			packet.append(snac1312.getData());
			emit incFlapSeq();
			editPack.append(packet);
			
			
			tcpSocket->write(editPack);
						
			
			modifyObject addGroup;
			
			addGroup.itemId = 0;
			addGroup.groupId = 0;
			addGroup.itemType = 1;
			addGroup.operation = 1;
			
			modifyReqList.append(addGroup);
			
			if ( isMergeAccounts )
				emit reupdateList();
}

void contactListTree::showBuddyMenu(treeBuddyItem *buddy)
{
	currentContextBuddy = buddy;
	
	currentContextMenu->clear();
	currentContextMenu->addAction(menuTitle);
	menuLabel->setText("<b>"+ buddy->getName() +"</b>");
	
	

	
	currentContextMenu->addAction(sendMessageAction);
	if ( iAmOnline )
	{
		if ( currentContextBuddy->getStatus() != contactOffline)
		{
			if ( currentContextBuddy->fileTransferSupport )
				currentContextMenu->addAction(fileTransferObject->getSendFileAction());
		}
		
		if ( currentContextBuddy->notAutho )
		{
			currentContextMenu->addAction(requestAuthorizationAction);

		}
				
		if ( currentContextBuddy->getStatus() == contactOffline )
		{
			currentContextMenu->addAction(statusCheckAction);
		}
		if ( currentContextBuddy->getStatus() != contactOffline &&
				currentContextBuddy->getStatus() != contactOnline &&
				currentContextBuddy->getStatus() != contactInvisible )
		{
			currentContextMenu->addAction(readAwayAction);
		}
		
		if ( currentContextBuddy->xStatusPresent)
			currentContextMenu->addAction(readXstatus);
		
		
	}
	
	currentContextMenu->addAction(copyUINAction);
	currentContextMenu->addAction(editNoteAction);
	currentContextMenu->addAction(userInformationAction);
	currentContextMenu->addAction(messageHistoryAction);
	
	if ( noteWidgetsList.contains(currentContextBuddy->getUin()))
		editNoteAction->setEnabled(false);
	else
		editNoteAction->setEnabled(true);
	
	if ( iAmOnline &&  currentContextBuddy->groupID)
	{
		currentContextMenu->addSeparator();
		currentContextMenu->addAction(renameContactAction);
		currentContextMenu->addAction(deleteContactAction);
		currentContextMenu->addAction(moveContactAction);
		currentContextMenu->addSeparator();
		if ( !visibleList.contains(currentContextBuddy->getUin()))
			currentContextMenu->addAction(addToVisibleAction);
		else
			currentContextMenu->addAction(deleteFromVisibleAction);
		if ( !invisibleList.contains(currentContextBuddy->getUin()))
			currentContextMenu->addAction(addToInvisibleAction);
		else
			currentContextMenu->addAction(deleteFromInvisibleAction);
		if ( !ignoreList.contains(currentContextBuddy->getUin()))
			currentContextMenu->addAction(addToIgnoreAction);
		else
			currentContextMenu->addAction(deleteFromIgnoreAction);
		currentContextMenu->addAction(allowToAddMe);
		currentContextMenu->addAction(removeMyself);
	}
	
	if ( !currentContextBuddy->groupID )
	{
		currentContextMenu->addAction(deleteContactAction);
		if ( iAmOnline)
		{
			currentContextMenu->addAction(addToContactListAction);
			currentContextMenu->addAction(addToIgnoreAction);
			currentContextMenu->addAction(allowToAddMe);
			currentContextMenu->addAction(removeMyself);
		}
	}
	
	if ( iAmOnline )
		currentContextMenu->addAction(createGroup);
	
	currentContextMenu->popup(contactList->mapToGlobal(contactList->mapFromGlobal(QCursor::pos())));
}

void contactListTree::sendMessageActionTriggered()
{
	doubleClickedBuddy(currentContextBuddy);
}

void contactListTree::userInformationActionTriggered()
{
	openInfoWindow(currentContextBuddy->getUin());
}

void contactListTree::statusCheckActionTriggered()
{
	checkStatusFor(currentContextBuddy->getUin());
}

void contactListTree::messageHistoryActionTriggered()
{
	showHistory(currentContextBuddy->getUin());
}

void contactListTree::readAwayActionTriggered()
{
	emit incSnacSeq();
	icqMessage message(codepage);
	
	if ( currentContextBuddy->icqLite )
	{
		message.awayType = 0x1a;
	} else {
	
	if ( currentContextBuddy->getStatus() == contactOccupied )
		message.awayType = 0xe9;
	else if ( currentContextBuddy->getStatus() == contactNa )
		message.awayType = 0xea;
	else if ( currentContextBuddy->getStatus() == contactDnd )
		message.awayType = 0xeb;
	else if ( currentContextBuddy->getStatus() == contactFfc )
		message.awayType = 0xec;
	else
		message.awayType = 0xe8;
	}
	message.requestAutoreply(tcpSocket, currentContextBuddy->getUin(),*flapSeq, *snacSeq);
	emit incFlapSeq();
	
	
	
	
	readAwayDialog *dialog = new readAwayDialog;
	dialog->setWindowTitle(tr("%1 away message").arg(currentContextBuddy->getName()));
	dialog->setAttribute(Qt::WA_QuitOnClose, false);
	dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		connect( dialog, SIGNAL(destroyed ( QObject *)),
				this, SLOT(deleteAwayWindow(QObject *)));
	dialog->show();
	awayMessageList.insert(message.msgCookie, dialog);
}

void contactListTree::deleteAwayWindow(QObject *o)
{
	readAwayDialog *tempWindow = (readAwayDialog  *)(o);
	awayMessageList.remove(awayMessageList.key(tempWindow));
}

void contactListTree::getAwayMessage(quint16 length)
{
	icqMessage message(codepage);
	message.getAwayMessage(socket, length);
	
	if ( message.msgType == 0xe8 ||
			message.msgType == 0xe9 ||
			message.msgType == 0xea ||
			message.msgType == 0xeb ||
			message.msgType == 0xec)
	{
		if ( awayMessageList.contains(message.msgCookie))
		{
			awayMessageList.value(message.msgCookie)->addMessage(message.msg);
		}
	} else if (message.msgType == 0x1a )
	{
		
		QString xStatusmsg = addXstatusMessage(message.from, message.byteArrayMsg);
		
		if ( awayMessageList.contains(message.msgCookie))
		{
			awayMessageList.value(message.msgCookie)->addXstatus(xStatusmsg);
		}
	}
}

void contactListTree::renameContactActionTriggered()
{
	addRenameDialog dialog;
			
			dialog.setWindowTitle(tr("Rename contact"));
			
			if ( dialog.exec() )
			{
				
				QString contactName = dialog.name;
				QString uin = currentContextBuddy->getUin();
					
				emit incSnacSeq();
				QByteArray packet2;
				packet2[0] = 0x2a;
				packet2[1] = 0x02;
				packet2.append(convertToByteArray((quint16)*flapSeq));
				if ( currentContextBuddy->notAutho )
					packet2.append(convertToByteArray((quint16)(28 + contactName.toUtf8().length() + uin.length())));
				else
					packet2.append(convertToByteArray((quint16)(24 + contactName.toUtf8().length() + uin. toUtf8().length())));
					
				snac snac1308;
				snac1308.setFamily(0x0013);
				snac1308.setSubType(0x0009);
				snac1308.setReqId(*snacSeq);
				packet2.append(snac1308.getData());
					
					
				packet2.append(convertToByteArray((quint16)uin.toUtf8().length()));
				packet2.append(uin.toUtf8());
					
					packet2.append(convertToByteArray((quint16)currentContextBuddy->groupID));
					packet2.append(convertToByteArray((quint16)currentContextBuddy->itemId));
					
					packet2.append(convertToByteArray((quint16)0x0000));
					
					if ( currentContextBuddy->notAutho)
						packet2.append(convertToByteArray((quint16)(8 + contactName.toUtf8().length())));
					else
						packet2.append(convertToByteArray((quint16)(4 + contactName.toUtf8().length())));
					packet2.append(convertToByteArray((quint16)0x0131));
					packet2.append(convertToByteArray((quint16)contactName.toUtf8().length()));
					
					packet2.append(contactName.toUtf8());
					
					if ( currentContextBuddy->notAutho)
					{
						packet2.append(convertToByteArray((quint16)0x0066));
						packet2.append(convertToByteArray((quint16)0));
					}
					
					emit incFlapSeq();
					
					
					
					tcpSocket->write(packet2);
								
					
					modifyObject renameBuddy;
					
					renameBuddy.itemId = currentContextBuddy->itemId;
					renameBuddy.groupId = currentContextBuddy->groupID;
					renameBuddy.itemType = 0;
					renameBuddy.operation = 1;
					renameBuddy.buddyName = contactName;
					renameBuddy.buddyUin = uin;
					
					modifyReqList.append(renameBuddy);
			}	
}

void contactListTree::deleteContactActionTriggered()
{

	deleteContactDialog dialog;
	dialog.setWindowTitle(tr("Delete %1").arg(currentContextBuddy->getName()));
	
	if ( !movingBuddy)
	{
		if ( !dialog.exec() )
		 return;
	}
		QString contactName = currentContextBuddy->getName();
		QString uin = currentContextBuddy->getUin();
		
		if ( !movingBuddy)
		{
			if ( dialog.deleteHistory() )
			historyObject->deleteHistory(uin);
		}
		
		
	movingBuddy = false;
		if ( buddyList.contains(uin))
		{
			if (!buddyList.value(uin)->groupID)
			{
				QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
						
				QStringList contactList = contacts.value("list/contacts").toStringList();
						
				contactList.removeAll(uin);
				contacts.setValue("list/contacts", contactList);
				
				contacts.remove(uin);
					
				treeBuddyItem *buddy = buddyList.value(uin);
						
						idBuddyList.removeAll(buddy->itemId);
						
						if ( infoWindowList.contains(uin) )
						{
							infoWindowList.value(uin)->close();
							infoWindowList.remove(uin);
						}
						
						if ( historyList.contains(uin) )
						{
							historyList.value(uin)->close();
							historyList.remove(uin);
						}
						
						if ( chatWindowList.contains(uin) )
						{
							chatWindowList.value(uin)->close();
							chatWindowList.remove(uin);
						}
						quint16 groupCount;
						QString groupName;
						treeGroupItem *group = groupList.value(buddy->groupID);
							
							group->buddiesList.removeAll(buddy->itemId);
							
							group->userCount--;
							group->updateText();
							groupCount = group->buddiesList.count();
							groupName = group->name;
							if ( !group->offlineList->childCount() )
								group->offlineList->setHidden(true);
							if ( !group->onlineList->childCount() )
										group->onlineList->setHidden(true);
							if ( !group->userCount && hideEmpty)
								group->setHidden(true);
							if ( isMergeAccounts )
								emit reupdateList();
						buddyList.remove(uin);
						delete buddy;
						
						return;
			}
		}
		
		
		
		QByteArray editPack;
			
			emit incSnacSeq();
			QByteArray packet;
			packet[0] = 0x2a;
			packet[1] = 0x02;
			packet.append(convertToByteArray((quint16)*flapSeq));
			packet.append(convertToByteArray((quint16)10));
			
			snac snac1311;
			snac1311.setFamily(0x0013);
			snac1311.setSubType(0x0011);
			snac1311.setReqId(*snacSeq);
			packet.append(snac1311.getData());
			emit incFlapSeq();
			editPack.append(packet);
			
			emit incSnacSeq();
			QByteArray packet2;
			packet2[0] = 0x2a;
			packet2[1] = 0x02;
			packet2.append(convertToByteArray((quint16)*flapSeq));
			if ( currentContextBuddy->notAutho )
				packet2.append(convertToByteArray((quint16)(28 + contactName.toUtf8().length() + uin.length())));
			else
				packet2.append(convertToByteArray((quint16)(24 + contactName.toUtf8().length() + uin. toUtf8().length())));
				
			snac snac130a;
			snac130a.setFamily(0x0013);
			snac130a.setSubType(0x000a);
			snac130a.setReqId(*snacSeq);
			packet2.append(snac130a.getData());
				
				
			packet2.append(convertToByteArray((quint16)uin.toUtf8().length()));
			packet2.append(uin.toUtf8());
				
			packet2.append(convertToByteArray((quint16)currentContextBuddy->groupID));
			packet2.append(convertToByteArray((quint16)currentContextBuddy->itemId));
				
			packet2.append(convertToByteArray((quint16)0x0000));
				
			if ( currentContextBuddy->notAutho)
				packet2.append(convertToByteArray((quint16)(8 + contactName.toUtf8().length())));
			else
				packet2.append(convertToByteArray((quint16)(4 + contactName.toUtf8().length())));
			packet2.append(convertToByteArray((quint16)0x0131));
				packet2.append(convertToByteArray((quint16)contactName.toUtf8().length()));
				
				packet2.append(contactName.toUtf8());
				
				if ( currentContextBuddy->notAutho)
				{
					packet2.append(convertToByteArray((quint16)0x0066));
					packet2.append(convertToByteArray((quint16)0));
				}
				
				emit incFlapSeq();
			
			editPack.append(packet2);
			
			
			tcpSocket->write(editPack);
						
			
			modifyObject delBuddy;
			
			delBuddy.itemId = currentContextBuddy->itemId;
			delBuddy.groupId = currentContextBuddy->groupID;
			delBuddy.itemType = 0;
			delBuddy.operation = 2;
			delBuddy.buddyName = contactName;
			delBuddy.buddyUin = uin;
			
			modifyReqList.append(delBuddy);
}

void contactListTree::moveContactActionTriggered()
{

			addBuddyDialog addDialog;
			addDialog.setWindowTitle(tr("Move %1 to:").arg(currentContextBuddy->getUin()));
			QStringList groups;
			foreach (treeGroupItem *group, groupList)
			{
				if ( groupList.key(group))
					groups<<group->name;
			}
			addDialog.setMoveData(groups);
			addDialog.rellocateDialogToCenter(qutIM::getInstance());
			if ( addDialog.exec() )
			{
				QString uin = currentContextBuddy->getUin();
				QString name = currentContextBuddy->getName();
				bool authReq = currentContextBuddy->notAutho;
				
				movingBuddy = true;
				deleteContactActionTriggered();
				sendUserAddReq(uin, name,authReq , addDialog.getGroup());
				
			}

}

void contactListTree::renameContact(const QString &uin, const QString &name)
{
	if ( buddyList.contains(uin))
	{
		QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
		
		contacts.setValue(uin + "/nickname", name);
		buddyList.value(uin)->setName(name);
		buddyList.value(uin)->updateBuddyText();
	}
	
	
}

void contactListTree::removeContact(const QString &uin)
{
	if ( buddyList.contains(uin) )
	{
		QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
		
		QStringList contactList = contacts.value("list/contacts").toStringList();
		
		contactList.removeAll(uin);
		contacts.setValue("list/contacts", contactList);
		
		contacts.remove(uin);
		
		treeBuddyItem *buddy = buddyList.value(uin);
		
		idBuddyList.removeAll(buddy->itemId);
		
		if ( infoWindowList.contains(uin) )
		{
			infoWindowList.value(uin)->close();
			infoWindowList.remove(uin);
		}
		
		if ( historyList.contains(uin) )
		{
			historyList.value(uin)->close();
			historyList.remove(uin);
		}
		
		if ( chatWindowList.contains(uin) )
		{
			chatWindowList.value(uin)->close();
			chatWindowList.remove(uin);
		}
		quint16 groupCount;
		QString groupName;
			treeGroupItem *group = groupList.value(buddy->groupID);
			
			group->buddiesList.removeAll(buddy->itemId);
			
			group->userCount--;
			group->updateText();
			groupCount = group->buddiesList.count();
			groupName = group->name;
			
			if ( !buddy->isOffline)
			{
				group->buddyOffline();
				group->onlineList->removeChild(buddy);
			} else
				group->offlineList->removeChild(buddy);
			
			if ( !group->offlineList->childCount() )
				group->offlineList->setHidden(true);
			if ( !group->onlineList->childCount() )
						group->onlineList->setHidden(true);
			if ( !group->userCount && hideEmpty)
				group->setHidden(true);
			

			
		if ( isMergeAccounts )
			emit reupdateList();
		buddyList.remove(uin);
		messageList.remove(uin);
		delete buddy;
		
		
		
		QByteArray editPack;
					

					
		emit incSnacSeq();
		QByteArray packet2;
		packet2[0] = 0x2a;
		packet2[1] = 0x02;
		packet2.append(convertToByteArray((quint16)*flapSeq));
		packet2.append(convertToByteArray((quint16)(24 + groupCount * 2 + groupName.toUtf8().length())));
						
		snac snac1308;
		snac1308.setFamily(0x0013);
		snac1308.setSubType(0x0009);
		snac1308.setReqId(*snacSeq);
		packet2.append(snac1308.getData());
					
					
		packet2.append(convertToByteArray((quint16)groupName.toUtf8().length()));
					
		packet2.append(groupName.toUtf8());
		packet2.append(convertToByteArray((quint16)groupList.key(group)));
					
		packet2.append(convertToByteArray((quint16)0x0000));
		
		packet2.append(convertToByteArray((quint16)0x0001));
					
		packet2.append(convertToByteArray((quint16)(4 + groupCount * 2)));
		packet2.append(convertToByteArray((quint16)0x00c8));
		packet2.append(convertToByteArray((quint16)(groupCount* 2)));
					
		foreach(quint16 buddyId, group->buddiesList)
		{
			if ( buddyId)
					packet2.append(convertToByteArray((quint16)buddyId));
		}
					
					
		emit incFlapSeq();
					
		editPack.append(packet2);
					
					
		emit incSnacSeq();
		QByteArray packet;
		packet[0] = 0x2a;
		packet[1] = 0x02;
		packet.append(convertToByteArray((quint16)*flapSeq));
		packet.append(convertToByteArray((quint16)10));
					
		snac snac1312;
		snac1312.setFamily(0x0013);
		snac1312.setSubType(0x0012);
		snac1312.setReqId(*snacSeq);
		packet.append(snac1312.getData());
		emit incFlapSeq();
		editPack.append(packet);
					
					
		tcpSocket->write(editPack);
								
					
		modifyObject addGroup;
					
					addGroup.itemId = 0;
					addGroup.groupId = groupList.key(group);
					addGroup.itemType = 1;
					addGroup.operation = 1;
					addGroup.buddyName = group->name;
					
					modifyReqList.append(addGroup);
		
		
		
	}
}

void contactListTree::addToVisibleActionTriggered()
{
	if ( visibleList.contains(currentContextBuddy->getUin()))
		return;
	
	if ( invisibleList.contains(currentContextBuddy->getUin()))
		deleteFromPrivacyList(currentContextBuddy->getUin(),1);
	
		
		QString uin = currentContextBuddy->getUin();
		QString name = currentContextBuddy->getName();
		emit incSnacSeq();
		QByteArray packet2;
		packet2[0] = 0x2a;
		packet2[1] = 0x02;
		packet2.append(convertToByteArray((quint16)*flapSeq));
		packet2.append(convertToByteArray((quint16)(24 + name.toUtf8().length() + uin.toUtf8().length())));
			
		snac snac1308;
		snac1308.setFamily(0x0013);
		snac1308.setSubType(0x0008);
		snac1308.setReqId(*snacSeq);
		packet2.append(snac1308.getData());
			
			
		packet2.append(convertToByteArray((quint16)uin.toUtf8().length()));
		packet2.append(uin.toUtf8());
			
		packet2.append(convertToByteArray((quint16)0));
		packet2.append(convertToByteArray((quint16)currentContextBuddy->itemId));
			
		packet2.append(convertToByteArray((quint16)0x0002));
			
		packet2.append(convertToByteArray((quint16)(4 + name.toUtf8().length())));
		packet2.append(convertToByteArray((quint16)0x0131));
		packet2.append(convertToByteArray((quint16)name.toUtf8().length()));
			
		packet2.append(name.toUtf8());
			
			
			emit incFlapSeq();
		
		
		
		tcpSocket->write(packet2);
					
		
		modifyObject visBuddy;
		
		visBuddy.itemId = currentContextBuddy->itemId;
		visBuddy.groupId = 0;
		visBuddy.itemType = 2;
		visBuddy.operation = 0;
		visBuddy.buddyName = name;
		visBuddy.buddyUin = uin;
		
		modifyReqList.append(visBuddy);
	
		visibleList.append(uin);
		QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
		contacts.setValue("list/visible", visibleList);
		if ( privacyListWindowOpen)
			privacyWindow->createLists();
	
}

void contactListTree::addToInvisibleActionTriggered()
{
	if ( invisibleList.contains(currentContextBuddy->getUin()))
		return;
	
	
	if ( visibleList.contains(currentContextBuddy->getUin()))
		deleteFromPrivacyList(currentContextBuddy->getUin(),0);
	
	
	QString uin = currentContextBuddy->getUin();
	QString name = currentContextBuddy->getName();
	emit incSnacSeq();
	QByteArray packet2;
	packet2[0] = 0x2a;
	packet2[1] = 0x02;
	packet2.append(convertToByteArray((quint16)*flapSeq));
	packet2.append(convertToByteArray((quint16)(24 + name.toUtf8().length() + uin.toUtf8().length())));
		
	snac snac1308;
	snac1308.setFamily(0x0013);
	snac1308.setSubType(0x0008);
	snac1308.setReqId(*snacSeq);
	packet2.append(snac1308.getData());
		
		
	packet2.append(convertToByteArray((quint16)uin.toUtf8().length()));
	packet2.append(uin.toUtf8());
		
	packet2.append(convertToByteArray((quint16)0));
	packet2.append(convertToByteArray((quint16)currentContextBuddy->itemId));
		
	packet2.append(convertToByteArray((quint16)0x0003));
		
	packet2.append(convertToByteArray((quint16)(4 + name.toUtf8().length())));
	packet2.append(convertToByteArray((quint16)0x0131));
	packet2.append(convertToByteArray((quint16)name.toUtf8().length()));
		
	packet2.append(name.toUtf8());
		
		
		emit incFlapSeq();
	
	
	
	tcpSocket->write(packet2);
				
	
	modifyObject visBuddy;
	
	visBuddy.itemId = currentContextBuddy->itemId;
	visBuddy.groupId = 0;
	visBuddy.itemType = 3;
	visBuddy.operation = 0;
	visBuddy.buddyName = name;
	visBuddy.buddyUin = uin;
	
	modifyReqList.append(visBuddy);

	invisibleList.append(uin);
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	contacts.setValue("list/invisible", invisibleList);
	if ( privacyListWindowOpen)
		privacyWindow->createLists();
}

void contactListTree::addToIgnoreActionTriggered()
{
	if ( ignoreList.contains(currentContextBuddy->getUin()))
		return;
	
	QString uin = currentContextBuddy->getUin();
	QString name = currentContextBuddy->getName();
	emit incSnacSeq();
	QByteArray packet2;
	packet2[0] = 0x2a;
	packet2[1] = 0x02;
	packet2.append(convertToByteArray((quint16)*flapSeq));
	packet2.append(convertToByteArray((quint16)(24 + name.toUtf8().length() + uin.toUtf8().length())));
		
	snac snac1308;
	snac1308.setFamily(0x0013);
	snac1308.setSubType(0x0008);
	snac1308.setReqId(*snacSeq);
	packet2.append(snac1308.getData());
		
		
	packet2.append(convertToByteArray((quint16)uin.toUtf8().length()));
	packet2.append(uin.toUtf8());
		
	packet2.append(convertToByteArray((quint16)0));
	packet2.append(convertToByteArray((quint16)currentContextBuddy->itemId));
		
	packet2.append(convertToByteArray((quint16)0x000e));
		
	packet2.append(convertToByteArray((quint16)(4 + name.toUtf8().length())));
	packet2.append(convertToByteArray((quint16)0x0131));
	packet2.append(convertToByteArray((quint16)name.toUtf8().length()));
		
	packet2.append(name.toUtf8());
		
		
		emit incFlapSeq();
	
	
	
	tcpSocket->write(packet2);
				
	
	modifyObject visBuddy;
	
	visBuddy.itemId = currentContextBuddy->itemId;
	visBuddy.groupId = 0;
	visBuddy.itemType = 0x000e;
	visBuddy.operation = 0;
	visBuddy.buddyName = name;
	visBuddy.buddyUin = uin;
	
	modifyReqList.append(visBuddy);

	ignoreList.append(uin);
	QSettings contacts(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	contacts.setValue("list/ignore", ignoreList);
	if ( privacyListWindowOpen)
		privacyWindow->createLists();
}

void contactListTree::requestAuthorizationActionTriggered()
{
	requestAuthDialog dialog;
	
	if ( dialog.exec() )
	{
		QString uin = currentContextBuddy->getUin();
		QString msg = dialog.getMessage();
		
		QByteArray authPack;
		
		emit incSnacSeq();
		QByteArray packet2;
		packet2[0] = 0x2a;
		packet2[1] = 0x02;
		packet2.append(convertToByteArray((quint16)*flapSeq));
		packet2.append(convertToByteArray((quint16)(15 + uin.toUtf8().length())));
			
		snac snac1314;
		snac1314.setFamily(0x0013);
		snac1314.setSubType(0x0014);
		snac1314.setReqId(*snacSeq);
		packet2.append(snac1314.getData());
			
		packet2[packet2.length()] = (quint8)uin.toUtf8().length();
		packet2.append(uin.toUtf8());

			
		packet2.append(convertToByteArray((quint16)0));
		packet2.append(convertToByteArray((quint16)0));
		
		authPack.append(packet2);
		
		emit incFlapSeq();
		emit incSnacSeq();
		
		
		QByteArray packet;
		packet[0] = 0x2a;
		packet[1] = 0x02;
		packet.append(convertToByteArray((quint16)*flapSeq));
		packet.append(convertToByteArray((quint16)(15 + uin.toUtf8().length() + msg.toUtf8().length())));
			
		snac snac1318;
		snac1318.setFamily(0x0013);
		snac1318.setSubType(0x0018);
		snac1318.setReqId(*snacSeq);
		packet.append(snac1318.getData());
			
		packet[packet.length()] = (quint8)uin.toUtf8().length();
		packet.append(uin.toUtf8());

			
		packet.append(convertToByteArray((quint16)msg.toUtf8().length()));
		packet.append(msg.toUtf8());
		packet.append(convertToByteArray((quint16)0));
		
		authPack.append(packet);
		emit incFlapSeq();
		tcpSocket->write(authPack);
		
		
	}
}

void contactListTree::getAuthorizationRequest(quint16 length)
{
	socket->read(8);
	
	length -= 8;
	
	quint8 uinLength = convertToInt8(socket->read(1));
	length--;
	
	QString uin = socket->read(uinLength);
	length -= uinLength;
	quint16 msgLength = byteArrayToInt16(socket->read(2));
	length -= 2;
	
	QString msg = QString::fromUtf8(socket->read(msgLength));
	length -= msgLength;
	
	socket->read(2);
	length -= 2;
	
	
	
	
	
	if ( buddyList.contains(uin) )
	{
		treeBuddyItem *buddy = buddyList.value(uin);
		
		Q_ASSERT(buddy);
		if (buddy)
		{
			buddy->waitingForAuth(true);
			buddy->authMessage = msg;
			emit userMessage(buddy->getUin(), buddy->getName(), msg, messageNotification, true);
		}
		
	} 
	else {
		
		if ( !blockAuth)
		{
			
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
			
			treeGroupItem *group = groupList.value(0);
			
			Q_ASSERT(group);
			if (!group)
				return ;

			treeBuddyItem *buddy;
			if ( showGroups )
			{
				Q_ASSERT(group->offlineList);
				if (group->offlineList)
					buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, group->offlineList);
			}
			else
			{
				Q_ASSERT(offlineList);
				if (offlineList)
					buddy = new treeBuddyItem(icqUin, statusIconObject->getOfflineIcon(), this, offlineList);
			}

			Q_ASSERT(buddy);
			if (!buddy)
				return ;

			initializeBuddy(buddy);
			buddy->underline = !dontUnderlineNotAutho;
			buddy->groupID = 0;
			buddy->groupName = group->name;
			group->userCount++;
			group->updateText();
			buddyList.insert(uin, buddy);
			buddy->setBuddyUin(uin);
			buddy->setName(uin);
			buddy->updateBuddyText();
			buddy->setCustomFont(offFont.fontFamily, offFont.fontSize, offFont.fontColor);
			if ( showGroups )
				updateNil();
			requestUinInformation(buddy->getUin());
			settings.beginGroup(buddy->getUin());
			settings.setValue("name", buddy->getUin());
			settings.setValue("groupid", 0);
			settings.setValue("nickname", buddy->getName());
			settings.endGroup();
			
			QStringList contacts = settings.value("list/contacts").toStringList();
			contacts<<buddy->getUin();
			settings.setValue("list/contacts", contacts);
			
			if ( contactListChanged  && isMergeAccounts )
						emit reupdateList();
			
			buddy->waitingForAuth(true);
			buddy->authMessage = msg;
			emit userMessage(buddy->getUin(), buddy->getName(), msg, messageNotification, true);
		}
	}
	
	
	
	
	if ( length)
		socket->read(length);
}

void contactListTree::openAuthReqFromBuddy(treeBuddyItem *buddy)
{
	
	acceptAuthDialog *dialog = new acceptAuthDialog(buddy->getUin());
	dialog->setWindowTitle(tr("Accept authorization from %1").arg(buddy->getName()));
	dialog->setMessage(buddy->authMessage);
	buddy->waitingForAuth(false);
	
	connect(dialog, SIGNAL(sendAuthReqAnswer(bool, const QString &)), this, SLOT(sendAuthReqAnswer(bool, const QString &)));
	dialog->show();
}

void contactListTree::authorizationAcceptedAnswer(quint16 length)
{
	socket->read(8);
	length -= 8;
	
	quint8 uinLength = convertToInt8(socket->read(1));
	length--;
	
	QString uin = socket->read(uinLength);
	length -= uinLength;
	
	quint8 mesgAccept = convertToInt8(socket->read(1));
	length --;
	
	QString message;
	if ( mesgAccept )
	{
		message = tr("Authorization accepted");
		if ( buddyList.contains(uin) )
		{
			buddyList.value(uin)->notAutho = false;
			buddyList.value(uin)->updateBuddyText();
		}
	}
	else
		message = tr("Authorization declined");
	
	if ( buddyList.contains(uin))
		emit userMessage(uin, buddyList.value(uin)->getName(), message, messageNotification, true);
	else
		emit userMessage(uin, uin, message, messageNotification, false);
	
	if ( length )
		socket->read(length);
}

void contactListTree::addToContactListActionTriggered()
{
	addUserToList(currentContextBuddy->getUin(), currentContextBuddy->getName(), currentContextBuddy->notAutho);
}

void contactListTree::allowToAddMeTriggered()
{
	QString uin = currentContextBuddy->getUin();
	emit incSnacSeq();
	QByteArray packet2;
	packet2[0] = 0x2a;
	packet2[1] = 0x02;
	packet2.append(convertToByteArray((quint16)*flapSeq));
	packet2.append(convertToByteArray((quint16)(15 + uin.toUtf8().length())));
		
	snac snac1314;
	snac1314.setFamily(0x0013);
	snac1314.setSubType(0x0014);
	snac1314.setReqId(*snacSeq);
	packet2.append(snac1314.getData());
		
	packet2[packet2.length()] = (quint8)uin.toUtf8().length();
	packet2.append(uin.toUtf8());

		
	packet2.append(convertToByteArray((quint16)0));
	packet2.append(convertToByteArray((quint16)0));
	
	
	emit incFlapSeq();
	
	tcpSocket->write(packet2);
}

void contactListTree::removeMyselfTriggered()
{
	QString uin = currentContextBuddy->getUin();
	emit incSnacSeq();
	QByteArray packet2;
	packet2[0] = 0x2a;
	packet2[1] = 0x02;
	packet2.append(convertToByteArray((quint16)*flapSeq));
	packet2.append(convertToByteArray((quint16)(11 + uin.toUtf8().length())));
		
	snac snac1316;
	snac1316.setFamily(0x0013);
	snac1316.setSubType(0x0016);
	snac1316.setReqId(*snacSeq);
	packet2.append(snac1316.getData());
		
	packet2[packet2.length()] = (quint8)uin.toUtf8().length();
	packet2.append(uin.toUtf8());
	
	
	emit incFlapSeq();
	
	tcpSocket->write(packet2);
}

void contactListTree::sendFile(QByteArray &part1, QByteArray &part2, QByteArray &part3)
{
	QByteArray packet;
	emit incSnacSeq();
	packet[0] = 0x2a;
	packet[1] = 0x02;
	packet.append(convertToByteArray((quint16)*flapSeq));
	QByteArray tmpPacket;
		
	snac snac0406;
	snac0406.setFamily(0x0004);
	snac0406.setSubType(0x0006);
	snac0406.setReqId(*snacSeq);
	tmpPacket.append(snac0406.getData());
	
	
	tmpPacket.append(part1);
	
	tmpPacket.append(convertToByteArray((quint16)0x0005));
	
	tmpPacket.append(convertToByteArray((quint16)(24 + part2.length() + part3.length())));
	
	quint32 localIP = tcpSocket->localAddress().toIPv4Address();
	
	tmpPacket.append(part2);
	tlv tlv02;
	tlv02.setType(0x0002);
	tlv02.setData((quint32)localIP);
	
	tlv tlv16;
	tlv16.setType(0x0016);
	tlv16.setData((quint32)(localIP ^ 0xffffffff));
		
	tlv tlv03;
	tlv03.setType(0x0003);
	tlv03.setData((quint32)localIP);
	
	tmpPacket.append(tlv02.getData());
	tmpPacket.append(tlv16.getData());
	tmpPacket.append(tlv03.getData());
	
	tmpPacket.append(part3);
	
	packet.append(convertToByteArray((quint16)tmpPacket.length()));
	packet.append(tmpPacket);
	
	emit incFlapSeq();
	
	tcpSocket->write(packet);
	
}

void contactListTree::sendFileActionTriggered()
{
	fileTransferObject->sendFileTriggered(currentContextBuddy->getUin());
}

void contactListTree::sendCancelSending(QByteArray &part)
{
	QByteArray packet;
	emit incSnacSeq();
	packet[0] = 0x2a;
	packet[1] = 0x02;
	packet.append(convertToByteArray((quint16)*flapSeq));
	packet.append(convertToByteArray((quint16)(10 + part.length())));	
	snac snac0406;
	snac0406.setFamily(0x0004);
	snac0406.setSubType(0x0006);
	snac0406.setReqId(*snacSeq);
	packet.append(snac0406.getData());
	packet.append(part);
	
	emit incFlapSeq();
	tcpSocket->write(packet);
}

void contactListTree::redirectToProxy(const QByteArray &part)
{
	QByteArray packet;
	emit incSnacSeq();
	packet[0] = 0x2a;
	packet[1] = 0x02;
	packet.append(convertToByteArray((quint16)*flapSeq));
	packet.append(convertToByteArray((quint16)(10 + part.length())));	
	snac snac0406;
	snac0406.setFamily(0x0004);
	snac0406.setSubType(0x0006);
	snac0406.setReqId(*snacSeq);
	packet.append(snac0406.getData());
	packet.append(part);
	
	emit incFlapSeq();
	tcpSocket->write(packet);
}

void contactListTree::sendAcceptMessage(const QByteArray &part)
{
	QByteArray packet;
	emit incSnacSeq();
	packet[0] = 0x2a;
	packet[1] = 0x02;
	packet.append(convertToByteArray((quint16)*flapSeq));
	packet.append(convertToByteArray((quint16)(10 + part.length())));	
	snac snac0406;
	snac0406.setFamily(0x0004);
	snac0406.setSubType(0x0006);
	snac0406.setReqId(*snacSeq);
	packet.append(snac0406.getData());
	packet.append(part);
	
	emit incFlapSeq();
	tcpSocket->write(packet);
}

void contactListTree::emoticonSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	
	settings.beginGroup("emoticons");
	
	emoticonXMLPath = settings.value("path", "").toString();
	
	
	
	if (emoticonXMLPath.isEmpty())
	{
		QStringList paths;
		paths<<"/usr/share/qutim"<<settings.fileName().section('/', 0, -3)<<".";
		foreach(QString path, paths)
		{
			
			QDir emoticonPath = path  + "/emoticons/";

			QStringList themes = emoticonPath.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
			
			foreach(QString dirName, themes)
			{

				QDir dir(path + "/" + dirName);
				QStringList filter;
				filter<<"*.xml";
				
				QStringList fileXML = dir.entryList(filter,QDir::Files);

				if ( fileXML.count())
				{
					emoticonXMLPath =  dir.path() + "/" + fileXML.at(0);
				}
			}
		}
		settings.setValue("path", emoticonXMLPath);
		
	}
	settings.endGroup();
	foreach(chatWindow *w, chatWindowList)
		w->setEmoticonPath(emoticonXMLPath);
}

void contactListTree::sendImage(const QString &uin, const QString &path)
{
	
	if ( buddyList.contains(uin) && QFile::exists(path))
	{
		emit incSnacSeq();
		
		icqMessage message(codepage);

		message.sendImage(tcpSocket, uin, path,*flapSeq, *snacSeq);
		emit incFlapSeq();
	
//	if ( saveHistory )
//		historyObject->saveHistoryMessage(msg.fromUin, 
//				accountNickname, QDateTime::currentDateTime() , false, msg.message);
	}
}

void contactListTree::readXstatusTriggered()
{
	emit incSnacSeq();
	icqMessage message(codepage);
	message.requestXStatus(tcpSocket, currentContextBuddy->getUin(),*flapSeq, *snacSeq);
	emit incFlapSeq();
	
	
	
	
	readAwayDialog *dialog = new readAwayDialog;
	dialog->setWindowTitle(tr("%1 xStatus message").arg(currentContextBuddy->getName()));
	dialog->setAttribute(Qt::WA_QuitOnClose, false);
	dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		connect( dialog, SIGNAL(destroyed ( QObject *)),
				this, SLOT(deleteAwayWindow(QObject *)));
	dialog->show();
	awayMessageList.insert(message.msgCookie, dialog);
}

void contactListTree::sendAuthReqAnswer(bool accep, const QString &uin)
{
	emit incSnacSeq();

	
	QByteArray packet;
	packet[0] = 0x2a;
	packet[1] = 0x02;
	packet.append(convertToByteArray((quint16)*flapSeq));
	packet.append(convertToByteArray((quint16)(16 + uin.toUtf8().length())));
		
	snac snac131a;
	snac131a.setFamily(0x0013);
	snac131a.setSubType(0x001a);
	snac131a.setReqId(*snacSeq);
	packet.append(snac131a.getData());
		
	packet[packet.length()] = (quint8)uin.toUtf8().length();
	packet.append(uin.toUtf8());

		
	if ( accep)
		packet[packet.length()] = (quint8)1;
	else
		packet[packet.length()] = (quint8)0;

	packet.append(convertToByteArray((quint16)0));
	packet.append(convertToByteArray((quint16)0));

	emit incFlapSeq();
	
	tcpSocket->write(packet);
}

void contactListTree::hideRoot(bool flag)
{
	rootItem->setHidden(flag);
}



QString contactListTree::addXstatusMessage(const QString &xtrauin,QByteArray &msg)
{
	if (msg.contains(QByteArray::fromHex("4177617920537461747573204d657373616765")))
	{

		
		msg = msg.right(msg.length() - 19);
		QString xtraaw = xTraAway(QString::fromUtf8(msg));
		
		if ( buddyList.contains(xtrauin))
		{
			buddyList.value(xtrauin)->xStatusCaption = xtraaw;
			buddyList.value(xtrauin)->createToolTip();
		}
		
		return xtraaw;
		
	} 
	else
	{
		msg = msg.right(msg.length() - 98);
		msg.chop(13);
		QString title = findTitle(QString::fromUtf8(msg));
		QString message = findMessage(QString::fromUtf8(msg));
		QString xtraaw = "<b>" + title +"</b><br>" + message.replace("\n", "<br>");
		if ( buddyList.contains(xtrauin))
		{
			buddyList.value(xtrauin)->xStatusCaption = title;
			buddyList.value(xtrauin)->xStatusMsg = message.replace("\n", "<br>");
			buddyList.value(xtrauin)->createToolTip();
		}
		return xtraaw;
	}
}

QString contactListTree::findTitle(QString _givenText) 
{ 
	QRegExp regExp("[&][l][t][;][t][i][t][l][e][&][g][t][;](.+)[&][l][t][;][/][t][i][t][l][e][&][g][t][;]"); 
	int pos = 0; 
	regExp.indexIn(_givenText, pos); 
	QString rez = regExp.cap(0); 
	return rez.mid(13, rez.length() - 27);
}

QString contactListTree::findMessage(QString _givenText) 
{ 
	QRegExp regExp("[&][l][t][;][d][e][s][c][&][g][t][;](.+)[&][l][t][;][/][d][e][s][c][&][g][t][;]"); 
	int pos = 0; 
	regExp.indexIn(_givenText, pos); 
	QString rez = regExp.cap(0); 
	return rez.mid(12, rez.length() - 25);
}

QString contactListTree::xTraAway(QString _givenText)
{

	QRegExp regExp("[<][B][O][D][Y][>](.+)[<][/][B][O][D][Y][>]"); 
	int pos = 0; 
	regExp.indexIn(_givenText, pos); 
	QString rez = regExp.cap(0);

	return rez.mid(6, rez.length() - 13);
}

void contactListTree::offlineHideButtonClicked(bool flag)
{

	if ( showOffline != flag)
	{

		showOffline = flag;

		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
		settings.setValue("contactlist/hideoff", showOffline);
		showOfflineUsers();
	}
}

void contactListTree::askForXstatusTimerTick()
{
	if ( xStatusTickList.count() )
	{
		
		treeBuddyItem *buddy = xStatusTickList.at(0);
		
		if (buddy->xStatusPresent && showXstatusesinToolTips )
		{
			if ( !buddy->icqLite)
			{
			emit incSnacSeq();
			icqMessage message(codepage);
			message.requestXStatus(tcpSocket, buddy->getUin(),*flapSeq, *snacSeq);
			emit incFlapSeq();
			} else {
				emit incSnacSeq();
				icqMessage message(codepage);
				message.awayType = 0x1a;
				message.requestAutoreply(tcpSocket, buddy->getUin(),*flapSeq, *snacSeq);
				emit incFlapSeq();
			}
		}

		xStatusTickList.removeAt(0);
		QTimer::singleShot(500,this, SLOT(askForXstatusTimerTick()));
	} else
		letMeAskYouAboutXstatusPlease = true;
}

void contactListTree::onUpdateTranslation()
{
	createGroup->setText(tr("New group"));
	renameGroup->setText(tr("Rename group"));
	deleteGroup->setText(tr("Delete group"));
	sendMessageAction->setText(tr("Send message"));
	userInformationAction->setText(tr("Contact details"));
	copyUINAction->setText(tr("Copy UIN to clipboard"));
	statusCheckAction->setText(tr("Contact status check"));
	messageHistoryAction->setText(tr("Message history"));
	readAwayAction->setText(tr("Read away message"));
	renameContactAction->setText(tr("Rename contact"));
	deleteContactAction->setText(tr("Delete contact"));
	moveContactAction->setText(tr("Move to group"));
	addToVisibleAction->setText(tr("Add to visible list"));
	addToInvisibleAction->setText(tr("Add to invisible list"));
	addToIgnoreAction->setText(tr("Add to ignore list"));
	requestAuthorizationAction->setText(tr("Authorization request"));
	addToContactListAction->setText(tr("Add to contact list"));
	allowToAddMe->setText(tr("Allow contact to add me"));
	removeMyself->setText(tr("Remove myself from contact's list"));
	readXstatus->setText(tr("Read custom status"));
}

void contactListTree::onReloadGeneralSettings()
{
	QHashIterator<QString, treeBuddyItem *> iterator(buddyList);

	while(iterator.hasNext())
	{
		treeBuddyItem *buddy = NULL;
		iterator.next();

		buddy = iterator.value();
		buddy->setIcon(0, (statusIconClass::getInstance()->*(buddy->statusIconMethod))());
	}
}

void contactListTree::sendFileFromWindow(const QString &uin)
{
	if ( buddyList.contains(uin))
	{
		if ( buddyList.value(uin)->fileTransferSupport)
		{
			if ( !buddyList.value(uin)->isOffline)	
				fileTransferObject->sendFileTriggered(uin);
		}
		else
			emit sendSystemMessage(tr("Contact does not support file transfer"));

	}
}

void contactListTree::deleteFromIgnoreActionTriggered()
{
	deleteFromPrivacyList(currentContextBuddy->getUin(), 2);
	if ( privacyListWindowOpen)
		privacyWindow->createLists();
}

void contactListTree::deleteFromInvisibleActionTriggered()
{
	deleteFromPrivacyList(currentContextBuddy->getUin(), 1);
	if ( privacyListWindowOpen)
		privacyWindow->createLists();
}

void contactListTree::deleteFromVisibleActionTriggered()
{
	deleteFromPrivacyList(currentContextBuddy->getUin(), 0);
	if ( privacyListWindowOpen)
		privacyWindow->createLists();
}

void contactListTree::copyUINActionTriggered()
{
	QApplication::clipboard()->setText(currentContextBuddy->getUin());
}

void contactListTree::editNoteActionTriggered()
{
	noteWidget *noteW = new noteWidget(icqUin, currentContextBuddy->getUin(), currentContextBuddy->getName());
	connect( noteW, SIGNAL(destroyed ( QObject *)),
			this, SLOT(deleteNoteWindow(QObject *)));
	noteWidgetsList.insert(currentContextBuddy->getUin(), noteW);
	noteW->show();
}

void contactListTree::onStatusChanged(accountStatus status)
{
        if (currentStatus == status) return;

        // playing sounds
        // going online
        if (((currentStatus == offline) || (currentStatus == connecting))
                && ((status != offline) && (status != connecting)))
                emit playSoundEvent(SoundEvent::Connected, status);
        // going offline
        else if (status == offline)
                emit playSoundEvent(SoundEvent::Disconnected, offline);
	
        // saving status
        currentStatus = status;
}

void contactListTree::deleteNoteWindow(QObject *obj)
{
	noteWidget *tempWindow = (noteWidget  *)(obj);
	noteWidgetsList.remove(noteWidgetsList.key(tempWindow));
}

void contactListTree::getChangeFontSignal(const QFont &f, const QColor &fc, const QColor &c)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.setValue("chatwindow/fontcolor", fc);
	settings.setValue("chatwindow/backcolor", c);
	settings.setValue("chatwindow/font", f);
	
	foreach(chatWindow *w, chatWindowList)
	{
		w->setWindowFont(f,fc,c);
	}
}

void contactListTree::getMessageAck(quint16 length)
{
	QByteArray msgCookie = socket->read(8);
	socket->read(2);
	length -= 10;
	
	quint8 uinLength = convertToInt8(socket->read(1));
	length--;
	
	QString uin = socket->read(uinLength);
	
	length -= uinLength;
	
	if ( length )
		socket->read(length);
	
	if ( chatWindowList.contains(uin) && messageCursorPositions.contains(msgCookie))
	{
		chatWindowList.value(uin)->messageApply(messageCursorPositions.value(msgCookie));
	}
	
	messageCursorPositions.remove(msgCookie);
}

QByteArray contactListTree::convertPassToCodePage(const QString &pass)
{
	if (!codec)
		return pass.toLocal8Bit();
	
	return codec->fromUnicode(pass);
}
