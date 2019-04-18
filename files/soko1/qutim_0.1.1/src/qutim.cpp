/*
    qutim

    Copyright (c) 2008 by Rustam Chakin <qutim.develop@gmail.com>
						  Dmitri Arekhta <DaemonES@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#include <QMutexLocker>
#include <QDesktopWidget>
#include <QCloseEvent>
#include <QSettings>
#include <QMutex>
#include <QDebug>
#include <QMenu>

#include "aboutinfo.h"
#include "qutimsettings.h"
#include "logindialog.h"
#include "contactseparator.h"

#include "qutim.h"

#if defined(Q_OS_WIN32)
#include "windows.h"

#pragma comment(lib, "user32.lib")
#endif

#if defined(Q_OS_MAC)
// macx specific functions, that exported by QT GUI lib
extern void qt_mac_set_dock_menu(QMenu *menu);
#endif

#if defined(_MSC_VER)
#pragma warning (disable:4138)
#endif

bool eventEater::eventFilter(QObject *obj, QEvent *event)
{
	if ( event->type() == QEvent::MouseButtonDblClick ||
			event->type() == QEvent::MouseButtonPress ||
			event->type() == QEvent::MouseButtonRelease ||
			event->type() == QEvent::MouseMove ||
			event->type() == QEvent::MouseTrackingChange ||
			event->type() == QEvent::KeyPress ||
			event->type() == QEvent::KeyRelease ||
			event->type() == QEvent::KeyboardLayoutChange)
	{
		fChanged = true;
	}
	
	return QObject::eventFilter(obj, event);
}


// Statics

qutIM				*qutIM::fInstance = NULL;
QMutex				qutIM::fInstanceGuard;



qutIM::qutIM(QWidget *parent) : 
	QWidget(parent),
	bShouldRun(true),
	fWindowStyle(CLRegularWindow)
{
//	qApp->installTranslator(&applicationTranslator);

	onlineList = NULL;
	offlineList = NULL;
	quitAction = NULL;

	readTranslation();
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(hide()));
	
	ui.setupUi(this);
	ui.contactListTree->setFocusProxy(this);
	eventObject = new eventEater(this);
	qApp->installEventFilter(eventObject);
	letMeQuit = false;
	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	setFocus(Qt::ActiveWindowFocusReason);
	createActions();
	if ( QSystemTrayIcon::isSystemTrayAvailable() )
	{
		createTrayIcon();
//		trayIcon->show();
		connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	             this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
	}
	else
	{
		trayIcon = NULL;
		trayMenu = NULL;
	}

	currentStatusMenu = NULL;
	unreadMessages = false;

//	settingsDialog = new qutimSettings(this);
//	connect ( settingsDialog, SIGNAL(generalSettingsChanged()),
//			this, SLOT(reloadGeneralSettings()) );
	if ( QSystemTrayIcon::isSystemTrayAvailable() )
          trayIcon->show();
        else
          trayIcon = NULL;
	
	if (showOffline)
		ui.showHideButton->setIcon(QIcon(":/icons/crystal_project/hideoffline.png"));
	else 
		ui.showHideButton->setIcon(QIcon(":/icons/crystal_project/shhideoffline.png"));
	
	ui.contactListTree->header()->hide(); // hide contact list header

//	ui.contactListTree->setAlternatingRowColors(true); 
	ui.contactListTree->setAllColumnsShowFocus(true);
	ui.contactListTree->header()->setResizeMode(QHeaderView::Stretch);
	ui.contactListTree->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	ui.contactListTree->header()->setResizeMode(2, QHeaderView::ResizeToContents);
//	ui.contactListTree->header()->resizeSection(0,16);
	ui.contactListTree->header()->setStretchLastSection(false);
	ui.contactListTree->hideColumn(3);

	loadMainSettings();

	createLoginDialog();

	if (!bShouldRun)
		return;

	createMainMenu();
	createAccountLine();

	
//	if ( QSystemTrayIcon::isSystemTrayAvailable() )
	if (trayIcon)
		updateTrayStatus();
	
	connect( ui.contactListTree, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
				this, SLOT(itemClicked(QTreeWidgetItem *, int)));
	connect( ui.contactListTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
					this, SLOT(itemDoubleClicked(QTreeWidgetItem *)));
	
//	ui.contactListTree->setSortingEnabled(true);
//	ui.contactListTree->sortByColumn( 1, Qt::AscendingOrder );
//	ui.contactListTree->resizeColumnToContents(0);
//	show();
	
	connect(timer, SIGNAL(timeout()), this, SLOT(hide()));
	QTimer::singleShot(60000, this, SLOT(checkEventChanging()));
	createContactList();
	if (trayIcon)
		updateTrayToolTip();
	aboutWindowOpen = false;

	// Connect idle detector to each oscar protocol instance
	foreach(icqAccount *account, icqList)
		connect(&fIdleDetector, SIGNAL(secondsIdle(int)), account->getProtocol(), SLOT(onSecondIdle(int)));
	
	// Activate idle detector
	fIdleDetector.start();
	
	QMutexLocker locker(&fInstanceGuard);
	fInstance = this;
	
	loadStyle();
}

qutIM::~qutIM()	
{
	// Stop idle detector
	fIdleDetector.stop();
}

qutIM *qutIM::getInstance()
{
	QMutexLocker locker(&fInstanceGuard);
	return fInstance;
}

void qutIM::createTrayIcon()
{
	trayMenu = new QMenu(this);
	trayMenu->addAction(settingsAction);
	trayMenu->addAction(switchUserAction);
	
#if !defined(Q_OS_MAC)
	// Mac OS has it's own quit trigger in Dock menu
	trayMenu->addSeparator();
	trayMenu->addAction(quitAction);
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
	trayIcon = new ExSysTrayIcon(this);
#else
	trayIcon = new QSystemTrayIcon(this);
#endif

	trayIcon->setIcon(QIcon(":/icons/qutim.png"));
	
	// It's very unlikely for Mac OS X application to have 
	// tray menu and reactions to the tray action at the same time.
	// So we decided to add tray menu to Dock
#if !defined(Q_OS_MAC)
	trayIcon->setContextMenu(trayMenu);
#else
	qt_mac_set_dock_menu(trayMenu);
#endif
}

void qutIM::resizeEvent(QResizeEvent * )
{
	if (fWindowStyle == CLBorderLessWindow)
	{
		QRegion visibleRegion(ui.contactListTree->geometry().left(), ui.contactListTree->geometry().top(),
								ui.contactListTree->geometry().width(), ui.contactListTree->geometry().height());
		setMask(visibleRegion);	
	}
}

void qutIM::closeEvent(QCloseEvent *event)
{
	hide();
	
	// I dunno why, but it works ok in such way, but accordingly to QT Docs, QEvent::spontaneous() should return true if event generated by system...
	if (event->spontaneous())
		event->ignore();
	else
	{
		appQuit();
	}
}

void qutIM::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch(reason)
	{
	case QSystemTrayIcon::Trigger:
		if (! unreadMessages )
		{
			if (this->isVisible())
				this->setVisible(false);
			else
			{
				this->setVisible(true);
				this->activateWindow();
			}
		}
		else
			readMessages();
		break;
	default:
		break;
	}
}

void qutIM::createActions()
{
	quitAction = new QAction(QIcon(":/icons/crystal_project/exit.png"), 
			tr("&Quit"), this);
	settingsAction = new QAction(QIcon(":/icons/crystal_project/settings.png"), 
			tr("&Settings..."), this);
	switchUserAction = new QAction(QIcon(":/icons/crystal_project/switch_user.png"),
			tr("Switch user"), this);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(appQuit()));
	connect(settingsAction, SIGNAL(triggered()), this, SLOT(qutimSettingsMenu()));
	connect(switchUserAction, SIGNAL(triggered()), this, SLOT(switchUser()));
}

void qutIM::appQuit()
{
	saveMainSettings(); // save all main settings on exit
//	letMeQuit = true;
	foreach(icqAccount *account, icqList)
		account->getProtocol()->getContactListClass()->appExiting();
	QCoreApplication::exit(0);
//	close();

}

void qutIM::saveMainSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	
	//window size and position saving
	//save if "save size and position" feature is enabled
	if ( settings.value("mainwindow/saveSizPos", false).toBool())
	{
		QDesktopWidget desktop;
		settings.beginGroup("mainwindow");
		// Save current contact list' screen
		settings.setValue("screenNum", desktop.screenNumber(this));
		// Save size
		settings.setValue("size", this->size());
		// Save position on the screen
		settings.setValue("position", this->pos());
		settings.endGroup();
	}
}

void qutIM::loadMainSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	
//	currentStyle = settings.value("style/path", ":/qss/default.qss").toString();
//	loadStyle(currentStyle);
	QDesktopWidget desktop;
	
	//load size and position if "save size and position" feature is enabled
	//int numScreens = desktop.numScreens();
	int mainWindowScreen = settings.value("mainwindow/screenNum", desktop.primaryScreen()).toInt();
	
	// Check wheather target screen is connected
	if (mainWindowScreen > desktop.numScreens())
	{
		mainWindowScreen = desktop.primaryScreen();
		settings.setValue("mainwindow/screenNum", QVariant(mainWindowScreen));
	}

	int screenX = desktop.availableGeometry(mainWindowScreen).x();
	int screenY = desktop.availableGeometry(mainWindowScreen).y();
	int screenHeight = desktop.availableGeometry(mainWindowScreen).height();

	/*for (int i = 0; i < desktop.numScreens(); i++)
	{
		QRect avGeom = desktop.availableGeometry(i);
		QRect scGeom = desktop.screenGeometry(i);
	}*/
	
	this->move(screenX, screenY);
	//this->mapTo(desktop.screen(mainWindowScreen), desktop.screen(mainWindowScreen)->pos());
	//QPoint offs = this->mapFromGlobal(desktop.screen(mainWindowScreen)->pos());
	QPoint offs = mapFromGlobal(pos());
	QPoint moveTo(desktop.availableGeometry(mainWindowScreen).right() - 150, screenY);
	this->resize(150, screenHeight);

	if (settings.value("mainwindow/saveSizPos", true).toBool())
	{
		QSize sze = settings.value("mainwindow/size", QSize(0,0)).toSize();
		moveTo = settings.value("mainwindow/position", moveTo).toPoint();
	
		if (sze.width() >= sizeHint().width() || sze.height() >= sizeHint().height())
			this->resize(sze);
	}
	
	if (moveTo.x() + frameGeometry().width() > desktop.availableGeometry(mainWindowScreen).right())
		moveTo.setX(desktop.availableGeometry(mainWindowScreen).right() - frameGeometry().width() + 1);
	if (moveTo.x() < desktop.availableGeometry(mainWindowScreen).left())
		moveTo.setX(desktop.availableGeometry(mainWindowScreen).left());
	if (moveTo.y() + frameGeometry().height() > desktop.availableGeometry(mainWindowScreen).bottom())
		moveTo.setY(desktop.availableGeometry(mainWindowScreen).bottom() - 2*frameGeometry().height()
			+ height());
	if (moveTo.y() < desktop.availableGeometry(mainWindowScreen).top())
		moveTo.setY(desktop.availableGeometry(mainWindowScreen).top());
	if (frameGeometry().height() > desktop.availableGeometry(mainWindowScreen).bottom())
	{
		resize(width(), desktop.availableGeometry(mainWindowScreen).bottom() + height() - frameGeometry().height() + 1);
	}
	
	moveTo += offs;
	// Rellocating contact list' window
	this->move(moveTo);
	
