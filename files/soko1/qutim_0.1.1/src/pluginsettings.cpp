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


#include "pluginsettings.h"

pluginSettings::pluginSettings(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	const QDir dir("");

	foreach (QObject *plugin, QPluginLoader::staticInstances())
	{
		ui.pluginList->addItem(plugin->metaObject()->className());
	}
	
    foreach (QString fileName, dir.entryList(QDir::Files)) {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
        	ui.pluginList->addItem(plugin->metaObject()->className());
        }
    }

}

pluginSettings::~pluginSettings()
{

}
