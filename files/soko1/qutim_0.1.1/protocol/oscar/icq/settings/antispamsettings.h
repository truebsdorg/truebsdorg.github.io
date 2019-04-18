/*
    antiSpamSettings

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


#ifndef ANTISPAMSETTINGS_H
#define ANTISPAMSETTINGS_H

#include <QtGui>
#include "ui_antispamsettings.h"

class antiSpamSettings : public QWidget
{
    Q_OBJECT

public:
    antiSpamSettings(QWidget *parent = 0);
    ~antiSpamSettings();
    void loadSettings(const QString &);
    void saveSettings(const QString &);
    
private slots:
	void widgetStateChanged() { changed = true; emit settingsChanged(); }
	
signals:
    void settingsChanged();
    void settingsSaved();
    
private:
    Ui::antiSpamSettingsClass ui;
    bool changed;
};

#endif // ANTISPAMSETTINGS_H