//	if( settings.value("mainwindow/hideStart", false).toBool() )
//		hide();
//	else show();
	
	createMenuAccounts = settings.value("mainwindow/accountMenu", true).toBool();
	addToStatusTrayMenu  = false;
	if (settings.value("systray/inmenu", false).toBool() && trayIcon)
		updateCurrentTrayStatusmenu(true);

	// Reload main window opacity
	int nOpacity = settings.value("mainwindow/opacity", 100).toInt();
	setWindowOpacity(static_cast<double>(nOpacity) / 100);
	
	hideSeparators = settings.value("contactlist/hidesep", false).toBool();
	showOffline = settings.value("contactlist/offline", false).toBool();
	hideEmptyGroups = settings.value("contactlist/hideempty", false).toBool();
	showGroups = settings.value("contactlist/groups", false).toBool();

	mergeAccounts = settings.value("contactlist/merge", false).toBool();
	if (mergeAccounts)
//		 createGroups();
		reupdateList();	

	sorting = settings.value("contactlist/sort", 3).toInt();
	ui.contactListTree->model()->setHeaderData(0,Qt::Horizontal,QVariant(sorting));
	if ( !settings.value("contactlist/clicons",false).toBool())
		ui.contactListTree->hideColumn(2);
		
	reloadCLWindowStyle(settings);
		
	autoHide = settings.value("contactlist/ahide",false).toBool();
	hideSec = settings.value("contactlist/hidesec",60).toInt();
	ui.contactListTree->setAlternatingRowColors(settings.value("contactlist/colorrow",true).toBool()); 
}

