/*
    qutimSettings

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


#include "qutimsettings.h"
#include <QDebug>

qutimSettings::qutimSettings(QWidget *parent) : 
	QDialog(parent),
	currentAccountIndex(0)
{
	ui.setupUi(this);
	createdUinSettings = false;
	connect( ui.accountBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeIcqSettings(int)));
	move(desktopCenter()); // move dialog to desktop center
	setMinimumSize(size());
	ui.settingsTree->header()->hide();
	createSettingsWidget();
}
qutimSettings::~qutimSettings()
{
	if (icqList->size() >= 0)
		icqList->at(currentAccountIndex)->removeSettings();
	delete msettings;
//	delete plugSettings;
}

QPoint qutimSettings::desktopCenter()
{
	QDesktopWidget desktop;
	return QPoint(desktop.width() / 2 - size().width() / 2, desktop.height() / 2 - size().height() / 2);
}

void qutimSettings::createSettingsWidget()
{
	general = new QTreeWidgetItem(ui.settingsTree);
	general->setText(0, tr("General"));
	general->setIcon(0, QIcon(":/icons/crystal_project/mainsettings.png"));
	createSettingsTree();
	ui.settingsTree->resizeColumnToContents(0);
	createWidgetsStack();

    connect(ui.settingsTree,
            SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(changeSettings(QTreeWidgetItem *, QTreeWidgetItem *)));
	ui.settingsTree->setCurrentItem(general);
//
//	QTreeWidgetItem *plugin = new QTreeWidgetItem(ui.settingsTree);
//	plugin->setText(0, tr("Plugins"));
//	plugin->setIcon(0, QIcon(":/icons/crystal_project/plugin.png"));
//
//	plugSettings = new pluginSettings;
//	ui.settingsStack->addWidget(plugSettings);
}

void qutimSettings::createSettingsTree()
{

}

void qutimSettings::createWidgetsStack()
{
	msettings = new mainSettings;
	connect(msettings, SIGNAL(settingsChanged()),
		this, SLOT(mainChange()));
	ui.settingsStack->addWidget(msettings);
}

void qutimSettings::changeSettings(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (!current) current = previous;
    ui.settingsStack->setCurrentIndex(ui.settingsTree->indexOfTopLevelItem(current));
}

void qutimSettings::on_okButton_clicked()
{
	if (ui.applyButton->isEnabled()) saveAllSettings();
	accept();
}

void qutimSettings::on_applyButton_clicked()
{
	saveAllSettings();
	ui.applyButton->setEnabled(false);
}

void qutimSettings::saveAllSettings()
{
//	mainSettings *msettings = dynamic_cast<mainSettings *>(ui.settingsStack->widget(0));
	mainSettings *msettings = qobject_cast<mainSettings *>(ui.settingsStack->widget(0));
	msettings->saveSettings();

	if (mainSettingsChanged)
		emit generalSettingsChanged();
	if (ui.accountBox->count())
		if (icqList->at(ui.accountBox->currentIndex())->getChangeState())
			icqList->at(ui.accountBox->currentIndex())->saveAllSettings();

}

void qutimSettings::loadAllSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "accounts");
	QStringList accountList = settings.value("accounts/list").toStringList();
//	ui.accountBox->clear();
	foreach( QString account, accountList)
	{
		ui.accountBox->addItem(QIcon(":/icons/qutim.png"), account);
	}

	QSettings settings2(QSettings::IniFormat, QSettings::UserScope, "qutim", "mainsettings");
	int index = accountList.indexOf(settings2.value("setting/defacc", "").toString());
	if (index > -1)
	{
		ui.accountBox->setCurrentIndex(index);
	}
	mainSettingsChanged = false;

//	mainSettings *msettings = dynamic_cast<mainSettings *>(ui.settingsStack->widget(0));
	mainSettings *msettings = qobject_cast<mainSettings *>(ui.settingsStack->widget(0));
	msettings->loadSettings();
}

void qutimSettings::changeIcqSettings(int index)
{
	if (createdUinSettings)
	{
		if (icqList->at(currentAccountIndex)->getChangeState())
		{
			QString account(icqList->at(currentAccountIndex)->getIcquin());
			QMessageBox msgBox(QMessageBox::NoIcon, tr("Delete account"),
				tr("Save %1 settings?").arg(account), QMessageBox::Yes | QMessageBox::No, this);
			switch( msgBox.exec() )
			{
			case QMessageBox::Yes:
				icqList->at(currentAccountIndex)->saveAllSettings();
				break;

			case QMessageBox::No:
				break;

			default:
				break;
			}
		}

		disconnect( icqList->at(currentAccountIndex), SIGNAL(changeSettingsApply()),
			this, SLOT(enableApply()));
		icqList->at(currentAccountIndex)->removeSettings();
	}

	// Sanity check to avoid crash
	if (index >= 0 && index < icqList->size())
	{
		icqList->at(index)->createSettings(ui.settingsTree, ui.settingsStack);
		connect(icqList->at(index), SIGNAL(changeSettingsApply()),
			this, SLOT(enableApply()));
		createdUinSettings = true;
		currentAccountIndex = index;
	}
}

void qutimSettings::onUpdateTranslation()
{
	general->setText(0, tr("General"));

	if (icqList->size())
	{
		icqList->at(currentAccountIndex)->removeSettings();
		icqList->at(currentAccountIndex)->createSettings(ui.settingsTree,
			ui.settingsStack);
	}
}
