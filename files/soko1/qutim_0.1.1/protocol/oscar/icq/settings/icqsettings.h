/*
    icqSettings

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


#ifndef ICQSETTINGS_H
#define ICQSETTINGS_H

#include <QtGui/QWidget>
#include <QSettings>
#include "ui_icqsettings.h"

class icqSettings : public QWidget
{
    Q_OBJECT

public:
    icqSettings(QWidget *parent = 0);
    ~icqSettings();
    void loadSettings(const QString &);
    void saveSettings(const QString &);

private slots:
	void widgetStateChanged() {   changed = true; emit settingsChanged();  }
	void clientIndexChanged(int);
    
signals:
    void settingsChanged();
    void settingsSaved();

private:
    Ui::icqSettingsClass ui;
    bool changed;
};

#endif // ICQSETTINGS_H