void qutIM::qutimSettingsMenu()
{
	settingsAction->setDisabled(true);
	switchUserAction->setDisabled(true);
	ui.showHideButton->setEnabled(false);
	if (currentStatusMenu)
		currentStatusMenu->setEnabled(false);
	settingsDialog = new qutimSettings(this);
	settingsDialog->setIcqList(&icqList);
	connect ( settingsDialog, SIGNAL(generalSettingsChanged()),
			this, SLOT(reloadGeneralSettings()) );
	connect ( settingsDialog, SIGNAL(destroyed()),
				this, SLOT(destroySettings()) );
	connect(this, SIGNAL(updateTranslation()),
		settingsDialog, SLOT(onUpdateTranslation()));
	
	settingsDialog->loadAllSettings(); //load all settings to "Settings" window
	settingsDialog->applyDisable(); //disable "Settings" window's Apply button 
	rellocateSettingsDialog(settingsDialog);
	settingsDialog->show();
	settingsDialog->setAttribute(Qt::WA_QuitOnClose, false); //don't close app on "Settings" exit
	settingsDialog->setAttribute(Qt::WA_DeleteOnClose, true);
}

void qutIM::destroySettings()
{
	settingsAction->setDisabled(false);
	switchUserAction->setDisabled(false);
	ui.showHideButton->setEnabled(true);
	if ( currentStatusMenu )
		currentStatusMenu->setDisabled(false);
}

void qutIM::rellocateSettingsDialog(qutimSettings *settings)
{
	QDesktopWidget desktop;
	// Get current screen num
	int curScreen = desktop.screenNumber(this);
	// Get available geometry of the screen
	QRect screenGeom = desktop.availableGeometry(curScreen);
	// Let's calculate point to move dialog
	QPoint moveTo(screenGeom.left(), screenGeom.top());
	
	moveTo.setX(moveTo.x() + screenGeom.width() / 2);
	moveTo.setY(moveTo.y() + screenGeom.height() / 2);
	
	moveTo.setX(moveTo.x() - settings->size().width() / 2);
	moveTo.setY(moveTo.y() - settings->size().height() / 2);
	
	settings->move(moveTo);
}

void qutIM::createContactList()
{
	foreach(icqAccount *account, icqList)		
	if ( account->getAutoConnect() )
				account->autoconnecting();	

}

void qutIM::createLoginDialog()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	
	bShouldRun = true;
	if(settings.value("logindialog/showstart", true).toBool())
	{
		loginDialog loginD(this);
		loginD.loadSettings(); //load all login settings

		// Show always login dialog on primary screen
		QDesktopWidget desktop;
		
		// In the case of virtual desktop on multiple monitors
		if (desktop.isVirtualDesktop())
		{
			QPoint moveTo(desktop.availableGeometry(desktop.primaryScreen()).left(), desktop.availableGeometry(desktop.primaryScreen()).top());
			moveTo.setX(moveTo.x() + desktop.availableGeometry(desktop.primaryScreen()).width() / 2);
			moveTo.setY(moveTo.y() + desktop.availableGeometry(desktop.primaryScreen()).height() / 2);

			moveTo.setX(moveTo.x() - loginD.size().width() / 2);
			moveTo.setY(moveTo.y() - loginD.size().height() / 2);

			// Rellocate dialog
			loginD.move(moveTo);
		}

		int logDlgExecuted = loginD.exec();
		if( logDlgExecuted )
			loginD.saveSettings(); //save all on "sign in"
		else
		{
			// It seems ESC was pressed
			bShouldRun = false;
			QApplication::quit();
			return ;
		}
	}

	if( settings.value("mainwindow/hideStart", false).toBool() )
		hide();
	else 
	{
#if defined(Q_OS_WIN32)
		DWORD wndStyle = ::GetWindowLong(this->winId(), GWL_EXSTYLE);
		::SetWindowLong(this->winId(), GWL_EXSTYLE, wndStyle | WS_EX_TOOLWINDOW);
#endif	
		show();
	}
//	loadMainSettings();
}

void qutIM::createAccountLine()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "accounts");
	QStringList accountList = settings.value("accounts/list").toStringList();
	int index = 2, accountLineWidth = 0;
	foreach( QString accountFromList, accountList)
	{
		icqAccount *account = new icqAccount(trayIcon, ui.contactListTree, accountFromList, this);
		
		connect(this, SIGNAL(updateTranslation()),
			account, SLOT(onUpdateTranslation()));

		if (trayIcon)
		connect(account, SIGNAL(statusChanged(const QIcon &)),
				this, SLOT(updateTrayIcon(const QIcon &)));
		
		account->createAccountButton(ui.gridLayout1/*ui.accountBtLayout*/, ui.accountLine, index);
		icqList.append(account);
		
		if (trayMenu)
		{
		if( createMenuAccounts )
			account->createMenuAccount(trayMenu, settingsAction); //add account status to tray menu
		
		if ( addToStatusTrayMenu )
		{
				account->createTrayMenuStatus(currentStatusMenu);
				connect(account, SIGNAL(changeStatusInTrayMenu(const QString &)),
						this, SLOT(setStatusFrom(const QString &)));
				
		}
		}
		
		
		connect(account, SIGNAL(getNewMessage()),
				this, SLOT(getNewMessage()));
//UNCOMMENT		
		if ( mergeAccounts )
		{
					account->getProtocol()->getContactListClass()->mergeAccounts(true);
//					account->getProtocol()->getContactListClass()->hideRoot(true);
		}
//UNCOMMENT	
		
//		if ( account->getAutoConnect() )
//			account->autoconnecting();
		
		
		accountLineWidth += 23;
		++index;
		
		account->getProtocol()->getContactListClass()->initializaMenus(addFindUsers, serviceMessages ,sendMultiple,
				privacyList, changeMyDetails, changeMyPassword);
	}
	
	if(createMenuAccounts && trayMenu)
		trayMenu->insertSeparator(settingsAction);
	
	if( accountLineWidth > 100)setMinimumSize(QSize(accountLineWidth, 100));
	
		
}

