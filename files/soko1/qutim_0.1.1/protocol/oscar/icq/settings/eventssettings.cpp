/*
    eventsSettings

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


#include "eventssettings.h"

eventsSettings::eventsSettings(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	changed = false;
	
	connect (ui.trayMessagesBox , SIGNAL(toggled(bool)),
										this, SLOT(widgetStateChanged()));
	connect (ui.widthSpinBox , SIGNAL(valueChanged(int)),
											this, SLOT(widgetStateChanged()));
	connect (ui.heightSpinBox , SIGNAL(valueChanged(int)),
											this, SLOT(widgetStateChanged()));
	connect (ui.timeSpinBox , SIGNAL(valueChanged(int)),
											this, SLOT(widgetStateChanged()));
	connect (ui.positionComboBox , SIGNAL(currentIndexChanged(int)),
											this, SLOT(widgetStateChanged()));
	connect (ui.styleComboBox , SIGNAL(currentIndexChanged(int)),
											this, SLOT(widgetStateChanged()));
	connect (ui.trayBalloonBox , SIGNAL(stateChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.balloonSecBox , SIGNAL(valueChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect (ui.trayBlinkBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.trayButtonBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.dshowBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.signOnBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.signOffBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.typingBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.changeStatusBox , SIGNAL(stateChanged(int)),
									this, SLOT(widgetStateChanged()));
	connect (ui.awayBox , SIGNAL(stateChanged(int)),
										this, SLOT(widgetStateChanged()));
	connect (ui.trayThemeBox , SIGNAL(currentIndexChanged(int)),
											this, SLOT(widgetStateChanged()));
	ui.trayThemeBox->addItem(tr("Default"), "");
	ui.testBox->addItem(tr("Incoming message"), 0);
	ui.testBox->addItem(tr("Online alert"), 1);
	ui.testBox->addItem(tr("System"), 2);
}

eventsSettings::~eventsSettings()
{

}

void eventsSettings::loadSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("events");
	
	
	QStringList paths;
	QString appPath = qApp->applicationDirPath();
	if (appPath.at(appPath.size()-1) == '/') appPath.chop(1);

#if !defined(Q_WS_WIN)
	// ../share/qutim
	paths << appPath.section('/', 0, -2) + "/share/qutim";
	// ~/.config/qutim
	paths << settings.fileName().section('/', 0, -3);
#endif
	// ./
	paths << appPath;
#if defined(Q_WS_MAC)
	// ./ Application bundle
	paths << qApp->applicationDirPath().section('/', 0, -2) + "/Resources";
#endif
	
	foreach(QString path, paths)
	{
		findThemes(path + "/traythemes");
	}	
	
	ui.trayThemeBox->setCurrentIndex(ui.trayThemeBox->findData(settings.value("path", "").toString()));
	
	ui.trayMessagesBox->setChecked(settings.value("showtray", true).toBool());
	
	
	int width = settings.value("width", 200).toInt();
	width = width < 100 ? 100 : width;
	width = width > 500 ? 500 : width;
	ui.widthSpinBox->setValue(width);
	
	int height = settings.value("height", 150).toInt();
	height = height < 100 ? 100 : height;
	height = height > 400 ? 400 : height;
	ui.heightSpinBox->setValue(height);
		
	int trayTime = settings.value("traytime", 6).toInt();
	trayTime = trayTime < 1 ? 1 : trayTime;
	trayTime = trayTime > 60 ? 60 : trayTime;
	ui.timeSpinBox->setValue(trayTime);
	
	int position = settings.value("position", 3).toInt();
	position = position < 0 ? 0 : position;
	position= position > 3 ? 3 : position;
	ui.positionComboBox->setCurrentIndex(position);
	
	int style = settings.value("style", 2).toInt();
	style = style < 0 ? 0 : style;
	style = style > 2 ? 2 : style;
	ui.styleComboBox->setCurrentIndex(style);
	
	ui.trayBalloonBox->setChecked(settings.value("showballoon", false).toBool());
	
	int balloonTime = settings.value("balloontime", 5).toInt();
	balloonTime = balloonTime < 1 ? 1 : balloonTime;
	balloonTime = balloonTime > 60 ? 60 : balloonTime;
	ui.balloonSecBox->setValue(balloonTime);
	
	ui.trayBlinkBox->setChecked(settings.value("trayblink", false).toBool());
	ui.trayButtonBox->setChecked(settings.value("traybutton", false).toBool());
	ui.dshowBox->setChecked(settings.value("dshowna", false).toBool());
	
	ui.signOnBox->setChecked(settings.value("notsignon", true).toBool());
	ui.signOffBox->setChecked(settings.value("notsignoff", true).toBool());
	ui.typingBox->setChecked(settings.value("nottyping", true).toBool());
	ui.awayBox->setChecked(settings.value("notaway", true).toBool());
	ui.changeStatusBox->setChecked(settings.value("notchange", true).toBool());
	
	settings.endGroup();
}

void eventsSettings::saveSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("events");
	
	settings.setValue("showtray", ui.trayMessagesBox->isChecked());
	settings.setValue("width", ui.widthSpinBox->value());
	settings.setValue("height", ui.heightSpinBox->value());
	settings.setValue("traytime", ui.timeSpinBox->value());
	settings.setValue("position", ui.positionComboBox->currentIndex());
	settings.setValue("style", ui.styleComboBox->currentIndex());
	
	settings.setValue("showballoon", ui.trayBalloonBox->isChecked());
	settings.setValue("balloontime", ui.balloonSecBox->value());
	settings.setValue("trayblink", ui.trayBlinkBox->isChecked());
	settings.setValue("traybutton", ui.trayButtonBox->isChecked());
	settings.setValue("dshowna", ui.dshowBox->isChecked());
	
	settings.setValue("notsignon", ui.signOnBox->isChecked());
	settings.setValue("notsignoff", ui.signOffBox->isChecked());
	settings.setValue("nottyping", ui.typingBox->isChecked());
	settings.setValue("notchange", ui.changeStatusBox->isChecked());
	settings.setValue("notaway", ui.awayBox->isChecked());
	settings.setValue("path", ui.trayThemeBox->itemData(ui.trayThemeBox->currentIndex()));
	settings.endGroup();
	
	
	if ( changed )
			emit settingsSaved();
	changed = false;
}

void eventsSettings::findThemes(const QString& path)
{
	QDir themePath(path);

		QStringList themes(themePath.entryList(QDir::AllDirs | QDir::NoDotAndDotDot));

		foreach(QString dirName, themes)
		{
				ui.trayThemeBox->addItem(dirName, path + "/" + dirName);
		}
}
