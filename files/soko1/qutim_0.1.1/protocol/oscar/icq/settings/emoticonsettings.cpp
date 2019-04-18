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

#include "emoticonsettings.h"

emoticonSettings::emoticonSettings(QWidget *parent)
:QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.themeBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(widgetStateChanged()));
}

emoticonSettings::~emoticonSettings()
{

}

void emoticonSettings::loadSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");

	QStringList paths;
	QString appPath = qApp->applicationDirPath();
	if (appPath.at(appPath.size()-1) == '/') appPath.chop(1);

#if !defined(Q_WS_WIN)
	// ../share/qutim
	paths << appPath.section('/', 0, -2) + "/share/qutim";
	// ~/.config/qutim
	paths << settings.fileName().section('/', 0, -3);
#endif
	// ./
	paths << appPath;
#if defined(Q_WS_MAC)
	// ./ Application bundle
	paths << qApp->applicationDirPath().section('/', 0, -2) + "/Resources";
#endif

	foreach(QString path, paths)
	{
		findEmoticonThemes(path + "/emoticons");
	}
	ui.themeBox->setCurrentIndex(0); // redundant

	settings.beginGroup("emoticons");
//	ui.themeBox->setCurrentIndex(settings.value("current", 0).toUInt()); // not very good
	QString emoticonsPath = settings.value("path", "").toString();
	if ( !emoticonsPath.isEmpty())
		ui.themeBox->setCurrentIndex(ui.themeBox->findData(emoticonsPath));
	else 
		if ( ui.themeBox->count())
			ui.themeBox->setCurrentIndex(0);
	settings.endGroup();
}

void emoticonSettings::saveSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("emoticons");
//	settings.setValue("current", ui.themeBox->currentIndex()); // redundant
	settings.setValue("path", ui.themeBox->itemData(ui.themeBox->currentIndex()));
	settings.endGroup();

	if (changed)
		emit settingsSaved();
	changed = false;
}

void emoticonSettings::findEmoticonThemes(const QString &path)
{
	QDir emoticonPath(path);

	QStringList themes(emoticonPath.entryList(QDir::AllDirs | QDir::NoDotAndDotDot));

	foreach(QString dirName, themes)
	{
		QDir dir(path + "/" + dirName);
		QStringList filter;
		filter << "*.xml";

		QStringList fileXML = dir.entryList(filter,QDir::Files);

		if (fileXML.count())
			ui.themeBox->addItem(dirName, dir.path() + "/" + fileXML.at(0));
	}
}

void emoticonSettings::on_themeBox_currentIndexChanged(int index)
{
	ui.emoticonList->clear();
	QFile file(ui.themeBox->itemData(index).toString());

	QString dirPath = ui.themeBox->itemData(index).toString().section('/', 0, -2);
	QDir dir (dirPath);

	QStringList fileList = dir.entryList(QDir::Files);

	if (file.open(QIODevice::ReadOnly) )
	{
		QDomDocument doc;
		if (doc.setContent(&file))
		{
			QDomElement rootElement = doc.documentElement();
			int emoticonCount = rootElement.childNodes().count();

			QDomElement emoticon = rootElement.firstChild().toElement();

			for (int i = 0; i < emoticonCount; i++)
			{
				if ( emoticon.tagName() == "emoticon")
				{
					QStringList fileName = fileList.filter(emoticon.attribute("file"));
					if ( !fileName.isEmpty())
					{
						QListWidgetItem *item = new QListWidgetItem(ui.emoticonList);
						item->setIcon(QIcon(dirPath + "/" + fileName.at(0)));
					}
				}
				emoticon = emoticon.nextSibling().toElement();
			}
		}
	}
}
