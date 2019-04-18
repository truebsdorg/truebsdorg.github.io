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


#ifndef QUTIMSETTINGS_H
#define QUTIMSETTINGS_H

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QDesktopWidget>
#include <QtGui/QDialog>
#include <QMessageBox>
#include "ui_qutimsettings.h"
#include "mainsettings.h"
//#include "pluginsettings.h"
#include "../protocol/oscar/icq/icqaccount.h"

class qutimSettings : public QDialog
{
    Q_OBJECT

public:
    qutimSettings(QWidget *parent = 0);
    ~qutimSettings();
    void createSettingsWidget();
    void applyDisable() { ui.applyButton->setEnabled(false); };
    void loadAllSettings();
    void setIcqList(QList<icqAccount*> *list){ icqList = list; };
public slots:
	void onUpdateTranslation();
private slots:
	void changeSettings(QTreeWidgetItem *, QTreeWidgetItem *);
	void on_okButton_clicked();
	void on_applyButton_clicked();
	void mainChange() { ui.applyButton->setEnabled(true); mainSettingsChanged = true;}
	void changeIcqSettings(int);
	void enableApply() { ui.applyButton->setEnabled(true); }

signals:
	void generalSettingsChanged();


private:
	QPoint desktopCenter();
	void saveAllSettings();
	void createSettingsTree();
	void createWidgetsStack();

	QSettings *settings;
	bool mainSettingsChanged;
	QList<icqAccount* > *icqList;
	int currentAccountIndex;
	int currentSettingsIndex;
	bool createdUinSettings;
    Ui::qutimSettingsClass ui;
    mainSettings *msettings;
//    pluginSettings *plugSettings;
    QTreeWidgetItem *general;
};

#endif // QUTIMSETTINGS_H
