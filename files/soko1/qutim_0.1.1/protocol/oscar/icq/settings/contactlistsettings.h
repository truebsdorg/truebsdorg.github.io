/*
    contactListSettings

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


#ifndef CONTACTLISTSETTINGS_H
#define CONTACTLISTSETTINGS_H

#include <QtGui>
#include <QColorDialog>
#include "ui_contactlistsettings.h"

class contactListSettings : public QWidget
{
    Q_OBJECT

public:
    contactListSettings(QWidget *parent = 0);
    ~contactListSettings();
    void loadSettings(const QString &);
    void saveSettings(const QString &);

private slots:
	void widgetStateChanged() { changed = true; emit settingsChanged(); }
	void on_accountColorButton_clicked();
	void on_groupColorButton_clicked();
	void on_onlineColorButton_clicked();
	void on_offlineColorButton_clicked();
	
signals:
    void settingsChanged();
    void settingsSaved();
    
private:
    Ui::contactListSettingsClass ui;
    bool changed;
    QColor accountCol;
    QColor groupCol;
    QColor onlineCol;
    QColor offlineCol;
};

#endif // CONTACTLISTSETTINGS_H
