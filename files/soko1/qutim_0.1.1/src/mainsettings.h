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


#ifndef MAINSETTINGS_H
#define MAINSETTINGS_H

#include <QtGui/QWidget>
#include <QSettings>
#include <QDebug>
#include <QMap>
#include <QMapIterator>

#include "../protocol/oscar/icq/statusiconfactory.h"
#include "ui_mainsettings.h"

class mainSettings : public QWidget
{
    Q_OBJECT

public:
    mainSettings(QWidget *parent = 0);
    ~mainSettings();
    void loadSettings();
    void saveSettings();

private slots:
	void sizePosBoxChanged(int) { sizePosBoxChange = true; emit settingsChanged(); }
	void hideBoxChanged(int) { hideBoxChange = true; emit settingsChanged(); }
	void sIconsGBChanded(int);
	void widgetStateChanged() { emit settingsChanged(); }
	void on_clAccountsBox_toggled(bool);
	void onOpacitySliderValueChanged(int);
	void onWinStyleComboBoxIndexChanged(int);
	
signals:
	void settingsChanged();
	
private:
	bool sizePosBoxChange;
	bool hideBoxChange;
	bool sIconsChange;
	bool opacityChanged;

	QMap<QString, statusIconFactory *>  fStatusIconsMap;
    Ui::mainSettingsClass ui;

	void resetStates();
	bool loadStatusIcons();
};

#endif // MAINSETTINGS_H