void qutIM::reloadGeneralSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	
	// Reload current qss style
	QString cStyle = settings.value("style/path", ":/qss/default.qss").toString();
	
	if ( cStyle != currentStyle)
	{
		loadStyle();
	}
	
	// Reload translation
	int currTranslation = settings.value("style/tr", 0).toInt();
	if ( currentTranslation != currTranslation)
	{
		currentTranslation = currTranslation;
		loadTranslation(currentTranslation);
	}
	
	// Reload icon theme
	statusIconClass::getInstance()->reloadIconsFromSettings();

	bool tmpAccountMenu = settings.value("mainwindow/accountMenu", true).toBool();
	if ( createMenuAccounts != tmpAccountMenu)
	{
		if (trayMenu)
		{
		foreach( icqAccount *account, icqList )
		{
			if ( tmpAccountMenu)
				account->createMenuAccount(trayMenu, settingsAction);
			else
				account->removeMenuAccount(trayMenu);
		}
		if ( tmpAccountMenu )
			trayMenu->insertSeparator(settingsAction);
		}
		createMenuAccounts = tmpAccountMenu;
	}

	// Notify each icqAccount about reloaded settings
	foreach( icqAccount *account, icqList )
	{
		account->onReloadGeneralSettings();
	}

	// Update current trayIcon
	if (trayIcon)
	{
		updateTrayStatus();	
		updateCurrentTrayStatusmenu(settings.value("systray/inmenu", false).toBool());
	}

	// Enable status metu
	if (currentStatusMenu)
		currentStatusMenu->setEnabled(false);
	
	// Reload main window opacity
	int nOpacity = settings.value("mainwindow/opacity", 100).toInt();
	setWindowOpacity(static_cast<double>(nOpacity) / 100);

	// Reload contact list look'n'feel
	bool hideSep = settings.value("contactlist/hidesep", false).toBool();
	bool hdEmp = settings.value("contactlist/hideempty", false).toBool(); 
	bool mrgAcc = settings.value("contactlist/merge", false).toBool();	
	bool shGrp = settings.value("contactlist/groups", false).toBool();
	bool shOff = settings.value("contactlist/offline", false).toBool();
	if ( mergeAccounts != mrgAcc)
	{
		if ( mrgAcc )
		{
			hideEmptyGroups = hdEmp;
			showGroups = shGrp;
			showOffline = shOff;
			hideSeparators = hideSep;
			mergeAccountsToList(mrgAcc);
		} else 
		{
			mergeAccountsToList(mrgAcc);
			hideEmptyGroups = hdEmp;
			showGroups = shGrp;
			showOffline = shOff;
			hideSeparators = hideSep;
		}
	}
	else 
	{
		
		if ( hideSeparators != hideSep )
		{
					hideSeparators = hideSep;
					setHideSeparators(hideSeparators);
		}
		
		if ( showGroups != shGrp )
		{
			showHideGroups(shGrp);
		}
		if ( hideEmptyGroups != hdEmp)
			hideEmptyGr(hdEmp);
		
		if ( showOffline != shOff)
			hideOffline(shOff);
	}
	
	quint8 tmpSort = settings.value("contactlist/sort", 3).toInt();
	if ( tmpSort != sorting)
	{
		sorting = tmpSort;
		ui.contactListTree->model()->setHeaderData(0,Qt::Horizontal,QVariant(sorting));
		updateSorting();
	}
	ui.contactListTree->setColumnHidden(2, !settings.value("contactlist/clicons", false).toBool());

	reloadCLWindowStyle(settings);
	
#if defined(Q_OS_WIN32)
	// Hide contact list' window from taskbar
	// TO-DO: Investigate QT for this purpose
	DWORD wndStyle = ::GetWindowLong(this->winId(), GWL_EXSTYLE);
	::SetWindowLong(this->winId(), GWL_EXSTYLE, wndStyle | WS_EX_TOOLWINDOW);
#endif

	show();
	
	autoHide = settings.value("contactlist/ahide",false).toBool();
	hideSec = settings.value("contactlist/hidesec",60).toInt();
	if ( !autoHide )
		timer->stop();
	ui.contactListTree->setAlternatingRowColors(settings.value("contactlist/colorrow",true).toBool()); 

	///===///
}

void qutIM::addAccount(const QString &account)
{
	icqAccount *newaccount = new icqAccount(trayIcon, ui.contactListTree, account, this);
	if (trayIcon)
	connect(newaccount, SIGNAL(statusChanged(const QIcon &)),
			this, SLOT(updateTrayIcon(const QIcon &)));
	icqList.append(newaccount);
	 newaccount->createAccountButton(ui.gridLayout1/*ui.accountBtLayout*/, ui.accountLine, icqList.size() + 2);
	if( icqList.size() * 23 > 100 )
		setMinimumSize(QSize(icqList.size()*23 + 23, 100));
	if (createMenuAccounts && trayMenu)
	{
		if( icqList.count() - 1 )
			newaccount->createMenuAccount(trayMenu, icqList.at(0)->getAction());
		else
		{
			newaccount->createMenuAccount(trayMenu, settingsAction);
			trayMenu->insertSeparator(settingsAction);
		}
	}
	if (addToStatusTrayMenu && trayMenu)
	{
				newaccount->createTrayMenuStatus(currentStatusMenu);
				connect(newaccount, SIGNAL(changeStatusInTrayMenu(const QString &)),
										this, SLOT(setStatusFrom(const QString &)));
	}
	
	connect(newaccount, SIGNAL(getNewMessage()),
				this, SLOT(getNewMessage()));
	
	
	if ( mergeAccounts )
				newaccount->getProtocol()->getContactListClass()->mergeAccounts(true);
	if ( newaccount->getAutoConnect() )
				newaccount->autoconnecting();
	
	newaccount->getProtocol()->getContactListClass()->initializaMenus(addFindUsers, serviceMessages, 
			sendMultiple, privacyList, changeMyDetails, changeMyPassword);
	
	
}

