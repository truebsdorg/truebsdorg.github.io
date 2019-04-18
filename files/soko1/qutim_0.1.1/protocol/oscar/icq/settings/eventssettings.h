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


#ifndef EVENTSSETTINGS_H
#define EVENTSSETTINGS_H

#include <QtGui>
#include "ui_eventssettings.h"

class eventsSettings : public QWidget
{
    Q_OBJECT

public:
    eventsSettings(QWidget *parent = 0);
    ~eventsSettings();
    void loadSettings(const QString &);
    void saveSettings(const QString &);
    
private slots:
	void widgetStateChanged() { changed = true; emit settingsChanged(); }
	void on_testEventButton_clicked(){emit showTestEvent( ui.testBox->itemData(ui.testBox->currentIndex()).toInt());}
	
signals:
    void settingsChanged();
    void settingsSaved();
    void showTestEvent(int);

private:
    Ui::eventsSettingsClass ui;
    bool changed;
    void findThemes(const QString &);
};

#endif // EVENTSSETTINGS_H
