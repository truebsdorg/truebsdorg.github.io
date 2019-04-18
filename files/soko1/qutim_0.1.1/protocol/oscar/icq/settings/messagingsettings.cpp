/*
    messagingSettings

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


#include "messagingsettings.h"

messagingSettings::messagingSettings(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	changed = false;
	connect (ui.tabBox , SIGNAL(stateChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.chatNamesBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.timeComboBox , SIGNAL(currentIndexChanged(int)),
							this, SLOT(widgetStateChanged()));
	connect (ui.onEnterBox , SIGNAL(stateChanged(int)),
			this, SLOT(widgetStateChanged()));
	connect (ui.closeTabBox , SIGNAL(stateChanged(int)),
				this, SLOT(widgetStateChanged()));
	connect (ui.eventBox , SIGNAL(stateChanged(int)),
					this, SLOT(widgetStateChanged()));
	connect (ui.openNewBox , SIGNAL(stateChanged(int)),
						this, SLOT(widgetStateChanged()));
	connect (ui.codepageBox , SIGNAL(currentIndexChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.typingBox , SIGNAL(stateChanged(int)),
							this, SLOT(widgetStateChanged()));
	
	connect (ui.lightChatBox , SIGNAL(stateChanged(int)),
							this, SLOT(widgetStateChanged()));
	connect (ui.hideMessageBox , SIGNAL(stateChanged(int)),
							this, SLOT(widgetStateChanged()));
}

messagingSettings::~messagingSettings()
{

}

void messagingSettings::loadSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("messaging");
	ui.tabBox->setChecked(settings.value("tab",true).toBool());
	ui.chatNamesBox->setChecked(settings.value("chatnames",true).toBool());
	ui.timeComboBox->setCurrentIndex(settings.value("timestamp", 1).toInt());
	ui.onEnterBox->setChecked(settings.value("onenter", false).toBool());
	ui.closeTabBox->setChecked(settings.value("closeonsend", false).toBool());
	ui.eventBox->setChecked(settings.value("event", false).toBool());
	ui.openNewBox->setChecked(settings.value("opennew", false).toBool());
	int codepageIndex = ui.codepageBox->findText(settings.value("codepage", "Windows-1251").toString());
	if (codepageIndex > -1 )
		ui.codepageBox->setCurrentIndex(codepageIndex);
	else 
		ui.codepageBox->setCurrentIndex(47);
	ui.typingBox->setChecked(settings.value("typing", false).toBool());
	ui.lightChatBox->setChecked(settings.value("lightchat", false).toBool());
	ui.hideMessageBox->setChecked(settings.value("dshowmsg", false).toBool());
	settings.endGroup();
}

void messagingSettings::saveSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("messaging");
	settings.setValue("tab", ui.tabBox->isChecked());
	settings.setValue("chatnames", ui.chatNamesBox->isChecked());
	settings.setValue("timestamp", ui.timeComboBox->currentIndex());
	settings.setValue("onenter", ui.onEnterBox->isChecked());
	settings.setValue("closeonsend", ui.closeTabBox->isChecked());
	settings.setValue("event", ui.eventBox->isChecked());
	settings.setValue("opennew", ui.openNewBox->isChecked());
	settings.setValue("codepage", ui.codepageBox->currentText());
	settings.setValue("typing", ui.typingBox->isChecked());
	settings.setValue("lightchat", ui.lightChatBox->isChecked());
	settings.setValue("dshowmsg", ui.hideMessageBox->isChecked());
	settings.endGroup();
	
	
	if ( changed )
			emit settingsSaved();
	changed = false;
}
