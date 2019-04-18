/*
    mainSettings

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

#include "mainsettings.h"

#include <QDir>

mainSettings::mainSettings(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	resetStates(); // reset widgets states
	connect(ui.sizePosBox, SIGNAL(stateChanged(int)),
			this, SLOT(sizePosBoxChanged(int)));
	connect(ui.hideBox, SIGNAL(stateChanged(int)),
				this, SLOT(hideBoxChanged(int)));
	connect(ui.loginDialogBox, SIGNAL(stateChanged(int)),
					this, SLOT(widgetStateChanged()));
	connect(ui.menuAccountBox,SIGNAL(stateChanged(int)),
			this, SLOT(widgetStateChanged()));
	connect(ui.showSettingsBox,SIGNAL(stateChanged(int)),
			this, SLOT(widgetStateChanged()));
	connect(ui.accountBox,SIGNAL(currentIndexChanged(int)),
			this, SLOT(widgetStateChanged()));
	connect(ui.noSysTrayButton,SIGNAL(toggled(bool)),
			this, SLOT(widgetStateChanged()));
	connect(ui.statusFromButton,SIGNAL(toggled(bool)),
				this, SLOT(widgetStateChanged()));
	connect(ui.trayAccountBox,SIGNAL(currentIndexChanged(int)),
				this, SLOT(widgetStateChanged()));
	connect(ui.inSysTrayBox, SIGNAL(stateChanged(int)),
			this, SLOT(widgetStateChanged()));
	connect(ui.clAccountsBox, SIGNAL(toggled(bool)),
				this, SLOT(widgetStateChanged()));
	connect(ui.showGroupsBox, SIGNAL(stateChanged(int)),
				this, SLOT(widgetStateChanged()));
	connect(ui.separatorBox, SIGNAL(stateChanged(int)),
					this, SLOT(widgetStateChanged()));
	connect(ui.hideGroupsBox, SIGNAL(stateChanged(int)),
					this, SLOT(widgetStateChanged()));
	connect(ui.unsortedButton,SIGNAL(toggled(bool)),
				this, SLOT(widgetStateChanged()));
	connect(ui.byStatusButton,SIGNAL(toggled(bool)),
				this, SLOT(widgetStateChanged()));
	connect(ui.byNameButton,SIGNAL(toggled(bool)),
				this, SLOT(widgetStateChanged()));
	connect(ui.offlineBox, SIGNAL(stateChanged(int)),
						this, SLOT(widgetStateChanged()));
	connect(ui.clientColumnBox, SIGNAL(stateChanged(int)),
						this, SLOT(widgetStateChanged()));
	connect(ui.onTopBox, SIGNAL(stateChanged(int)),
							this, SLOT(widgetStateChanged()));
	connect(ui.ahideBox, SIGNAL(stateChanged(int)),
							this, SLOT(widgetStateChanged()));
	connect(ui.secondsBox, SIGNAL(valueChanged(int)),
								this, SLOT(widgetStateChanged()));
	connect(ui.rowBox, SIGNAL(stateChanged(int)),
								this, SLOT(widgetStateChanged()));

	// Connect opacity slider moves
	connect(ui.clOpacitySlider, SIGNAL(valueChanged(int)),
				this, SLOT(onOpacitySliderValueChanged(int)));
	
//	ui.styleComboBox->addItem(tr("Default"), QVariant(":/qss/default.qss"));
//	ui.styleComboBox->addItem(tr("Simple"), QVariant(":/qss/simple.qss"));
	connect(ui.styleComboBox,SIGNAL(currentIndexChanged(int)),
				this, SLOT(widgetStateChanged()));
	connect(ui.translationComboBox,SIGNAL(currentIndexChanged(int)),
				this, SLOT(widgetStateChanged()));

	// Setup status icons combobox
	if (loadStatusIcons())
	{
		ui.sIconsComboBox->clear();
		QMapIterator<QString, statusIconFactory *> iterator(fStatusIconsMap);
		while(iterator.hasNext())
		{
			iterator.next();
			ui.sIconsComboBox->addItem(QIcon(iterator.value()->getOnlinePath()),
										iterator.key());
		}

		connect(ui.sIconsComboBox, SIGNAL(currentIndexChanged(int)),
				this, SLOT(sIconsGBChanded(int)));
	}
	
	connect(ui.clWinStyleComboBox, SIGNAL(currentIndexChanged(int)), 
				this, SLOT(onWinStyleComboBoxIndexChanged(int)));

//	ui.label_2->setHidden(true);
//	ui.translationComboBox->setHidden(true);
	
}

mainSettings::~mainSettings()
{
	for(QMap<QString, statusIconFactory *>::iterator i = fStatusIconsMap.begin(); i != fStatusIconsMap.end(); ++i)
	{
		delete (*i);
		(*i) = NULL;
	}
	fStatusIconsMap.clear();
}


void mainSettings::resetStates()
{
	sizePosBoxChange = false;
	hideBoxChange = false;
	sIconsChange = false;
	opacityChanged = false;
}

bool mainSettings::loadStatusIcons()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");

	for(QMap<QString, statusIconFactory *>::iterator i = fStatusIconsMap.begin(); i != fStatusIconsMap.end(); ++i)
	{
		delete (*i);
		(*i) = NULL;
	}
	fStatusIconsMap.clear();
	fStatusIconsMap.insert(tr("Default"), new statusIconFactory(BuiltInStatusIcons));

	QString appPath = qApp->applicationDirPath();
	if (appPath.at(appPath.size()-1) == '/') appPath.chop(1);
	QStringList paths;

#if !defined(Q_WS_WIN)
	// ../share/qutim
	paths << appPath.section('/', 0, -2) + "/share/qutim";
	// ~/.config/qutim
	paths << settings.fileName().section('/', 0, -2);
#endif
	// ./
	paths << appPath;
#if defined(Q_WS_MAC)
	// ./ Application bundle
	paths << qApp->applicationDirPath().section('/', 0, -2) + "/Resources";
#endif
	foreach(QString path, paths)
	{
		QString iconPath =  path  + "/StatusIcons";
		QDir sIconsDir(iconPath);
		QStringList filters;
		filters << "*.AdiumStatusIcons";
		QStringList sIconsList = sIconsDir.entryList(filters);

		foreach(QString sIcon, sIconsList)
		{
			QString sIconName = sIcon.section('.', 0, 0);
			QString sIconDir = iconPath + "/" + sIcon;

			fStatusIconsMap.insert(sIconName, new statusIconFactory(AdiumStatusIcons, sIconDir));
		}
		
		QString stylePath = path  + "/styles"; 
		
		QDir themePath(stylePath);

		QStringList themes(themePath.entryList(QDir::AllDirs | QDir::NoDotAndDotDot));

		foreach(QString dirName, themes)
		{
			ui.styleComboBox->addItem(dirName, stylePath + "/" + dirName);
		}
	}

	ui.styleComboBox->setCurrentIndex(ui.styleComboBox->findData(settings.value("style/path", "").toString()));
	return true;
}

void mainSettings::loadSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	
	ui.styleComboBox->setCurrentIndex(settings.value("style/index", 0).toInt());
	ui.translationComboBox->setCurrentIndex(settings.value("style/tr", 0).toInt());
	
	settings.beginGroup("mainwindow");
	ui.sizePosBox->setChecked(settings.value("saveSizPos", true).toBool());
	ui.hideBox->setChecked(settings.value("hideStart", false).toBool());
	ui.menuAccountBox->setChecked(settings.value("accountMenu", true).toBool());
	settings.endGroup();
	ui.loginDialogBox->setChecked(settings.value("logindialog/showstart", true).toBool());
	QSettings settings2(QSettings::IniFormat, QSettings::UserScope, "qutim", "accounts");
	QStringList accountList = settings2.value("accounts/list").toStringList();
	foreach(QString account, accountList)
	{
		ui.accountBox->addItem(QIcon(":/icons/qutim.png"),account);
		ui.trayAccountBox->addItem(QIcon(":/icons/qutim.png"),account);
	}
	if ( settings.value("setting/bydefault", false).toBool() )
	{
		ui.showSettingsBox->setChecked(true);
		int index = accountList.indexOf(settings.value("setting/defacc").toString());
		ui.accountBox->setCurrentIndex(index > -1 ? index : 0);
	}
	
	
	int index = accountList.indexOf(settings.value("systray/current").toString());
	switch ( settings.value("systray/show", 1).toInt() )
	{
	case 0:
		ui.noSysTrayButton->setChecked(true);
		ui.trayAccountBox->setEnabled(false);
		ui.inSysTrayBox->setEnabled(false);
		break;
	case 1:
		ui.statusFromButton->setChecked(true);
		ui.trayAccountBox->setEnabled(true);
		ui.inSysTrayBox->setEnabled(true);
		ui.inSysTrayBox->setChecked(settings.value("systray/inmenu", false).toBool());
		ui.trayAccountBox->setCurrentIndex(index > -1 ? index : 0);
		break;
	default:
		break;
	}
	
	
	bool shwGrps = settings.value("contactlist/groups", false).toBool();
	ui.showGroupsBox->setChecked(shwGrps);
//	ui.hideGroupsBox->setEnabled(shwGrps);
	ui.separatorBox->setChecked(settings.value("contactlist/hidesep", false).toBool());
	ui.hideGroupsBox->setChecked(settings.value("contactlist/hideempty", false).toBool());
	ui.offlineBox->setChecked(settings.value("contactlist/offline", false).toBool());
	ui.clientColumnBox->setChecked(settings.value("contactlist/clicons", false).toBool());
	ui.clAccountsBox->setChecked(settings.value("contactlist/merge", false).toBool());
	if (!ui.clAccountsBox->isChecked() )
		ui.hideGroupsBox->setEnabled(false);
	
	switch(settings.value("contactlist/sort", 1).toInt())
	{
	case 0:
		ui.unsortedButton->setChecked(true);
		break;
	case 3:
		ui.byStatusButton->setChecked(true);
		break;
	case 1:
		ui.byNameButton->setChecked(true);
		break;
	default:
		ui.unsortedButton->setChecked(true);
	}
	ui.onTopBox->setChecked(settings.value("contactlist/ontop", false).toBool());
	ui.ahideBox->setChecked(settings.value("contactlist/ahide", false).toBool());
	ui.secondsBox->setValue(settings.value("contactlist/hidesec", 60).toInt());
	ui.rowBox->setChecked(settings.value("contactlist/colorrow", true).toBool());

	// Read current status icon theme
	statusIconType iconType = static_cast<statusIconType>(settings.value("statusicons/type", BuiltInStatusIcons).toInt());
	QString iconSettingsFile = settings.value("statusicons/settings", QString("")).toString();
	
	{
		// Lets find index of current status icons in our QMap
		QMapIterator<QString, statusIconFactory *> iterator(fStatusIconsMap);
		
		int counter = 0;
		while (iterator.hasNext())
		{
			iterator.next();
			if (iconSettingsFile == iterator.value()->getSettingsFilePath())
			{
				break;
			}
			else if (iconType == BuiltInStatusIcons && iconSettingsFile == QString(""))
				break;
				
			counter++;
		}
		ui.sIconsComboBox->setCurrentIndex(counter);
	}

	// Load opacity of the main window (contact list)
	int nOpacity = settings.value("mainwindow/opacity", 100).toInt();
	ui.clOpacitySlider->setValue(nOpacity);
	
	// Load window style
	int nWindowStyle = settings.value("contactlist/windowstyle", 0).toInt();
	ui.clWinStyleComboBox->setCurrentIndex(nWindowStyle);
}

void mainSettings::saveSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	int index = ui.styleComboBox->currentIndex();
	
	settings.setValue("style/index", index);
	
	settings.setValue("style/path", ui.styleComboBox->itemData(ui.styleComboBox->currentIndex()));
	
	settings.setValue("style/tr", ui.translationComboBox->currentIndex());
	
	settings.beginGroup("mainwindow");
	if( sizePosBoxChange ) settings.setValue("saveSizPos", ui.sizePosBox->isChecked());
	if( hideBoxChange ) settings.setValue("hideStart", ui.hideBox->isChecked());
	settings.setValue("accountMenu", ui.menuAccountBox->isChecked());
	settings.endGroup();
	settings.setValue("logindialog/showstart", ui.loginDialogBox->isChecked());
	if ( ui.showSettingsBox->isChecked() )
	{
		settings.setValue("setting/bydefault", true);
		settings.setValue("setting/defacc", ui.accountBox->currentText());
	} else {
		settings.remove("setting/bydefault");
		settings.remove("setting/defacc");
	}
	if ( ui.noSysTrayButton->isChecked() )
	{
		settings.setValue("systray/show", 0);
	}
	
	
	if ( ui.statusFromButton->isChecked() )
	{
		settings.setValue("systray/show", 1);
		settings.setValue("systray/current", ui.trayAccountBox->currentText());
		settings.setValue("systray/inmenu", ui.inSysTrayBox->isChecked());
	} else {
		settings.remove("systray/current");
		settings.remove("systray/inmenu");
	}
		settings.setValue("contactlist/merge", ui.clAccountsBox->isChecked());
		settings.setValue("contactlist/groups", ui.showGroupsBox->isChecked());
		settings.setValue("contactlist/hideempty", ui.hideGroupsBox->isChecked());
		settings.setValue("contactlist/hidesep", ui.separatorBox->isChecked());
	if ( ui.unsortedButton->isChecked() )
		settings.setValue("contactlist/sort",0);
	if ( ui.byStatusButton->isChecked() )
			settings.setValue("contactlist/sort",3);
	if ( ui.byNameButton->isChecked() )
			settings.setValue("contactlist/sort",1);
	settings.setValue("contactlist/offline", ui.offlineBox->isChecked());
	settings.setValue("contactlist/clicons", ui.clientColumnBox->isChecked());
	settings.setValue("contactlist/ontop",ui.onTopBox->isChecked());
	settings.setValue("contactlist/ahide",ui.ahideBox->isChecked());
	settings.setValue("contactlist/hidesec", ui.secondsBox->value());
	settings.setValue("contactlist/colorrow", ui.rowBox->isChecked());

	if (sIconsChange)
	{
		int sIconIndex = ui.sIconsComboBox->currentIndex();
		if (ui.sIconsComboBox->currentText() == tr("Default"))
		{
			settings.setValue("statusicons/type", BuiltInStatusIcons);
			settings.setValue("statusicons/settings", QString(""));
		}
		else
		{
			int counter = 0;
			QMapIterator<QString, statusIconFactory *> iterator(fStatusIconsMap);
			while(iterator.hasNext())
			{
				iterator.next();
				if (sIconIndex == counter++)
				{
					settings.setValue("statusicons/type", AdiumStatusIcons);
					settings.setValue("statusicons/settings", iterator.value()->getSettingsFilePath());
					break;
				}
			}
		}
	}

	// Save current opacity value
	if (opacityChanged)
	{
		settings.setValue("mainwindow/opacity", ui.clOpacitySlider->value());
	}
	
	// Save windows style
	settings.setValue("contactlist/windowstyle", ui.clWinStyleComboBox->currentIndex());
}

void mainSettings::sIconsGBChanded(int)
{
	sIconsChange = true;
	emit settingsChanged();
}

void mainSettings::on_clAccountsBox_toggled(bool on)
{
	if ( on )
	if ( ui.showGroupsBox->isChecked() )
		ui.hideGroupsBox->setEnabled(true);
}

void mainSettings::onOpacitySliderValueChanged(int value)
{
	ui.clOpacityPersLbl->setText(QString("%1%").arg(value));
	opacityChanged = true;

	emit settingsChanged();
}

void mainSettings::onWinStyleComboBoxIndexChanged(int /* index */)
{
	emit settingsChanged();
}
	
