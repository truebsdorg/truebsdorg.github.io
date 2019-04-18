/*
    icqAccount

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

#include <QtGui>

#include "../../../src/accountbutton.h"
#include "settings/icqsettings.h"
#include "settings/networksettings.h"
#include "settings/contactlistsettings.h"
#include "settings/messagingsettings.h"
#include "settings/statussettings.h"
#include "settings/historysettings.h"
#include "settings/eventssettings.h"
#include "settings/antispamsettings.h"
#include "settings/emoticonsettings.h"
#include "settings/soundsettings.h"
#include "oscarprotocol.h"
#include "statusdialog.h"
#include "traymessage.h"
#include "customstatusdialog.h"
#include "soundevents.h"
#include "contactlist.h"
#include "statusiconfactory.h"
#include "icqaccount.h"

icqAccount::icqAccount(QObject *parent) : 
	QObject(parent),
	icqGeneral(NULL),
	icqNetwork(NULL),
	icqCl(NULL),
	msgSett(NULL),
	statSett(NULL),
	histSett(NULL),
	eventSett(NULL),
	antiSpamSett(NULL),
	emoticSett(NULL),
	soundSett(NULL),
	generalSettings(NULL),
	icqnetworkSettings(NULL),
	clSettings(NULL),
	msgSettings(NULL),
	statSettings(NULL),
	histSettings(NULL),
	evSettings(NULL),
	antiSpSettings(NULL),
	emoticSettings(NULL),
	sndSettings(NULL)
{
	
}

icqAccount::icqAccount(QSystemTrayIcon *tray, QTreeWidget *contactTree, QString string, QObject *parent)
	: QObject(parent), icqUin(string), contactListTree(contactTree), trayIcon(tray)
{
	currentTrayStatus = false;
	statusTrayMenuExist = false;
	deleteingAccount = false;
	menuExist = false;
	firsTrayMessageIsShown = false;
	positionInStack = 1;
	currentXstatus = 0;
	statusIconIndex = 0;
	iAmConnected = false;
	connect(this, SIGNAL(addToEventList(bool)),
								parent, SLOT(addToEventList(bool)));
	
	if (trayIcon)
		connect(this, SIGNAL(updateTrayToolTip()),
									parent, SLOT(updateTrayToolTip()));
	
//      createSoundEvents();
	
	thisIcqProtocol = new oscarProtocol(icqUin,contactListTree,this);
	connect(thisIcqProtocol, SIGNAL(statusChanged(accountStatus)),
			this, SLOT(setStatusIcon(accountStatus)));
	connect(thisIcqProtocol, SIGNAL(statusChanged(accountStatus)),
			this, SLOT(onOscarStatusChanged(accountStatus)));

	connect(thisIcqProtocol, SIGNAL(accountConnected(bool)),
			this, SLOT(accountConnected(bool)));
	connect(thisIcqProtocol, SIGNAL(systemMessage(const QString &)),
			this, SLOT(systemMessage(const QString &)));
	connect(thisIcqProtocol, SIGNAL(userMessage(const QString &, const QString &, const QString &, userMessageType, bool)),
				this, SLOT(userMessage(const QString &, const QString &, const QString &, userMessageType, bool)));
	connect(thisIcqProtocol, SIGNAL(getNewMessage()),
                this, SIGNAL(getNewMessage()));
	connect(thisIcqProtocol, SIGNAL(readAllMessages()),
						this, SIGNAL(readAllMessages()));
	connect(thisIcqProtocol, SIGNAL(readAllMessages()),
							parent, SLOT(accountReadAllMessages()));
	connect(thisIcqProtocol->getContactListClass(), SIGNAL(sendGroupList(QHash<quint16, treeGroupItem *>)),
			parent, SLOT(getGroupList(QHash<quint16, treeGroupItem *>)));
	connect(thisIcqProtocol->getContactListClass(), SIGNAL(buddyChangeStatus(treeBuddyItem *, bool)),
				parent, SLOT(buddyChangeStatus(treeBuddyItem *, bool)));
	connect(thisIcqProtocol->getContactListClass(), SIGNAL(updateOnlineList()),
					parent, SLOT(updateSorting()));
	connect(thisIcqProtocol->getContactListClass(), SIGNAL(reupdateList()),
						parent, SLOT(reupdateList()));
	connect(this, SIGNAL(updateTranslation()),
		thisIcqProtocol, SIGNAL(updateTranslation()));
//	connect(this, SIGNAL(soundSettingsChanged()),
//		getProtocol()->getContactListClass(),
//		SIGNAL(soundSettingsUpdated()));

	
	createIcons();
	createStatusMenu();
	
	chooseStatus = new QAction(currentIcon, icqUin, this);
	chooseStatus->setCheckable(true);
	connect ( chooseStatus, SIGNAL(triggered()),
			this, SLOT(emitChangeStatus()));
	
	loadAccountSettings();
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	configPath = settings.fileName().section('/', 0, -2);

//	plugins = new pluginSystem(icqUin, this);
//	thisIcqProtocol->getContactListClass()->plugins = plugins;

//	createContacts();
}

icqAccount::~icqAccount()
{
	if ( !deleteingAccount )
		saveAccountSettings();
//	delete thisIcqProtocol;

	delete privacyStatus;
	delete statusMenu;
}

void icqAccount::createAccountButton(QGridLayout *boxLayout, QWidget *line, int  index )
{
	accountLineButton = new accountButton(line);
//#if defined(Q_OS_MAC)
//	boxLayout->addWidget(accountLineButton, 0, Qt::AlignLeft);
//#else
//	boxLayout->addWidget(accountLineButton, 0, Qt::AlignRight);
//#endif
	boxLayout->addWidget(accountLineButton, 0, index);
	accountLineButton->setToolTip(icqUin);
	accountLineButton->setIcon(currentIcon);
	accountLineButton->setPopupMode(QToolButton::InstantPopup);
	accountLineButton->setMenu(statusMenu);
}

void icqAccount::createMenuAccount(QMenu *menu, QAction *before)
{
	accountAction = menu->insertMenu(before, statusMenu);
	menuExist = true;
}

void icqAccount::removeMenuAccount(QMenu *menu)
{
	menuExist = false;
	menu->removeAction(accountAction);
}

void icqAccount::removeAccountButton()
{
	delete accountLineButton;
}

void icqAccount::createSettings(QTreeWidget *settingsTree, QStackedWidget *settingsStack)
{
	settingsChanged = false;
	icqGeneral = new QTreeWidgetItem(settingsTree);
	icqGeneral->setText(0, icqUin);
	icqGeneral->setIcon(0, QIcon(":/icons/qutim.png"));
	generalSettings = new icqSettings;
	settingsStack->addWidget(generalSettings);
	
	
	icqNetwork = new QTreeWidgetItem(settingsTree);
	icqNetwork->setText(0, tr("Network"));
	icqNetwork->setIcon(0, QIcon(":/icons/crystal_project/network.png"));
	icqnetworkSettings = new networkSettings;
	settingsStack->addWidget(icqnetworkSettings);
	
	
	icqCl = new QTreeWidgetItem(settingsTree);
	icqCl->setText(0,tr("Contact list"));
	icqCl->setIcon(0,QIcon(":/icons/crystal_project/contactlist.png"));
	clSettings = new contactListSettings;
	settingsStack->addWidget(clSettings);
	
	msgSett = new QTreeWidgetItem(settingsTree);
	msgSett->setText(0,tr("Messaging"));
	msgSett->setIcon(0,QIcon(":/icons/crystal_project/messaging.png"));
	msgSettings = new messagingSettings;
	settingsStack->addWidget(msgSettings);
	
	statSett = new QTreeWidgetItem(settingsTree);
	statSett->setText(0,tr("Statuses"));
	statSett->setIcon(0,QIcon(":/icons/crystal_project/statuses.png"));
	statSettings = new statusSettings;
	settingsStack->addWidget(statSettings);
	
	histSett = new QTreeWidgetItem(settingsTree);
	histSett->setText(0,tr("History"));
	histSett->setIcon(0,QIcon(":/icons/crystal_project/history.png"));
	histSettings = new historySettings;
	settingsStack->addWidget(histSettings);
	
	eventSett = new QTreeWidgetItem(settingsTree);
	eventSett->setText(0,tr("Events"));
	eventSett->setIcon(0,QIcon(":/icons/crystal_project/events.png"));
	evSettings = new eventsSettings;
	settingsStack->addWidget(evSettings);
	
	antiSpamSett = new QTreeWidgetItem(settingsTree);
	antiSpamSett->setText(0,tr("Anti-spam"));
	antiSpamSett->setIcon(0,QIcon(":/icons/crystal_project/antispam.png"));
	antiSpSettings = new antiSpamSettings;
	settingsStack->addWidget(antiSpSettings);
	
	emoticSett = new QTreeWidgetItem(settingsTree);
	emoticSett->setText(0,tr("Emoticons"));
	emoticSett->setIcon(0,QIcon(":/icons/crystal_project/emoticon.png"));
	emoticSettings = new emoticonSettings;
	settingsStack->addWidget(emoticSettings);
	
	soundSett = new QTreeWidgetItem(settingsTree);
	soundSett->setText(0, tr("Sounds"));
	soundSett->setIcon(0, QIcon(":/icons/crystal_project/soundsett.png"));
	sndSettings = new soundSettings;
	settingsStack->addWidget(sndSettings);
	
	loadAllSettings();
	connect(generalSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(icqnetworkSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(clSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(msgSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(statSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(histSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(evSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(antiSpSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(emoticSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	connect(sndSettings, SIGNAL(settingsChanged()),
			this, SLOT(icqSettingsChanged()));
	
	connect(generalSettings, SIGNAL(settingsSaved()),
			this, SLOT(generalSettingsChanged()));
	
	connect(icqnetworkSettings, SIGNAL(settingsSaved()),
			this, SLOT(networkSettingsChanged()));
	connect(clSettings, SIGNAL(settingsSaved()),
		getProtocol()->getContactListClass(), SLOT(clSettingsChanged()));
	connect(msgSettings, SIGNAL(settingsSaved()),
		getProtocol()->getContactListClass(), SLOT(msgSettingsChanged()));
	connect(statSettings, SIGNAL(settingsSaved()),
		getProtocol()->getContactListClass(), SLOT(statusSettingsChanged()));
	connect(histSettings, SIGNAL(settingsSaved()),
		getProtocol()->getContactListClass(), SLOT(historySettingsChanged()));
	connect(evSettings, SIGNAL(settingsSaved()),
		getProtocol()->getContactListClass(), SLOT(eventsSettingsChanged()));
	connect(evSettings, SIGNAL(settingsSaved()),
						this, SLOT(eventsSettingsChanged()));
	connect(antiSpSettings, SIGNAL(settingsSaved()),
		getProtocol()->getContactListClass(), SLOT(antispamSettingsChanged()));
	connect(emoticSettings, SIGNAL(settingsSaved()),
		getProtocol()->getContactListClass(), SLOT(emoticonSettingsChanged()));
	connect(sndSettings, SIGNAL(settingsSaved()),
		getProtocol()->getContactListClass(),
		SIGNAL(soundSettingsChanged()));
	
	
	connect(evSettings, SIGNAL(showTestEvent(int)),
									this, SLOT(showTestEvent(int)));
	
//	plugins->addSettings(settingsTree, settingsStack);

}


void icqAccount::removeSettings()
{
	delete icqGeneral;
	delete generalSettings;
	delete icqNetwork;
	delete icqnetworkSettings;
	delete icqCl;
	delete clSettings;
	delete msgSett;
	delete msgSettings;
	delete statSett;
	delete statSettings;
	delete histSett;
	delete histSettings;
	delete eventSett;
	delete evSettings;
	delete antiSpSettings;
	delete antiSpamSett;
	delete emoticSett;
	delete emoticSettings;
	delete soundSett;
	delete sndSettings;
	
//	plugins->deleteSettings();
}

void icqAccount::loadAllSettings()
{
	generalSettings->loadSettings(icqUin);
	icqnetworkSettings->loadSettings(icqUin);
	clSettings->loadSettings(icqUin);
	msgSettings->loadSettings(icqUin);
	statSettings->loadSettings(icqUin);
	histSettings->loadSettings(icqUin);
	evSettings->loadSettings(icqUin);
	antiSpSettings->loadSettings(icqUin);
	emoticSettings->loadSettings(icqUin);
	sndSettings->loadSettings(icqUin);
}

void icqAccount::saveAllSettings()
{
	generalSettings->saveSettings(icqUin);
	icqnetworkSettings->saveSettings(icqUin);
	clSettings->saveSettings(icqUin);
	msgSettings->saveSettings(icqUin);
	statSettings->saveSettings(icqUin);
	histSettings->saveSettings(icqUin);
	evSettings->saveSettings(icqUin);
	antiSpSettings->saveSettings(icqUin);
	emoticSettings->saveSettings(icqUin);
	sndSettings->saveSettings(icqUin);

	settingsChanged = false;
}

void icqAccount::icqSettingsChanged()
{
	emit changeSettingsApply();
	settingsChanged = true;
}

void icqAccount::createIcons()
{
	/*onlineIcon = new QIcon(statusIconClass::getInstance()->getOnlineIcon());
	offlineIcon = new QIcon(statusIconClass::getInstance()->getOfflineIcon());
	ffcIcon = new QIcon(statusIconClass::getInstance()->getFreeForChatIcon());
	awayIcon = new QIcon(statusIconClass::getInstance()->getAwayIcon());
	naIcon = new QIcon(statusIconClass::getInstance()->getNotAvailableIcon());
	occupiedIcon = new QIcon(statusIconClass::getInstance()->getOccupiedIcon());
	dndIcon = new QIcon(statusIconClass::getInstance()->getDoNotDisturbIcon());
	invisibleIcon = new QIcon(statusIconClass::getInstance()->getInvisibleIcon());
	connectingIcon = new QIcon(statusIconClass::getInstance()->getConnectingIcon());
	currentIcon = new QIcon(statusIconClass::getInstance()->getOfflineIcon());
	
	currentIconPath = statusIconClass::getInstance()->getIconFactory()->getOfflinePath();
	
	lunchIcon = new QIcon(statusIconClass::getInstance()->getLunchIcon());
	evilIcon = new QIcon(statusIconClass::getInstance()->getEvilIcon());
	depressionIcon = new QIcon(statusIconClass::getInstance()->getDepressionIcon());
	atHomeIcon = new QIcon(statusIconClass::getInstance()->getAtHomeIcon());
	atWorkIcon = new QIcon(statusIconClass::getInstance()->getAtWorkIcon());
	*/
	currentIcon = statusIconClass::getInstance()->getOfflineIcon();
	currentIconPath = statusIconClass::getInstance()->getIconFactory()->getOfflinePath();	
	
}
void icqAccount::createStatusMenu()
{
	
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	showCustomStatus = settings.value("statuses/customstat",true).toBool();
	
	onlineAction = new QAction(statusIconClass::getInstance()->getOnlineIcon(), tr("Online"), this);
		connect(onlineAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(onlineAction);

		offlineAction = new QAction(statusIconClass::getInstance()->getOfflineIcon(), tr("Offline"), this);
		connect(offlineAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(offlineAction);

		ffcAction = new QAction(statusIconClass::getInstance()->getFreeForChatIcon(), tr("Free for chat"), this);
		connect(ffcAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(ffcAction);

		awayAction = new QAction(statusIconClass::getInstance()->getAwayIcon(), tr("Away"), this);
		connect(awayAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(awayAction);

		naAction = new QAction(statusIconClass::getInstance()->getNotAvailableIcon(), tr("NA"), this);
		connect(naAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(naAction);

		occupiedAction = new QAction(statusIconClass::getInstance()->getOccupiedIcon(), tr("Occupied"), this);
		connect(occupiedAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(occupiedAction);

		dndAction = new QAction(statusIconClass::getInstance()->getDoNotDisturbIcon(), tr("DND"), this);
		connect(dndAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(dndAction);

		invisibleAction = new QAction(statusIconClass::getInstance()->getInvisibleIcon(), tr("Invisible"), this);
		connect(invisibleAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(invisibleAction);

		lunchAction = new QAction(statusIconClass::getInstance()->getLunchIcon(), tr("Lunch"), this);
		connect(lunchAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(lunchAction);

		evilAction = new QAction(statusIconClass::getInstance()->getEvilIcon(), tr("Evil"), this);
		connect(evilAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(evilAction);

		depressionAction = new QAction(statusIconClass::getInstance()->getDepressionIcon(), tr("Depression"), this);
		connect(depressionAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(depressionAction);

		atHomeAction = new QAction(statusIconClass::getInstance()->getAtHomeIcon(), tr("At Home"), this);
		connect(atHomeAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(atHomeAction);

		atWorkAction = new QAction(statusIconClass::getInstance()->getAtWorkIcon(), tr("At Work"), this);
		connect(atWorkAction, SIGNAL(triggered()), this, SLOT(setStatus()));
		statusMenuActions.push_back(atWorkAction);

	// Set all of status actions as unchecked
	QVectorIterator<QAction *> iterator(statusMenuActions);
	while (iterator.hasNext())
	{
		iterator.next()->setCheckable(true);
	}

	customStatus = new QAction(tr("Custom status"), this);
	connect(customStatus, SIGNAL(triggered()), this, SLOT(customStatusTriggered()));
	
	statusMenu = new QMenu;
	privacyStatus = new QMenu(tr("Privacy status"));
	privacyStatus->setIcon(QIcon(":/icons/crystal_project/privacy.png"));
	
	privacyGroup = new QActionGroup(this);
	
	visibleForAll = new QAction(tr("Visible for all"), this);
	visibleForAll->setCheckable(true);
	privacyGroup->addAction(visibleForAll);
	connect(visibleForAll, SIGNAL(triggered()), this, SLOT(setVisibleForAll()));
	
	visibleForVis = new QAction(tr("Visible only for visible list"), this);
	visibleForVis->setCheckable(true);
	privacyGroup->addAction(visibleForVis);
	connect(visibleForVis, SIGNAL(triggered()), this, SLOT(setVisibleForVis()));
	
	notVisibleForInv = new QAction(tr("Invisible only for invisible list"), this);
	notVisibleForInv->setCheckable(true);
	privacyGroup->addAction(notVisibleForInv);
	connect(notVisibleForInv, SIGNAL(triggered()), this, SLOT(setNotVisibleForInv()));
	
	visibleForContact = new QAction(tr("Visible only for contact list"), this);
	visibleForContact->setCheckable(true);
	privacyGroup->addAction(visibleForContact);
	connect(visibleForContact, SIGNAL(triggered()), this, SLOT(setVisibleForContact()));
		
	invisibleForAll = new QAction(tr("Invisible for all"), this);
	invisibleForAll->setCheckable(true);
	privacyGroup->addAction(invisibleForAll);
	connect(invisibleForAll, SIGNAL(triggered()), this, SLOT(setInvisibleForAll()));
	
	privacyStatus->addAction(visibleForAll);
	privacyStatus->addAction(visibleForVis);
	privacyStatus->addAction(notVisibleForInv);
	privacyStatus->addAction(visibleForContact);
	privacyStatus->addAction(invisibleForAll);
	
	
	
	statusMenu->setTitle(icqUin);
	statusMenu->setIcon(currentIcon);
	statusMenu->addAction(onlineAction);
	statusMenu->addAction(ffcAction);
	statusMenu->addAction(awayAction);
	
	statusMenu->addAction(lunchAction);
	statusMenu->addAction(evilAction);
	statusMenu->addAction(depressionAction);
	statusMenu->addAction(atHomeAction);
	statusMenu->addAction(atWorkAction);
	
	statusMenu->addAction(naAction);
	statusMenu->addAction(occupiedAction);
	statusMenu->addAction(dndAction);
	statusMenu->addAction(invisibleAction);
	statusMenu->addSeparator();
	statusMenu->addAction(customStatus);
	statusMenu->addSeparator();
	statusMenu->addMenu(privacyStatus);
	statusMenu->addAction(offlineAction);
	
	updateStatusMenu(showCustomStatus);
	
	
	
	quint32 privacy = settings.value("statuses/privacy", 4).toUInt();
	
	switch( privacy )
	{
	case 1:
		visibleForAll->setChecked(true);
		break;
	case 2:
		visibleForVis->setChecked(true);
		break;
	case 3:
		notVisibleForInv->setChecked(true);
		break;
	case 4:
		visibleForContact->setChecked(true);
		break;
	case 5:
		invisibleForAll->setChecked(true);
		break;
	default:	
		visibleForContact->setChecked(true);
	}
	
}


void icqAccount::setStatusIcon(accountStatus status)
{
	switch (status)
	{
	case online:
		currentIcon = statusIconClass::getInstance()->getOnlineIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getOnlinePath();
		break;
	
	case ffc:
		currentIcon = statusIconClass::getInstance()->getFreeForChatIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getFreeForChatPath();
		break;
		
	case away:
		currentIcon = statusIconClass::getInstance()->getAwayIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getAwayPath();
		break;
		
	case na:
		currentIcon = statusIconClass::getInstance()->getNotAvailableIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getNotAvailablePath();
		break;
		
	case occupied:
		currentIcon = statusIconClass::getInstance()->getOccupiedIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getOccupiedPath();;
		break;
	
	case dnd:
		currentIcon = statusIconClass::getInstance()->getDoNotDisturbIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getDoNotDisturbPath();
		break;
		
	case invisible:
		currentIcon = statusIconClass::getInstance()->getInvisibleIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getInvisiblePath();;
		break;
		
	case offline:
		currentIcon = statusIconClass::getInstance()->getOfflineIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getOfflinePath();
		iAmConnected = false;
//              emit playSoundEvent(SoundEvent::Disconnected, getStatus());
		break;
		
	case connecting:
		currentIcon = statusIconClass::getInstance()-> getConnectingIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getConnectingPath();
		break;
	
	case lunch:
		currentIcon = statusIconClass::getInstance()->getLunchIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getLunchPath();
		break;
		
	case evil:
		currentIcon = statusIconClass::getInstance()->getEvilIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getEvilPath();
		break;
			
	case depression:
		currentIcon = statusIconClass::getInstance()->getDepressionIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getDepressionPath();
		break;
	
	case athome:
		currentIcon = statusIconClass::getInstance()->getAtHomeIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getAtHomePath();
		break;
	case atwork:
		currentIcon = statusIconClass::getInstance()->getAtWorkIcon();
		currentIconPath = statusIconClass::getInstance()->getIconFactory()->getAtWorkPath();
		break;
			
	default:
		break;
	}
	
	if ( statusIconIndex != 1)
	{
		updateIconStatus();
		emit updateTrayToolTip();
	} else
	{
		if ( !currentXstatus || status==offline)
		{
			updateIconStatus();
			emit updateTrayToolTip();
		} else if (iAmConnected){
			currentIconPath = statusIconClass::getInstance()->xstatusList.at(currentXstatus - 1);
			currentIcon = QIcon(currentIconPath);	
			updateIconStatus();

			emit updateTrayToolTip();
		}

			
		
	}
}

void icqAccount::onOscarStatusChanged(accountStatus status)
{
	// Set all of status actions as unchecked
	QVectorIterator<QAction *> iterator(statusMenuActions);
	while (iterator.hasNext())
	{
		iterator.next()->setChecked(false);
	}

	if (status == offline) 
	{
		offlineAction->setChecked(true);
	}
	else if (status == online) 
	{
		onlineAction->setChecked(true);
	}
	else if (status == ffc)
	{
		ffcAction->setChecked(true);
	}
	else if (status == away) 
	{
		awayAction->setChecked(true);
	}
	else if (status == na) 
	{
		naAction->setChecked(true);
	}
	else if (status == occupied) 
	{
		occupiedAction->setChecked(true);
	}
	else if (status == dnd) 
	{
		dndAction->setChecked(true); 
	}
	else if (status == invisible) 
	{
		invisibleAction->setChecked(true);
	}
	else if (status == lunch) 
	{
		lunchAction->setChecked(true);
	}
	else if (status ==  evil) 
	{
		evilAction->setChecked(true);
	}
	else if (status == depression) 
	{
		depressionAction->setChecked(true);
	}
	else if (status == athome) 
	{
		atHomeAction->setChecked(true);
	}
	else if (status == atwork) 
	{
		atWorkAction->setChecked(true);
	}
}

void icqAccount::setStatus()
// shold not be used directly!!!
// use statusAction->trigger() if you have to
// call setStatus() slot
{
	// default status
	accountStatus status = online;
	// Getting triggered action
	QAction *act = qobject_cast<QAction *>(sender());

	// Set all of status actions as unchecked
	QVectorIterator<QAction *> iterator(statusMenuActions);
	while (iterator.hasNext())
	{
		iterator.next()->setChecked(false);
	}

	// Set button checked
	//act->setChecked(true);

	// setting status
	if (act == offlineAction)status = offline;
	else if (act == ffcAction) status = ffc;
	else if (act == awayAction) status = away;
	else if (act == naAction) status = na;
	else if (act == occupiedAction) status = occupied;
	else if (act == dndAction) status = dnd;
	else if (act == invisibleAction) status = invisible;
	else if (act == lunchAction) status = lunch;
	else if (act == evilAction) status = evil;
	else if (act == depressionAction) status = depression;
	else if (act == atHomeAction) status = athome;
	else if (act == atWorkAction) status = atwork;

	// Saving settings routine
	if ((status != online) && (status != offline) && (status != ffc)
		&& (status != invisible))
	{
		QString sName(StatusNames[static_cast<int>(status)]);
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
		if (!settings.value("autoreply/" + sName + "dshow", false).toBool())
		{
			statusDialog sDialog;
			sDialog.setStatusMessage(settings.value("autoreply/" + sName
				+ "msg", "").toString());
			if ( sDialog.exec() )
			{
				settings.setValue("autoreply/" + sName + "dshow", sDialog.dontShow);
				settings.setValue("autoreply/" + sName
					+ "msg", sDialog.statusMessage.left(1000));
				thisIcqProtocol->setStatus(status);
			}
		} else 
			thisIcqProtocol->setStatus(status);


	}
	// Disconnecting
	else
	{
		if (status == offline)
			thisIcqProtocol->userDisconnected = true;
		
		// Apply new status
		thisIcqProtocol->setStatus(status);
	}
}

void icqAccount::updateIconStatus()
{
	accountLineButton->setIcon(currentIcon);
	if ( menuExist )
		accountAction->setIcon(currentIcon);
	if ( statusTrayMenuExist)
		chooseStatus->setIcon(currentIcon);
	
	if ( currentTrayStatus )
		emit statusChanged(currentIcon);
}

void icqAccount::createTrayMenuStatus(QMenu *menu)
{
	menu->addAction(chooseStatus);
	chooseStatus->setIcon(currentIcon);
	statusTrayMenuExist = true;
}

void icqAccount::removeTrayMenuStatus(QMenu *menu)
{
	menu->removeAction(chooseStatus);
	statusTrayMenuExist = false;
}

void icqAccount::emitChangeStatus()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	settings.setValue("systray/current", icqUin);
	emit changeStatusInTrayMenu(icqUin); 
}

void icqAccount::loadAccountSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	autoConnect = settings.value("connection/auto", true).toBool();
	statusIconIndex = settings.value("main/staticon", 0).toInt();
	thisIcqProtocol->reconnectOnDisc = settings.value("connection/reconnect", true).toBool();
	
	settings.beginGroup("clientid");
	
	clientIndex = settings.value("index", 0).toUInt();
	protocolVersion = settings.value("protocol", 1).toUInt();

		clientCap1 = settings.value("cap1").toString();
		clientCap2 = settings.value("cap2").toString();
		clientCap3 = settings.value("cap3").toString();
	
	settings.endGroup();
	
	
	networkSettingsChanged();
	
	
	settings.beginGroup("events");
	showBalloon = settings.value("showballoon", false).toBool();
	
	balloonTime = settings.value("balloontime", 5).toInt();
	balloonTime = balloonTime < 1 ? 1 : balloonTime;
	balloonTime = balloonTime > 60 ? 60 : balloonTime;
	
	dontShowifNA = settings.value("dshowna", false).toBool();
	
	showTrayMessages = settings.value("showtray", true).toBool();
	
	int width = settings.value("width", 200).toInt();
	width = width < 100 ? 100 : width;
	width = width > 500 ? 500 : width;
	trayMessageWidth = width;
	
	int height = settings.value("height", 150).toInt();
	height = height < 100 ? 100 : height;
	height = height > 400 ? 400 : height;
	trayMessageHeight = height;
	
	int trayTime = settings.value("traytime", 6).toInt();
	trayTime = trayTime < 1 ? 1 : trayTime;
	trayTime = trayTime > 60 ? 60 : trayTime;
	trayMessageTime = trayTime;
	
	
	int position = settings.value("position", 3).toInt();
	position = position < 0 ? 0 : position;
	position= position > 3 ? 3 : position;
	trayMessagePosition = static_cast<TrayPosition>(position);
	
	int style = settings.value("style", 2).toInt();
	style = style < 0 ? 0 : style;
	style = style > 2 ? 2 : style;
	trayMessageStyle = style;
	
	readTrayThemes(settings.value("path", "").toString());
	
	settings.endGroup();
	
	currentXstatus = settings.value("xstatus/index", 0).toInt();
	if ( currentXstatus )
	{
		customStatus->setIcon(QIcon(statusIconClass::getInstance()->xstatusList.at(currentXstatus - 1)));
	}else 
		customStatus->setIcon(QIcon());
}

void icqAccount::saveAccountSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	if (getStatus() != offline)
	{
		settings.setValue("connection/currstatus", getStatus());
	}
	else
	{
		settings.remove("connection/currstatus");
	}
}

void icqAccount::autoconnecting()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	if ( settings.value("connection/statonexit", true).toBool() )
		thisIcqProtocol->setStatus(static_cast<accountStatus>(settings.value("connection/currstatus", 0).toInt()));
	else
		thisIcqProtocol->setStatus(online);
}

void icqAccount::createContacts()
{
		
}

void icqAccount::systemMessage(const QString &message)
{
//	bool showTrue = true;
	
//	if ( dontShowifNA )
//	{
//		if (getStatus() == na || getStatus() == occupied)
//			showTrue = false;
//	}
	
	bool showTrue = !(dontShowifNA && (getStatus() == na || getStatus() == occupied));

	if (showBalloon && showTrue)
		trayIcon->showMessage(tr("System message for : %1").arg(icqUin), message, QSystemTrayIcon::NoIcon, balloonTime * 1000);
	if (showTrayMessages && showTrue)
		traySystemMessage(message);
//              emit playSoundEvent(SoundEvent::SystemEvent, getStatus());
}

void icqAccount::userMessage(const QString &fromUin, const QString &from, const QString &message, userMessageType type, bool fromList)
{
//	bool showTrue = true;
	QString trayMessageMsg;
		
	bool showTrue = !(dontShowifNA && (getStatus() == na || getStatus() == occupied));
	
	QString msg;
	switch ( type )
	{
	case statusNotyfication:	
				msg = from + " " + message;
				trayMessageMsg =  message;
				break;
			case messageNotification:
				msg = tr("Message from %1:\n%2").arg(from).arg(message);
				trayMessageMsg = message;

				// Play sound for incoming message
//				emit playSoundEvent(SoundEvents::MessageGet,
//					thisIcqProtocol->getStatus());
				break;
			case readNotification:
				msg = tr("%1 is reading your away message").arg(from);
				trayMessageMsg = tr("reading your away message");
				if ( !fromList )
					msg.append(tr("(not from list)"));
				
				
				break;
			case xstatusReadNotification:
				msg = tr("%1 is reading your xStatus message").arg(from);
				trayMessageMsg = tr("reading your xStatus message");
				if ( !fromList )
					msg.append(tr("(not from list)"));
				
				
				break;	
			case typingNotification:
				msg = tr("%1 is typing").arg(from);
				trayMessageMsg = tr("typing");
				
				if ( !fromList )
					msg.append(tr("(not from list)"));
				
				break;
			case blockedMessage:
				msg = tr("Blocked message from %1:\n%2").arg(from).arg(message);
				trayMessageMsg = "(BLOCKED)\n" + message;
				break;
			default:
				;
	}
	
	if ( showBalloon && showTrue)
	{
		
	trayIcon->showMessage(tr("User message for : %1").arg(icqUin), msg, QSystemTrayIcon::NoIcon, balloonTime * 1000);
	}
	
	if ( showTrayMessages && showTrue )
	{
		trayUserMessage(fromUin, from, trayMessageMsg,type);
	}
}

void icqAccount::removeContactList()
{
	thisIcqProtocol->removeContactList();
}

void icqAccount::readMessageStack()
{
	thisIcqProtocol->readreadMessageStack();
}

void icqAccount::networkSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	thisIcqProtocol->sendKeepAlive(settings.value("connection/alive", true).toBool());
	
}

void icqAccount::updateStatusMenu(bool f)
{
	showCustomStatus = f;
	lunchAction->setVisible(f);
	evilAction->setVisible(f);
	depressionAction->setVisible(f);
	atHomeAction->setVisible(f);
	atWorkAction->setVisible(f);
}

void icqAccount::setVisibleForAll()
{
	thisIcqProtocol->getContactListClass()->changePrivacy(1);
}

void icqAccount::setVisibleForVis()
{
	thisIcqProtocol->getContactListClass()->changePrivacy(2);
}

void icqAccount::setNotVisibleForInv()
{
	thisIcqProtocol->getContactListClass()->changePrivacy(3);	
}

void icqAccount::setVisibleForContact()
{
	thisIcqProtocol->getContactListClass()->changePrivacy(4);
}

void icqAccount::setInvisibleForAll()
{
	thisIcqProtocol->getContactListClass()->changePrivacy(5);
}

void icqAccount::eventsSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	
	settings.beginGroup("events");
	showBalloon = settings.value("showballoon", false).toBool();
		
	balloonTime = settings.value("balloontime", 5).toInt();
	balloonTime = balloonTime < 1 ? 1 : balloonTime;
	balloonTime = balloonTime > 60 ? 60 : balloonTime;
		
	dontShowifNA = settings.value("dshowna", false).toBool();
	
	
	showTrayMessages = showTrayMessages = settings.value("showtray", true).toBool();
	
	int width = settings.value("width", 200).toInt();
	width = width < 100 ? 100 : width;
	width = width > 500 ? 500 : width;
	trayMessageWidth = width;
	
	int height = settings.value("height", 150).toInt();
	height = height < 100 ? 100 : height;
	height = height > 400 ? 400 : height;
	trayMessageHeight = height;
	
	int trayTime = settings.value("traytime", 6).toInt();
	trayTime = trayTime < 1 ? 1 : trayTime;
	trayTime = trayTime > 60 ? 60 : trayTime;
	trayMessageTime = trayTime;
	
	
	int position = settings.value("position", 3).toInt();
	position = position < 0 ? 0 : position;
	position= position > 3 ? 3 : position;
	trayMessagePosition = static_cast<TrayPosition>(position);
	
	int style = settings.value("style", 2).toInt();
	style = style < 0 ? 0 : style;
	style = style > 2 ? 2 : style;
	trayMessageStyle = style;
	
	readTrayThemes(settings.value("path", "").toString());
	
	settings.endGroup();	
}

void icqAccount::traySystemMessage(const QString &msg)
{
	if ( !firsTrayMessageIsShown )
		{
				firsTrayMessageIsShown = true;
				positionInStack = 1;
		}
		
		trayMessage *trayMessageWindow = new trayMessage("", trayMessageWidth, 
				trayMessageHeight, trayMessageTime, trayMessagePosition, 
				trayMessageStyle, positionInStack , false);
		
		trayMessageWindow->setTheme(trayThemeHeaderSys, trayThemeHeaderSysCSS, trayThemeContentSys, trayThemeContentSysCSS, trayThemePath);
		trayMessageWindow->setSystemData(thisIcqProtocol->getContactListClass()->accountNickname,msg);
		
		
		trayMessageWindow->firstTrayWindow = firsTrayMessageIsShown;
		
		connect( trayMessageWindow, SIGNAL(destroyed ( QObject *)),
								this, SLOT(deleteTrayWindow(QObject *)));
		
		trayMessageWindow->showTrayMessage();
		
		if ( (++positionInStack) > 3)
			positionInStack = 3;
		
		trayMessagesList.append(trayMessageWindow);
}

void icqAccount::trayUserMessage(const QString &fromUin, const QString &from, const QString &msg,userMessageType type )
{
	
	if ( !firsTrayMessageIsShown )
	{
			firsTrayMessageIsShown = true;
			positionInStack = 1;
	}
	
	trayMessage *trayMessageWindow = new trayMessage(fromUin, trayMessageWidth, 
			trayMessageHeight, trayMessageTime, trayMessagePosition, 
			trayMessageStyle, positionInStack );
	
	switch(type)
	{
	case messageNotification:
		trayMessageWindow->setTheme(trayThemeHeaderMsg, trayThemeHeaderMsgCSS, trayThemeContentMsg, trayThemeContentMsgCSS, trayThemePath);
		break;
	default:
		trayMessageWindow->setTheme(trayThemeHeaderOnl, trayThemeHeaderOnlCSS, trayThemeContentOnl, trayThemeContentOnlCSS, trayThemePath);
	}
	trayMessageWindow->setData(thisIcqProtocol->getContactListClass()->accountNickname,
			from, getIconPathForUin(fromUin), msg);
	
	
	trayMessageWindow->firstTrayWindow = firsTrayMessageIsShown;
	
	connect( trayMessageWindow, SIGNAL(destroyed ( QObject *)),
							this, SLOT(deleteTrayWindow(QObject *)));
	connect( trayMessageWindow, SIGNAL(startChat ( const QString &)),
								thisIcqProtocol->getContactListClass(), SLOT(startChatWith ( const QString &)));
	
	trayMessageWindow->showTrayMessage();
	
	if ( (++positionInStack) > 3)
		positionInStack = 3;
	
	trayMessagesList.append(trayMessageWindow);
}

void icqAccount::deleteTrayWindow(QObject *obj)
{
	trayMessage *tempWindow = (trayMessage *)(obj);
	
	if ( tempWindow->firstTrayWindow )
		firsTrayMessageIsShown = false;
	
	trayMessagesList.removeAll(tempWindow);
}

QString icqAccount::getIconPathForUin(const QString &uin) const
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "contacts");
	
	
	QString hashName = settings.value((uin + "/iconhash"), "").toByteArray();
	if ( !hashName.isEmpty( ) )
		return (configPath + "/icqicons/" + hashName);
	else
		return QString("");
}

void icqAccount::generalSettingsChanged()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	thisIcqProtocol->getContactListClass()->setAvatarDisabled(settings.value("connection/disavatars", false).toBool());
	thisIcqProtocol->reconnectOnDisc = settings.value("connection/reconnect", true).toBool();
	
	int statindex = settings.value("main/staticon", 0).toInt();
	
	if ( statusIconIndex != statindex )
	{
		statusIconIndex = statindex;
		
		if ( statusIconIndex != 1)
		{
			updateIconStatus();
			emit updateTrayToolTip();
		} else
		{
			if ( !currentXstatus)
				updateIconStatus();
				emit updateTrayToolTip();
		}
		
		if (currentXstatus )
		{
			if ( statusIconIndex == 2 || statusIconIndex == 1)
			{
				currentIconPath = statusIconClass::getInstance()->xstatusList.at(currentXstatus - 1);
				currentIcon = QIcon(currentIconPath);
				
				updateIconStatus();
				emit updateTrayToolTip();
			}			else
				setStatusIcon(getStatus());
		}
		else
		{
			setStatusIcon(getStatus());
		}
		}
	
	settings.beginGroup("clientid");
	
	unsigned newClientIndex = settings.value("index", 0).toUInt();
	unsigned newProtocolVersion = settings.value("protocol", 1).toUInt();

	QString newClientCap1 = settings.value("cap1").toString();
	QString	newClientCap2 = settings.value("cap2").toString();
	QString	newClientCap3 = settings.value("cap3").toString();
	
	settings.endGroup();
	
	if (checkClientIdentification(newClientIndex, newProtocolVersion,
			newClientCap1, newClientCap2, newClientCap3))
		thisIcqProtocol->resendCapabilities();
	
}

bool icqAccount::checkClientIdentification(unsigned index, unsigned pVersion, const QString &cap1, const QString &cap2, const QString &cap3)
{
	bool changed = false;
	
	changed = (index == clientIndex ? changed : true); 
	changed = (pVersion == protocolVersion ? changed : true); 
	changed = (cap1 == clientCap1 ? changed : true); 
	changed = (cap2 == clientCap2 ? changed : true); 
	changed = (cap3 == clientCap3 ? changed : true); 
	
	
	clientIndex = index;
	protocolVersion = pVersion;
	clientCap1 = cap1;
	clientCap2 = cap2;
	clientCap3 = cap3;
	
	return changed;
}

void icqAccount::customStatusTriggered()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+icqUin, "account");
	settings.beginGroup("xstatus");
	customStatusDialog dialog(icqUin);
	QPoint point = accountLineButton->mapToGlobal(QPoint(0,0));
	dialog.move(point.x() - dialog.width(), point.y() - dialog.height());
	dialog.setStatuses(settings.value("index", 0).toInt(), statusIconClass::getInstance()->xstatusList);
	
	settings.endGroup();
	

	if ( dialog.exec() )
	{

		currentXstatus = dialog.status;
		if ( currentXstatus )
		{
			customStatus->setIcon(QIcon(statusIconClass::getInstance()->xstatusList.at(currentXstatus - 1)));
		} else 
			customStatus->setIcon(QIcon());
		thisIcqProtocol->sendOnlyCapabilities();
		
		
		if (currentXstatus )
		{
			if ( statusIconIndex == 2 || statusIconIndex == 1)
			{
				currentIconPath = statusIconClass::getInstance()->xstatusList.at(currentXstatus - 1);
				currentIcon = QIcon(currentIconPath);				
				updateIconStatus();

				emit updateTrayToolTip();
			}
			else
				setStatusIcon(getStatus());
		}
		else
		{
			setStatusIcon(getStatus());
		}
		}
}

void icqAccount::onUpdateTranslation()
{
	onlineAction->setText(tr("Online"));
	offlineAction->setText(tr("Offline"));
	ffcAction->setText(tr("Free for chat"));
	awayAction->setText(tr("Away"));
	naAction->setText(tr("NA"));
	occupiedAction->setText(tr("Occupied"));
	dndAction->setText(tr("DND"));
	invisibleAction->setText(tr("Invisible"));
	lunchAction->setText(tr("Lunch"));
	evilAction->setText(tr("Evil"));
	depressionAction->setText(tr("Depression"));
	atHomeAction->setText(tr("At Home"));
	atWorkAction->setText(tr("At Work"));
	customStatus->setText(tr("Custom status"));

	privacyStatus->setTitle(tr("Privacy status"));

	visibleForAll->setText(tr("Visible for all"));
	visibleForVis->setText(tr("Visible only for visible list"));
	notVisibleForInv->setText(tr("Invisible only for invisible list"));
	visibleForContact->setText(tr("Visible only for contact list"));
	invisibleForAll->setText(tr("Invisible for all"));

	emit updateTranslation();
}

void icqAccount::onReloadGeneralSettings()
{
	// Update status icons for status menu
	onlineAction->setIcon(statusIconClass::getInstance()->getOnlineIcon());
	offlineAction->setIcon(statusIconClass::getInstance()->getOfflineIcon());
	ffcAction->setIcon(statusIconClass::getInstance()->getFreeForChatIcon());
	awayAction->setIcon(statusIconClass::getInstance()->getAwayIcon());
	naAction->setIcon(statusIconClass::getInstance()->getNotAvailableIcon());
	occupiedAction->setIcon(statusIconClass::getInstance()->getOccupiedIcon());
	dndAction->setIcon(statusIconClass::getInstance()->getDoNotDisturbIcon());
	invisibleAction->setIcon(statusIconClass::getInstance()->getInvisibleIcon());
	lunchAction->setIcon(statusIconClass::getInstance()->getLunchIcon());
	evilAction->setIcon(statusIconClass::getInstance()->getEvilIcon());
	depressionAction->setIcon(statusIconClass::getInstance()->getDepressionIcon());
	atHomeAction->setIcon(statusIconClass::getInstance()->getAtHomeIcon());
	atWorkAction->setIcon(statusIconClass::getInstance()->getAtWorkIcon());
	accountLineButton->setIcon(currentIcon);

	//Update current icon
	setStatusIcon(getStatus());
	// Update button with status menu
	thisIcqProtocol->onReloadGeneralSettings();
}

void icqAccount::readTrayThemes(const QString &path)
{
	trayThemePath = path;
	
	if ( trayThemePath.isEmpty() )
	{
		trayThemeContentMsg.clear();
		trayThemeContentMsgCSS.clear();
		trayThemeContentOnl.clear();
		trayThemeContentOnlCSS.clear();
		trayThemeContentSys.clear();
		trayThemeContentSysCSS.clear();
		trayThemeHeaderMsg.clear();
		trayThemeHeaderMsgCSS.clear();
		trayThemeHeaderOnl.clear();
		trayThemeHeaderOnlCSS.clear();
		trayThemeHeaderSys.clear();
		trayThemeHeaderSysCSS.clear();
	} else 
	{
	QFile fileOnlHead(path + "/onlalert/header.html");
	
	if ( fileOnlHead.open(QIODevice::ReadOnly))
	{
		trayThemeHeaderOnl = fileOnlHead.readAll();
		fileOnlHead.close();
	}

	QFile fileOnlHeadCSS(path + "/onlalert/header.css");
	
	if ( fileOnlHeadCSS.open(QIODevice::ReadOnly))
	{
		trayThemeHeaderOnlCSS = fileOnlHeadCSS.readAll();
		fileOnlHeadCSS.close();
	}
	
	QFile fileOnlCont(path + "/onlalert/content.html");
	
	if ( fileOnlCont.open(QIODevice::ReadOnly))
	{
		trayThemeContentOnl = fileOnlCont.readAll();
		fileOnlCont.close();
	}
	
	QFile fileOnlContCSS(path + "/onlalert/content.css");
	
	if ( fileOnlContCSS.open(QIODevice::ReadOnly))
	{
		trayThemeContentOnlCSS = fileOnlContCSS.readAll();
		fileOnlContCSS.close();
	}

	QFile fileMsgHead(path + "/msg/header.html");
	
	if ( fileMsgHead.open(QIODevice::ReadOnly))
	{
		trayThemeHeaderMsg = fileMsgHead.readAll();
		fileMsgHead.close();
	}
	
	QFile fileMsgHeadCSS(path + "/msg/header.css");
	
	if ( fileMsgHeadCSS.open(QIODevice::ReadOnly))
	{
		trayThemeHeaderMsgCSS = fileMsgHeadCSS.readAll();
		fileMsgHeadCSS.close();
	}
	

	QFile fileMsgCont(path + "/msg/content.html");
	
	if ( fileMsgCont.open(QIODevice::ReadOnly))
	{
		trayThemeContentMsg = fileMsgCont.readAll();
		fileMsgCont.close();
	}
	
	QFile fileMsgContCSS(path + "/msg/content.css");
	
	if ( fileMsgContCSS.open(QIODevice::ReadOnly))
	{
		trayThemeContentMsgCSS = fileMsgContCSS.readAll();
		fileMsgContCSS.close();
	}

	QFile fileSysHead(path + "/system/header.html");
	
	if ( fileSysHead.open(QIODevice::ReadOnly))
	{
		trayThemeHeaderSys = fileSysHead.readAll();
		fileSysHead.close();
	}
	
	QFile fileSysHeadCSS(path + "/system/header.css");
	
	if ( fileSysHeadCSS.open(QIODevice::ReadOnly))
	{
		trayThemeHeaderSysCSS = fileSysHeadCSS.readAll();
		fileSysHeadCSS.close();
	}

	QFile fileSysCont(path + "/system/content.html");
	
	if ( fileSysCont.open(QIODevice::ReadOnly))
	{
		trayThemeContentSys = fileSysCont.readAll();
		fileSysCont.close();
	}

	QFile fileSysContCSS(path + "/system/content.css");
	
	if ( fileSysContCSS.open(QIODevice::ReadOnly))
	{
		trayThemeContentSysCSS = fileSysContCSS.readAll();
		fileSysContCSS.close();
	}
	}
}

void icqAccount::setChooseStatusCheck(bool check)
{
	chooseStatus->setChecked(check);
}

void icqAccount::showTestEvent(int type)
{

	switch (type) {
	case 0:
		trayUserMessage("1", tr("Test-bot"), tr("Hi!"), messageNotification);
		break;
	case 1:
		trayUserMessage("1", tr("Test-bot"), tr("Hi!"), statusNotyfication);
		break;
	case 2:
		traySystemMessage(tr("System message!"));
		break;
	}
}