void qutIM::removeAccount(const QString &account)
{
	foreach( icqAccount *tmpAccount, icqList)
	{
		if ( account == tmpAccount->getIcquin() )
		{
			if (trayMenu)
			{
			if ( createMenuAccounts )
				tmpAccount->removeMenuAccount(trayMenu);
			
			if ( addToStatusTrayMenu )
			{
						tmpAccount->removeTrayMenuStatus(currentStatusMenu);
					disconnect(tmpAccount, 0, this, 0);
				}
			}
			
			tmpAccount->removeAccountButton();
			tmpAccount->removeContactList();
			tmpAccount->deleteingAccount = true;
			icqList.removeAt(icqList.indexOf(tmpAccount));
			disconnect(tmpAccount, 0,
					this, 0);
			if ( mergeAccounts )	
			foreach( treeBuddyItem *buddy,
					tmpAccount->getProtocol()->getContactListClass()->getBuddyList())
				{
					if ( buddy->isOffline )
						offlineList->removeChild(buddy);
					else
						onlineList->removeChild(buddy);
						
						delete buddy;
				}
			
			
			delete tmpAccount;
			if (trayIcon)
				updateTrayStatus();
			break;
		}
	}
//	if ( mergeAccounts )
//		reupdateList();
}

void qutIM::switchUser()
{
	loginDialog loginD(this);
	loginD.setRunType(true);
	connect(&loginD, SIGNAL(addingAccount(const QString &)),
			this, SLOT(addAccount(const QString &)));
	connect(&loginD, SIGNAL(removingAccount(const QString &)),
				this, SLOT(removeAccount(const QString &)));
	loginD.loadSettings(); //load all login settings
	if( loginD.exec() )
		loginD.saveSettings(); //save all on "sign in"
	else
		// It seems ESC was pressed
		QApplication::quit();
}

void qutIM::updateTrayStatus()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	switch ( settings.value("systray/show", 1).toInt() )
		{
		case 0:
			if (trayIcon)
				trayIcon->setIcon(QIcon(":/icons/qutim.png"));
			tempIcon = QIcon(":/icons/qutim.png");
//			addToStatusTrayMenu = false;
			if (trayIcon)
				updateCurrentTrayStatusmenu(false);
			break;
		case 1:
			setStatusFrom(settings.value("systray/current", "").toString());
			break;
		default:
			break;
		}
}

void qutIM::setStatusFrom(const QString &icqUin)
{
	
	bool accountInList = false;
	if ( icqUin != "")
	{		
		foreach( icqAccount *account, icqList )
		{
			account->setTrayCurrent(false);
			account->setChooseStatusCheck(false);
			if ( account->getIcquin() == icqUin)
			{
				account->setTrayCurrent(true);
				account->setChooseStatusCheck(true);
				if (trayIcon)
					trayIcon->setIcon(account->getCurrentIcon());
				tempIcon = account->getCurrentIcon();
				accountInList = true;
			}
		}
	}
	else if (!icqList.empty())
	{
		icqList.at(0)->setTrayCurrent(true);
		if (trayIcon)
			trayIcon->setIcon(icqList.at(0)->getCurrentIcon());
		tempIcon = icqList.at(0)->getCurrentIcon();
	}
	
	if ( accountInList )
	{
		icqList.at(0)->setTrayCurrent(true);
		if (trayIcon)
			trayIcon->setIcon(icqList.at(0)->getCurrentIcon());	
		tempIcon = icqList.at(0)->getCurrentIcon();
	}
}

void qutIM::updateTrayIcon(const QIcon &statusIcon)
{
	if (trayIcon)
		trayIcon->setIcon(statusIcon);
	tempIcon = statusIcon;
}

void qutIM::updateCurrentTrayStatusmenu(bool show)
{
	if (!trayMenu) return;
	if ( show )
	{
		if ( !addToStatusTrayMenu)
		{
			currentStatusMenu = new QMenu(this);
			currentStatusMenu->setTitle(tr("Tray status from:"));
			currentStatusMenu->setIcon(QIcon(":/icons/crystal_project/status.png"));
			trayMenu->insertMenu(switchUserAction, currentStatusMenu);
			addToStatusTrayMenu = true;
			foreach( icqAccount *account, icqList)
			{
				account->createTrayMenuStatus(currentStatusMenu);
				connect(account, SIGNAL(changeStatusInTrayMenu(const QString &)),
										this, SLOT(setStatusFrom(const QString &)));
			}
		}
	} else {
		
		if (addToStatusTrayMenu && trayMenu)
		{
			foreach( icqAccount *account, icqList)
			{
				account->removeTrayMenuStatus(currentStatusMenu);
				disconnect(account, SIGNAL(changeStatusInTrayMenu(const QString &)),
										this, SLOT(setStatusFrom(const QString &)));
			}
			delete currentStatusMenu;
			currentStatusMenu = NULL;
		}
		addToStatusTrayMenu = false;
	}
	
}

void qutIM::itemClicked(QTreeWidgetItem *item, int column)
{
	if ( item->type() == 1  && column == 0)
	{
		treeGroupItem *group = dynamic_cast<treeGroupItem *>(item);
			group->groupClicked();
	} else if ( item->type() == 0 )
	{
		if (item->childCount())
		{
				item->setExpanded(!item->isExpanded());
				item->sortChildren(1, Qt::AscendingOrder);
		}
	}
	
}

void qutIM::itemDoubleClicked(QTreeWidgetItem *item)
{
	if ( item->type() == 1)
	{
		treeGroupItem *group = dynamic_cast<treeGroupItem *>(item);
		group->groupClicked();
	}
	
	if ( item->type() == 2 )
	{
			treeBuddyItem *buddy = dynamic_cast<treeBuddyItem *>(item);
			buddy->par->doubleClickedBuddy(buddy);
	} //else if (item->type() == 3)
	//{
		
	//}
}

void qutIM::getNewMessage()
{
	if (trayIcon)
	{
	if ( !unreadMessages )
	{
		unreadMessages = true;
		msgIcon = true;
		updateMessageIcon();
	}
	icqAccount *tmpAccount = qobject_cast<icqAccount *>(sender());
	if ( !messageStack.contains(tmpAccount) )
		messageStack.append(tmpAccount);
	}
}

