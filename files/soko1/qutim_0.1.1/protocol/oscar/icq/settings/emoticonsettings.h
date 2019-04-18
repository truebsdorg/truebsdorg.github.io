/*
    emoticonSettings

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


#ifndef EMOTICONSETTINGS_H
#define EMOTICONSETTINGS_H

#include <QtGui>
#include <QtXml>
#include "ui_emoticonsettings.h"


class emoticonSettings : public QWidget
{
    Q_OBJECT

public:
    emoticonSettings(QWidget *parent = 0);
    ~emoticonSettings();
    void loadSettings(const QString &);
    void saveSettings(const QString &);
    
signals:
    void settingsChanged();
    void settingsSaved();

private slots:
	void on_themeBox_currentIndexChanged ( int index );
	void widgetStateChanged() { changed = true; emit settingsChanged(); }
    
private:
    Ui::emoticonSettingsClass ui;
    bool changed;
    void findEmoticonThemes(const QString &);
};

#endif // EMOTICONSETTINGS_H
