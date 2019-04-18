/*
    pluginSettings

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

#ifndef PLUGINSETTINGS_H
#define PLUGINSETTINGS_H

#include <QtGui>
#include "ui_pluginsettings.h"
#include "plugins/plugininterfaces.h"

class pluginSettings : public QWidget
{
    Q_OBJECT

public:
    pluginSettings(QWidget *parent = 0);
    ~pluginSettings();

private:
    Ui::pluginSettingsClass ui;
};

#endif // PLUGINSETTINGS_H
