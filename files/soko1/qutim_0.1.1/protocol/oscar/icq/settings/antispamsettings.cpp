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


#include "antispamsettings.h"

antiSpamSettings::antiSpamSettings(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	changed = false;
	
	connect (ui.acceptListBox , SIGNAL(toggled(bool)),
					this, SLOT(widgetStateChanged()));
	connect (ui.notifyBox , SIGNAL(toggled(bool)),
					this, SLOT(widgetStateChanged()));
	connect (ui.authBox , SIGNAL(toggled(bool)),
					this, SLOT(widgetStateChanged()));
	connect (ui.urlBox , SIGNAL(toggled(bool)),
					this, SLOT(widgetStateChanged()));
	connect (ui.botBox , SIGNAL(toggled(bool)),
					this, SLOT(widgetStateChanged()));
	connect (ui.replyBox , SIGNAL(toggled(bool)),
					this, SLOT(widgetStateChanged()));

	
	connect (ui.questionEdit , SIGNAL(textChanged ( )),
					this, SLOT(widgetStateChanged()));
	connect (ui.answerEdit , SIGNAL(textChanged ( const QString & )),
					this, SLOT(widgetStateChanged()));
	connect (ui.rightAnswerEdit , SIGNAL(textChanged ( const QString & )),
					this, SLOT(widgetStateChanged()));
	
}

antiSpamSettings::~antiSpamSettings()
{

}

void antiSpamSettings::loadSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("antispam");
	
	
	ui.notifyBox->setChecked(settings.value("notify", true).toBool());
	ui.authBox->setChecked(settings.value("auth", false).toBool());
	ui.urlBox->setChecked(settings.value("url", false).toBool());
	ui.botBox->setChecked(settings.value("bot", false).toBool());
	ui.replyBox->setChecked(settings.value("reply", false).toBool());
	
	ui.questionEdit->append(Qt::escape(settings.value("question").toString()));
	ui.answerEdit->setText(settings.value("answer").toString());
	ui.rightAnswerEdit->setText(settings.value("rightansw").toString());
	ui.acceptListBox->setChecked(settings.value("accept", false).toBool());
	settings.endGroup();
}

void antiSpamSettings::saveSettings(const QString &account)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim/ICQ."+account, "account");
	settings.beginGroup("antispam");
	
	settings.setValue("accept", ui.acceptListBox->isChecked());
	settings.setValue("notify", ui.notifyBox->isChecked());
	settings.setValue("auth", ui.authBox->isChecked());
	settings.setValue("url", ui.urlBox->isChecked());
	settings.setValue("bot", ui.botBox->isChecked());
	settings.setValue("reply", ui.replyBox->isChecked());
	settings.setValue("question", ui.questionEdit->toPlainText().left(1000));
	settings.setValue("answer", ui.answerEdit->text());
	settings.setValue("rightansw", ui.rightAnswerEdit->text());
	
	
	settings.endGroup();
	if ( changed )
			emit settingsSaved();
	changed = false;
	
}