void qutIM::updateMessageIcon()
{
	// Check wheather we have unread messaged
	if ( unreadMessages )
	{
		// And tray icon is enabled
		if (trayIcon)
		{
			// We want to make some blink effect, so change one icon with other
			if ( msgIcon )
				trayIcon->setIcon(statusIconClass::getInstance()->getContentIcon());
			else 
				trayIcon->setIcon(QIcon());
			//trayIcon->setIcon(tempIcon);
		}
		msgIcon = !msgIcon;
		QTimer::singleShot(500,this, SLOT(updateMessageIcon()));
	}
	else if (trayIcon)
		trayIcon->setIcon(tempIcon);
}

void qutIM::readMessages()
{
	unreadMessages = false;
	updateMessageIcon();
	foreach( icqAccount *tmpAccount, messageStack)
		tmpAccount->readMessageStack();
	messageStack.clear();
}

void qutIM::reloadCLWindowStyle(const QSettings &settings)
{
	Qt::WindowFlags  flags = 0;
	
	if ( settings.value("contactlist/ontop", false).toBool())
		flags |= Qt::WindowStaysOnTopHint;
	else
		flags &= ~Qt::WindowStaysOnTopHint;
		
	fWindowStyle = static_cast<CLWindowStyle>(settings.value("contactlist/windowstyle", CLRegularWindow).toInt());	
	if (fWindowStyle == CLRegularWindow)
	{
		clearMask();
	}
	else if (fWindowStyle == CLBorderLessWindow)
	{
		// Strange behaviour happens on Mac OS X, so we will disable this flag for a while
#if !defined(Q_OS_MAC)
		flags |= Qt::FramelessWindowHint;
#endif
		
		/*ui.menuButton->setVisible(false);
		ui.showHideButton->setVisible(false);
		ui.infoButton->setVisible(false);
		ui.toolButtonsLayout->setEnabled(false);
		ui.toolButtonsLayout->invalidate();*/
	}
		
	setWindowFlags(flags);
	
	if (fWindowStyle == CLBorderLessWindow)
	{
		//ui.contactListTree->updateGeometry();
		QRegion visibleRegion(ui.contactListTree->geometry().left(), ui.contactListTree->geometry().top(),
								ui.contactListTree->geometry().width(), ui.contactListTree->geometry().height());
		setMask(visibleRegion);	
	}
}

void qutIM::accountReadAllMessages()
{
	messageStack.removeAll((icqAccount *)sender());
	unreadMessages = false;
}

void qutIM::mergeAccountsToList(bool flag)
{
	mergeAccounts = flag;
	if ( flag )
	{
//		ui.contactListTree->clear();
		createGroups();	
		foreach( icqAccount *account, icqList )
		{

			getGroupList(account->getProtocol()->getContactListClass()->mergeAccounts(flag));
		}

		setHideSeparators(hideSeparators);
		updateSorting();
		
	} else {
		removeGroups();
	}
}

void qutIM::createGroups()
{
	if ( !showGroups )
	{
		onlineList = new contactSeparator(ui.contactListTree);
		onlineList->setText(1,tr("Online"));
		onlineList->setTextAlignment(1, Qt::AlignCenter);
		offlineList = new contactSeparator(ui.contactListTree);
		offlineList->setText(1,tr("Offline"));
		offlineList->setTextAlignment(1, Qt::AlignCenter);
		offlineList->setFont(1, QFont(offlineList->font(1).family(), 7));
//		QLinearGradient linearGrad(0,0,0,16);
//		linearGrad.setColorAt(0, QColor(255,255,255));
//		linearGrad.setColorAt(0.5, QColor(248,248,248));
//		linearGrad.setColorAt(1, QColor(255,255,255));
//		QBrush brush(linearGrad);
//		offlineList->setBackground(1, brush);
//		offlineList->setBackground(0, brush);
//		offlineList->setBackground(2, brush);
//		onlineList->setBackground(0, brush);
//		onlineList->setBackground(1, brush);
//		onlineList->setBackground(2, brush);
		onlineList->setFont(1, QFont(offlineList->font(1).family(), 7));
		onlineList->setHidden(true);
		offlineList->setHidden(true);
	} 
}

void qutIM::removeGroups()
{
	if (!showGroups)
	{
		if ( offlineList )
		{
			offlineList->takeChildren();
			delete offlineList;
		}
		
		if ( onlineList )
		{
			onlineList->takeChildren();
			delete onlineList;
		}
	}
	else
	{
		foreach( treeGroupItem *group, mergeGroupList)
		{
			group->onlineList->takeChildren();
			group->offlineList->takeChildren();
			delete group;
		}
		mergeGroupList.clear();
	}
	foreach( icqAccount *account, icqList )
	{
		account->getProtocol()->getContactListClass()->mergeAccounts(false);
	}
}

