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


#ifndef HISTORYSETTINGS_H
#define HISTORYSETTINGS_H

#include <QtGui>
#include "ui_historysettings.h"

class historySettings : public QWidget
{
    Q_OBJECT

public:
    historySettings(QWidget *parent = 0);
    ~historySettings();
    void loadSettings(const QString &);
    void saveSettings(const QString &);
    
private slots:
	void widgetStateChanged() { changed = true; emit settingsChanged(); }
	
signals:
    void settingsChanged();
    void settingsSaved();
    
private:
    Ui::historySettingsClass ui;
    bool changed;
};

#endif // HISTORYSETTINGS_H
