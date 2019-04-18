/*****************************************************************************
**                                                                          **
**    Copyright (C) 2008 Denisss (Denis Novikov). All rights reserved.      **
**                                                                          **
**    This file contains  implementation of class SoundEvents               **
**                                                                          **
**    This  file may be used  under the terms of the GNU General  Public    **
**    License  version 2.0 as published by the Free Software Foundation.    **
**    Alternatively  you may (at your option) use any  later  version of    **
**    the GNU General Public License if such license has  been  publicly    **
**    approved by Trolltech ASA (or its  successors, if any) and the KDE    **
**    Free Qt Foundation.                                                   **
**                                                                          **
**        This file is provided AS IS with NO  WARRANTY OF ANY KIND,        **
**          INCLUDING THE  WARRANTY OF DESIGN, MERCHANTABILITY AND          **
**                   FITNESS  FOR A PARTICULAR PURPOSE.                     **
**                                                                          **
*****************************************************************************/

#include <QtCore>
#include <QSound>

//#include <phonon> // not ready yet

#include "soundevents.h"

using namespace SoundEvent;

SoundEvents::SoundEvents(const QString &anAccount, QObject *parent)
:QObject(parent), account(anAccount), isConnected(false), birthPlayed(false)
	, chatPlay(false)
{
	loadSettings();
	play(Startup, online, true);
}

//static
void SoundEvents::play(const SoundSystem &sys,
	const QString &eventFile)
{
	if (sys == None) return;
	if (sys == LibPhonon)
	{
		// not ready yet
	}
	else if (sys == LibSound)
	{
		QSound::play(eventFile);
	}
	else if (sys == UserCommand)
	{
		QProcess::startDetached(eventFile);
	}
}


void SoundEvents::loadSettings()
{
	sounds.clear();
	soundDirs.clear();

	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."
		+ account, "account");

	settings.beginGroup("sounds");

	currentSys = static_cast<SoundSystem>(settings.value("system", 0).toInt());

	statuses = settings.value("statuses", QBitArray(STATUS_COUNT, true)).toBitArray();
	statuses.resize(STATUS_COUNT);

	QString fileName;
	QString dir;
	int i, dirIdx;
	for (i = 0; i < EVENT_COUNT; i++)
	{
		fileName = settings.value("event" + QString::number(i), "-").toString();
		if (fileName.at(0) == '-') continue;
		fileName = fileName.mid(1);
		if (fileName.isEmpty() || !QFile::exists(fileName)) continue;
		dir = fileName.section('/', 0, -2);
		if (dir.at(dir.size()-1) == '/') dir.chop(1);
		dirIdx = soundDirs.indexOf(dir);
		if (dirIdx < 0)
		{
			dirIdx = soundDirs.size();
			soundDirs.append(dir);
		}

		sounds.insert(static_cast<Events>(i), QPair<int, QString>(dirIdx,
			fileName.mid(fileName.lastIndexOf('/')+1)));
	}

	chatPlay = !settings.value("nochat", false).toBool();

	command = settings.value("command", QString()).toString().trimmed();
	if (currentSys == UserCommand)
	{
		if (command.isEmpty()) currentSys = None;
		else if (!command.contains("%1")) command += " \"%1\"";
	}

	settings.endGroup();
}

void SoundEvents::play(const Events &event,
	const accountStatus &status, const bool ignoreStatus)
{
	if (!isConnected && (event == Disconnected)) return;
	if (event == Connected) isConnected = true;
	else if (event == Disconnected) isConnected = false;

	// check if sound system is chosen
	if (currentSys == None) return;
	// check account status
	if (!statuses.at(static_cast<int>(status)) && !ignoreStatus) return;
	// check if event assigned with some sound file
	if (!sounds.contains(event)) return;

	QPair<int, QString> filePair(sounds.value(event));
	QString fileName = filePair.second;


	// check if sound file exists
	if (!QFile::exists(QString("%1/%2").arg(soundDirs.at(filePair.first)).arg(fileName)))
		return;

	if (event == ContactBirthday)
	{
		if (birthPlayed) return;
		birthPlayed = true;
	}

	// remember cd
	QString saveCD(QDir::currentPath());
	QDir::setCurrent(soundDirs.at(filePair.first));

	if (currentSys == UserCommand)
	{
		play(currentSys, command.arg(fileName));
	}
	else
	{
		play(currentSys, fileName);
	}
	// restore cd
	QDir::setCurrent(saveCD);
}