void qutIM::getGroupList(QHash<quint16, treeGroupItem *> groupList)
{
	
	if ( !showGroups )
	{
		foreach(treeGroupItem *group, groupList)
		{
			offlineList->addChildren(group->offlineList->takeChildren());
			onlineList->addChildren(group->onlineList->takeChildren());
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
	} else {
		foreach(treeGroupItem *group, groupList)
		{
			if ( !mergeGroupList.contains(group->name) )
			{
				treeGroupItem *newGroup = new treeGroupItem(ui.contactListTree);
				newGroup->setGroupText(group->name);
				newGroup->setOnOffLists();
				newGroup->userCount = 1;
				newGroup->setExpanded(true);
//				newGroup->userCount = 0;
				newGroup->onlineList->setHidden(true);
				newGroup->offlineList->setHidden(true);
				mergeGroupList.insert(newGroup->name, newGroup);
			} else {
				mergeGroupList.value(group->name)->userCount += group->userCount;
			}
			
			treeGroupItem *newGroup2 = mergeGroupList.value(group->name);
			newGroup2->onlineList->addChildren(group->onlineList->takeChildren());
			newGroup2->offlineList->addChildren(group->offlineList->takeChildren());
			newGroup2->userCount = newGroup2->onlineList->childCount() + newGroup2->offlineList->childCount();
//			newGroup2->userOnline += group->userOnline;
			newGroup2->userOnline = newGroup2->onlineList->childCount();
			newGroup2->updateText();
			if (newGroup2->userCount > 0)
			{
				if ( newGroup2->onlineList->childCount() )
				{
					newGroup2->onlineList->setHidden(false);
					newGroup2->onlineList->setExpanded(true);
				}
				if ( newGroup2->offlineList->childCount() )
				{
					if ( !showOffline )
						newGroup2->offlineList->setHidden(false);	
						newGroup2->offlineList->setExpanded(true);
				}
//				newGroup2->setExpanded(true);
				newGroup2->setHidden(false);
			}
			newGroup2->onlineList->sortChildren(1,Qt::AscendingOrder);
			if ( !showOffline )
				newGroup2->offlineList->sortChildren(1,Qt::AscendingOrder);
		}
		hideEmptyGr(hideEmptyGroups);
			
	}
	setHideSeparators(hideSeparators);
	updateSorting();

}

void qutIM::buddyChangeStatus(treeBuddyItem *buddy, bool isOffline)
{
	if ( !showGroups )
	{
		if ( isOffline )
		{
			onlineList->removeChild(buddy);
			offlineList->addChild(buddy);
			offlineList->sortChildren(1,Qt::AscendingOrder);
		} else {
			offlineList->removeChild(buddy);
			onlineList->addChild(buddy);
			
		}
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
	} else {
		treeGroupItem *group = mergeGroupList.value(buddy->groupName);
		if ( isOffline )
		{
			group->onlineList->removeChild(buddy);
			group->offlineList->addChild(buddy);
			group->buddyOffline();
		} else {
			group->offlineList->removeChild(buddy);
			group->onlineList->addChild(buddy);
			group->buddyOnline();
			
		}
		if ( !group->onlineList->childCount() )
		{
			group->onlineList->setHidden(true);
			if ( showOffline && hideEmptyGroups)
				group->setHidden(true);
		}
		else
		{
			group->onlineList->setHidden(false);
			group->onlineList->setExpanded(true);
			if ( showOffline && hideEmptyGroups)
				group->setHidden(false);
		}
		
		if ( !group->offlineList->childCount() )
		{
			group->offlineList->setHidden(true);

		}
		else
		{
			if ( !showOffline )
				group->offlineList->setHidden(false);
			group->offlineList->setExpanded(true);
		}
		
		
	}
}

void qutIM::showHideGroups(bool flag)
{
	showGroups = flag;
	if ( flag )
	{
		if (offlineList)
		{
			offlineList->takeChildren();
			delete offlineList;
		}
		if (onlineList)
		{
				onlineList->takeChildren();
				delete onlineList;
		}

	} else {
	foreach( treeGroupItem *group, mergeGroupList)
	{
		group->onlineList->takeChildren();
		group->offlineList->takeChildren();
		delete group;
	}
		mergeGroupList.clear();
		createGroups();
	}
	
	foreach( icqAccount *account, icqList )
	{
		account->getProtocol()->getContactListClass()->mergeAccounts(false);
		account->getProtocol()->getContactListClass()->mergeAccounts(true);
		getGroupList(account->getProtocol()->getContactListClass()->groupList);
				
	}
	setHideSeparators(hideSeparators);
	updateSorting();
}

void qutIM::hideEmptyGr(bool flag)
{
		hideEmptyGroups = flag;
		foreach(treeGroupItem *group, mergeGroupList)
		{
			if ( !group->userCount ) 
				group->setHidden(flag);
                else if (!group->onlineList->childCount() && showOffline)
				group->setHidden(flag);
			}
}

void qutIM::updateSorting()
{
	if ( mergeAccounts )
	{
		setUpdatesEnabled(false);
		if ( showGroups)
		{
			foreach( treeGroupItem *group, mergeGroupList)
			{
				group->onlineList->sortChildren(1,Qt::AscendingOrder);
				group->offlineList->sortChildren(1,Qt::AscendingOrder);
			}
                }
                else
                {
			onlineList->sortChildren(1,Qt::AscendingOrder);
			offlineList->sortChildren(1,Qt::AscendingOrder);
		}
		setUpdatesEnabled(true);
        }
        else
        {
	foreach(icqAccount *account, icqList)
	{
		account->getProtocol()->getContactListClass()->updateSorting();
	}
	}
}

void qutIM::hideOffline(bool flag)
{
	showOffline = flag;
	if ( showGroups )
	{
		foreach( treeGroupItem *group, mergeGroupList)
		{
			
			if ( group->offlineList->childCount() )
			{
				group->offlineList->setHidden(flag);
				group->setHidden(false);
				if ( hideEmptyGroups && showOffline && !group->onlineList->childCount())
					group->setHidden(true);
			}else if (hideEmptyGroups && !group->onlineList->childCount() )
				group->setHidden(true);
			group->offlineList->sortChildren(1, Qt::AscendingOrder);
		}
	} else {
		
		offlineList->sortChildren(1, Qt::AscendingOrder);
		if ( offlineList->childCount() )
			offlineList->setHidden(flag);
	}
}


void qutIM::focusOutEvent ( QFocusEvent */*event*/ )
{
	if ( autoHide )
		timer->start(hideSec * 1000);

}

void qutIM::focusInEvent(QFocusEvent */*event*/)
{
	timer->stop();

}

void qutIM::reupdateList()
{
	removeGroups();
	createGroups();	
	foreach( icqAccount *account, icqList )
	{

		getGroupList(account->getProtocol()->getContactListClass()->mergeAccounts(true));
		
	}
	setHideSeparators(hideSeparators);
	updateSorting();
	
}

void qutIM::checkEventChanging()
{
	if ( eventObject->getChanged() )
	{
		eventObject->setChanged(false);
	} 
	QTimer::singleShot(60000, this, SLOT(checkEventChanging()));
}

void qutIM::addToEventList(bool add)
{
	icqAccount *account = (icqAccount*)sender();
	if ( add )
		updateEventList.append(account);
	else
		updateEventList.removeAll(account);
}

void qutIM::updateTrayToolTip()
{
	if (!trayIcon) return;
	QString toolTip;
	foreach(icqAccount *account, icqList)
	{
		toolTip.append(tr("<img src='%1'>  %2<br>").arg(account->currentIconPath).arg(account->getIcquin()));
	}
	toolTip.chop(4);
	trayIcon->setToolTip(toolTip);
}

void qutIM::setHideSeparators(bool hide)
{
	ui.contactListTree->setUpdatesEnabled(false);
	if ( !showGroups )
	{
		onlineList->hideSeparator(hide);
		
		offlineList->hideSeparator(hide);
		onlineList->sortChildren(1, Qt::AscendingOrder);
		offlineList->sortChildren(1, Qt::AscendingOrder);
		
	} else {
		foreach(treeGroupItem *group, mergeGroupList)
		{
			
			group->hideSeparators(hide);	
		}
	}

	ui.contactListTree->setUpdatesEnabled(true);
}

void qutIM::createMainMenu()
{
	addFindUsers  = new QMenu(tr("&Add/find users"));
	addFindUsers->setIcon(QIcon(":/icons/crystal_project/search.png"));
	
	sendMultiple = new QMenu(tr("Send multiple"));
	sendMultiple->setIcon(QIcon(":/icons/crystal_project/multiple.png"));
	
	privacyList = new QMenu(tr("Privacy lists"));
	privacyList->setIcon(QIcon(":/icons/crystal_project/privacylist.png"));
		
	serviceMessages = new QMenu(tr("Service messages"));
	serviceMessages->setIcon(QIcon(":/icons/crystal_project/servicemessage.png"));
	
	changeMyDetails = new QMenu(tr("View/change my details"));
	changeMyDetails->setIcon(QIcon(":/icons/crystal_project/changedetails.png"));
		
	changeMyPassword = new QMenu(tr("Change my password"));
	changeMyPassword->setIcon(QIcon(":/icons/crystal_project/password.png"));
	
	mainMenu = new QMenu(this);
	mainMenu->addMenu(addFindUsers);
	mainMenu->addMenu(sendMultiple);
	mainMenu->addMenu(privacyList);
	mainMenu->addMenu(serviceMessages);
	mainMenu->addMenu(changeMyDetails);
	mainMenu->addMenu(changeMyPassword);
	mainMenu->addAction(settingsAction);
	mainMenu->addAction(switchUserAction);
	mainMenu->addSeparator();
	mainMenu->addAction(quitAction);
	ui.menuButton->setMenu(mainMenu);
}


void qutIM::on_contactListTree_customContextMenuRequested(const QPoint &point)
{
	QTreeWidgetItem *clickedItemForContext = 0;
		
	clickedItemForContext = ui.contactListTree->itemAt(point);
		
	if ( clickedItemForContext )
	{
			if ( clickedItemForContext->type() == 1 )
			{
				treeGroupItem *group = dynamic_cast<treeGroupItem *>(clickedItemForContext);
				
				if ( group->fromItem)
					group->par->showGroupMenu(group);
			}
			if ( clickedItemForContext->type() == 2)
			{
				treeBuddyItem *buddy = dynamic_cast<treeBuddyItem *>(clickedItemForContext);
				buddy->par->showBuddyMenu(buddy);
			}
	}
}

void qutIM::loadStyle()
{
	//Disable theme engine for Mac OS X
#if !defined(Q_OS_MAC)
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	
	currentStyle = settings.value("style/path", ":/qss/default.qss").toString();
	QDir styleDir(currentStyle);
	QStringList filters;
	filters << "*.qss";
	QStringList styleList = styleDir.entryList(filters);
	if ( styleList.count())
	{
		QFile file(currentStyle + "/" +styleList.at(0));
	    if(file.open(QFile::ReadOnly))
	    {
	    	qApp->setStyleSheet("");
		    QString styleSheet = QLatin1String(file.readAll());
		    qApp->setStyleSheet(styleSheet.replace("%path%", currentStyle));
		    file.close();
	    }
	} else 
		qApp->setStyleSheet("");
#endif
}

void qutIM::on_infoButton_clicked()
{
	if ( !aboutWindowOpen )
	{
		aboutWindowOpen = true;
		infoWindow = new aboutInfo;
		connect(infoWindow, SIGNAL(destroyed ( QObject * )),
				this, SLOT(infoWindowDestroyed(QObject *)));
		infoWindow->show();
	}
}

void qutIM::infoWindowDestroyed(QObject *)
{
	aboutWindowOpen = false;

}

void qutIM::loadTranslation(int index)
{
	qApp->removeTranslator(&applicationTranslator);
	switch(index)
	{
//	case 0:
//		applicationTranslator.load("");
//		break;
	case 1:
		applicationTranslator.load(":/translation/qutim_lt.qm");
		break;
	case 2:
		applicationTranslator.load(":/translation/qutim_ru.qm");
		break;
	case 3:
		applicationTranslator.load(":/translation/qutim_ua.qm");
		break;	
	default:
		applicationTranslator.load("");
	}
	qApp->installTranslator(&applicationTranslator);

	if (quitAction)
	{
		if (trayIcon) updateTrayToolTip();

		addFindUsers->setTitle(tr("&Add/find users"));
		sendMultiple->setTitle(tr("Send multiple"));
		privacyList->setTitle(tr("Privacy lists"));
		serviceMessages->setTitle(tr("Service messages"));
		changeMyDetails->setTitle(tr("View/change my details"));
		changeMyPassword->setTitle(tr("Change my password"));

		quitAction->setText(tr("&Quit"));
		settingsAction->setText(tr("&Settings..."));
		switchUserAction->setText(tr("Switch user"));
	}
	emit updateTranslation();
}

void qutIM::readTranslation()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	currentTranslation = settings.value("style/tr", 0).toInt();
	loadTranslation(currentTranslation);
}

void qutIM::on_showHideButton_clicked()
{
	if (!showOffline)
	   ui.showHideButton->setIcon(QIcon(":/icons/crystal_project/hideoffline.png"));
	else 
		ui.showHideButton->setIcon(QIcon(":/icons/crystal_project/shhideoffline.png"));
	
	
	if ( mergeAccounts )
	{
		hideOffline(!showOffline);
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
		settings.setValue("contactlist/offline", showOffline);
        }
        else
		{
                foreach(icqAccount *account, icqList)
			account->getProtocol()->getContactListClass()->offlineHideButtonClicked(!showOffline);

		showOffline = !showOffline;
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
		settings.setValue("contactlist/offline", showOffline);
	}
}
