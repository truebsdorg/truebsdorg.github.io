/*****************************************************************************
**                                                                          **
**    Copyright (C) 2008 Denisss (Denis Novikov). All rights reserved.      **
**                                                                          **
**    This file contains  declaration of class SoundEvents                  **
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

#ifndef SOUNDEVENTS_H
#define SOUNDEVENTS_H

#include <QObject>
#include <QMap>
#include <QPair>
#include <QBitArray>
#include <QStringList>

#include "quticqglobals.h"

class SoundEvents : public QObject
{
Q_OBJECT
	public:
		SoundEvents(const QString &anAccount, QObject *parent = 0);
		static void play(const SoundEvent::SoundSystem &sys,
			const QString &eventFile);
		inline bool playIfChatWindowIsActive() const { return chatPlay; };

	private:
		QString command, account;
//		icqAccount *account;
		SoundEvent::SoundSystem currentSys;
		QStringList soundDirs;
		QMap<SoundEvent::Events, QPair<int, QString> > sounds;
		QBitArray statuses;
		bool isConnected, birthPlayed, chatPlay;

	public slots:
		void loadSettings();
		void play(const SoundEvent::Events &event, const accountStatus &status,
			const bool ignoreStatus = false);
};

#endif // SOUNDEVENTS_H
