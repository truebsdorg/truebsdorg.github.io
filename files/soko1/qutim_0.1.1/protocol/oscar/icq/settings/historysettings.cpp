/*
    historySettings

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


#include "historysettings.h"

historySettings::historySettings(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	changed = false;
	connect (ui.saveHistoryBox , SIGNAL(stateChanged(int)),
		this, SLOT(widgetStateChanged()));
	connect (ui.nilHistoryBox , SIGNAL(stateChanged(int)),
		this, SLOT(widgetStateChanged()));
	connect (ui.serviceBox , SIGNAL(stateChanged(int)),
		this, SLOT(widgetStateChanged()));
	connect (ui.recentBox , SIGNAL(stateChanged(int)),
		this, SLOT(widgetStateChanged()));
	connect (ui.messagesCountBox , SIGNAL(valueChanged(int)),
		this, SLOT(widgetStateChanged()));
	connect (ui.onlineNotifyBox , SIGNAL(stateChanged(int)),
			this, SLOT(widgetStateChanged()));
	connect (ui.offlineNotifyBox , SIGNAL(stateChanged(int)),
			this, SLOT(widgetStateChanged()));
	connect (ui.readAwayNotifyBox , SIGNAL(stateChanged(int)),
			this, SLOT(widgetStateChanged()));
}

historySettings::~historySettings()
{

}

void historySettings::loadSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("history");
	ui.saveHistoryBox->setChecked(settings.value("save", true).toBool());
	ui.nilHistoryBox->setChecked(settings.value("nil", true).toBool());
	ui.serviceBox->setChecked(settings.value("service", true).toBool());
	ui.recentBox->setChecked(settings.value("recent", true).toBool());
	ui.messagesCountBox->setValue(settings.value("recentcount", 3).toUInt());
	ui.onlineNotifyBox->setChecked(settings.value("onlnot", false).toBool());
	ui.offlineNotifyBox->setChecked(settings.value("offnot", false).toBool());
	ui.readAwayNotifyBox->setChecked(settings.value("awaynot", true).toBool());
	settings.endGroup();
}

void historySettings::saveSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("history");
	
	settings.setValue("save", ui.saveHistoryBox->isChecked());
	settings.setValue("nil", ui.nilHistoryBox->isChecked());
	settings.setValue("service", ui.serviceBox->isChecked());
	settings.setValue("recent", ui.recentBox->isChecked());
	settings.setValue("recentcount", ui.messagesCountBox->value());
	settings.setValue("onlnot", ui.onlineNotifyBox->isChecked());
	settings.setValue("offnot", ui.offlineNotifyBox->isChecked());
	settings.setValue("awaynot", ui.readAwayNotifyBox->isChecked());
	settings.endGroup();
	
	if ( changed )
			emit settingsSaved();
	changed = false;
}
