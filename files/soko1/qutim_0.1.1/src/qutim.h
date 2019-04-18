/*
    qutim

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


#ifndef QUTIM_H
#define QUTIM_H

#include <QWidget>


#include "ui_qutim.h"
#include "idle/idle.h"
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
	#include "ex/exsystrayicon.h"
#else
	#include <QSystemTrayIcon>
#endif

//#include "accountbutton.h"
#include "../protocol/oscar/icq/icqaccount.h"
#include "../protocol/oscar/icq/treegroupitem.h"
#include "../protocol/oscar/icq/treebuddyitem.h"
#include "../protocol/oscar/icq/contactlist.h"

class QCloseEvent;
class QMutex;
class QMenu;
class aboutInfo;
class qutimSettings;

class eventEater : public QObject
{
	Q_OBJECT
	
public:
	eventEater(QWidget *parent = 0) : QObject(parent), fChanged(false) { }
	
        bool getChanged() const { return fChanged; };
        void setChanged(const bool changed) { fChanged = changed; };
		
protected:
	bool fChanged;
	
	bool eventFilter(QObject *obj, QEvent *event);
	
};


class qutIM : public QWidget
{
    Q_OBJECT
	
public:
    qutIM(QWidget *parent = 0);
	virtual ~qutIM();
	
	static qutIM *getInstance();
	bool isShouldRun() { return bShouldRun; }
	
private slots:
	void trayActivated(QSystemTrayIcon::ActivationReason);
	void updateTrayIcon(const QIcon &);
	
	void appQuit();
	
	void qutimSettingsMenu();
	void reloadGeneralSettings();
	void destroySettings();
		
	void addAccount(const QString &);
	void removeAccount(const QString &);
	
	void switchUser();
	void setStatusFrom(const QString &); 
	
	void itemClicked(QTreeWidgetItem *, int);
	void itemDoubleClicked(QTreeWidgetItem *);
	
	void getNewMessage();
	void updateMessageIcon();
	
	void accountReadAllMessages();
	
	void getGroupList(QHash<quint16, treeGroupItem *>);
	void buddyChangeStatus(treeBuddyItem *, bool);
	
	void updateSorting();
	void reupdateList();
	void checkEventChanging();
	void addToEventList(bool);
	
	void updateTrayToolTip();
	
	void on_contactListTree_customContextMenuRequested ( const QPoint & );
	void on_infoButton_clicked();
	void infoWindowDestroyed(QObject *);
	void on_showHideButton_clicked();
	
protected:
	virtual void resizeEvent ( QResizeEvent * event );
	virtual void closeEvent(QCloseEvent *event);
	virtual void focusOutEvent( QFocusEvent * event );
	virtual void focusInEvent ( QFocusEvent * event ); 
    
private:
	static qutIM		*fInstance;
	static QMutex		fInstanceGuard;

    Ui::qutIMClass ui;
	
    QAction *quitAction;
    QAction *settingsAction;
    QAction *switchUserAction;
    QMenu *trayMenu;
    QMenu *mainMenu;
    qutimSettings *settingsDialog;
	
    QMenu *addFindUsers;
    QMenu *sendMultiple;
    QMenu *privacyList;
    QMenu *serviceMessages;
    QMenu *changeMyDetails;
    QMenu *changeMyPassword;
	
    QList<icqAccount *> icqList;
    QList<icqAccount *> messageStack;
    QList<icqAccount *> updateEventList;
	bool bShouldRun;
    bool createMenuAccounts;
    bool addToStatusTrayMenu;
    bool letMeQuit;
    bool unreadMessages;
    bool msgIcon;
    QMenu *currentStatusMenu;
    bool mergeAccounts;
    bool showGroups;
    bool hideEmptyGroups;
    bool hideSeparators;
    quint8 sorting;
    bool showOffline;
    bool autoHide;
    int hideSec;
	
	CLWindowStyle fWindowStyle;

#ifndef Q_OS_WIN32
    QSystemTrayIcon *trayIcon;
#else
    ExSysTrayIcon *trayIcon;
#endif

    contactSeparator *onlineList;
    
    contactSeparator *offlineList;
    QTimer *timer;
    QHash<QString, treeGroupItem *> mergeGroupList;
    eventEater *eventObject;
	
    int currentTranslation;
    aboutInfo *infoWindow;
    bool aboutWindowOpen;
    QTranslator applicationTranslator;

    QString currentStyle;
    QIcon tempIcon;
	// Idle detector
	Idle fIdleDetector;
		    
    void mergeAccountsToList(bool);
	
    void createLoginDialog();
    void createTrayIcon();
    void createActions();
	void createMainMenu();
	
    void saveMainSettings();
    void loadMainSettings();
	
	void rellocateSettingsDialog(qutimSettings *settings);
	
    void createContactList();
    void createAccountLine();
	
    void updateTrayStatus();
    void statusFromFirstLog();
    void updateCurrentTrayStatusmenu(bool);

    void readMessages();
	
	void reloadCLWindowStyle(const QSettings &settings);
	
    void createGroups();
    void removeGroups();
    void showHideGroups(bool);
    void hideEmptyGr(bool);
    void hideOffline(bool);
    void setHideSeparators(bool);
    
    void loadStyle();
    void loadTranslation(const int);
    void readTranslation();


signals:
	void updateTranslation();

};

#endif // QUTIM_H
